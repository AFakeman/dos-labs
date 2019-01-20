build\hello4.obj : src\hello4.c .AUTODEPEND
 cd build
 *wcc386 ..\src\hello4.c -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -6r -bt=dos -fo=.obj -mf
 cd ..

build\hello4.exe : build\hello4.obj .AUTODEPEND
 cd build
 @%write hello4.lk1 FIL hello4.obj
 @%append hello4.lk1
 *wlink name hello4 d all sys dos4g op m op maxe=25 op q op symf @hello4.lk1
 cd ..

