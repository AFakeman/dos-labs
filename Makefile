.PHONY: run_parallels_vm clean

WATCOM_IMAGE=afakeman/wine-open-watcom
FLOPPY_IMAGE=file.flp
MOUNT_POINT=tmp
VM_NAME='MS-DOS 6.22'

all: ${FLOPPY_IMAGE}

build/%.exe : src/%.c
	docker run --rm -it \
		-v ${PWD}/src:/home/xclient/src \
		-v ${PWD}/build:/home/xclient/build \
		-v ${PWD}/make.mk:/home/xclient/make.mk \
		${WATCOM_IMAGE} wine cmd.exe /C "C:\\WATCOM\\owsetenv.bat & wmake -f make.mk -h -e build\\$*.exe"

build/dos4gw.exe :
	docker run --rm -it ${WATCOM_IMAGE} cat .wine/drive_c/WATCOM/binw/dos4gw.exe > build/dos4gw.exe

${FLOPPY_IMAGE} : build/dos4gw.exe $(wildcard build/*.exe)
	dd if=/dev/zero bs=512 count=2880 of="${FLOPPY_IMAGE}" && \
	newfs_msdos -v FLOPPY -f 1440 ./"${FLOPPY_IMAGE}" && \
    hdiutil attach -imagekey diskimage-class=CRawDiskImage \
		${FLOPPY_IMAGE} -mountpoint ${MOUNT_POINT} && \
    cp build/*.exe ${MOUNT_POINT} && \
    hdiutil unmount ${MOUNT_POINT}

run_parallels_vm : ${FLOPPY_IMAGE}
	prlctl stop ${VM_NAME} --kill && \
	prlctl start ${VM_NAME}

clean :
	rm -rf build ${FLOPPY_IMAGE}
