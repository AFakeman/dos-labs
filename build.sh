#! /bin/bash

docker run --rm -it \
    -v $PWD/src:/home/xclient/src \
    -v $PWD/build:/home/xclient/build \
    -v $PWD/make.mk:/home/xclient/make.mk \
    --entrypoint /bin/bash \
    open-watcom wine cmd.exe /C 'C:\WATCOM\owsetenv.bat & wmake -f make.mk -h -e build\hello4.exe'
