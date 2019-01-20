#!/bin/bash

set -e

VM_NAME='MS-DOS 6.22'
DISK='file.flp'
MOUNT_POINT='tmp'

function copy_to_disk() {
    DISK=$1
    shift
    MOUNT_POINT=$1
    shift
    hdiutil attach -imagekey diskimage-class=CRawDiskImage $DISK -mountpoint $MOUNT_POINT
    cp build/*.exe $MOUNT_POINT
    hdiutil unmount $MOUNT_POINT
}

prlctl stop "$VM_NAME" --kill
copy_to_disk "$DISK" "$MOUNT_POINT"
prlctl start "$VM_NAME"
prlctl set "$VM_NAME" --device-add fdd --image $DISK --recreate || echo Disk already connected
