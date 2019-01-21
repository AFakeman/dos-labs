hello1.obj : src\hello1.c .AUTODEPEND
 *wcc386 src\hello1.c -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -6r -bt=dos -fo=.obj -mf

build\hello1.exe : hello1.obj .AUTODEPEND
 @%write hello1.lk1 FIL hello1.obj
 @%append hello1.lk1
 *wlink name build\hello1 d all sys dos4g op m op maxe=25 op q op symf @hello1.lk1

hello2.obj : src\hello2.c .AUTODEPEND
 *wcc386 src\hello2.c -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -6r -bt=dos -fo=.obj -mf

build\hello2.exe : hello2.obj .AUTODEPEND
 @%write hello2.lk1 FIL hello2.obj
 @%append hello2.lk1
 *wlink name build\hello2 d all sys dos4g op m op maxe=25 op q op symf @hello2.lk1

hello3.obj : src\hello3.c .AUTODEPEND
 *wcc386 src\hello3.c -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -6r -bt=dos -fo=.obj -mf

build\hello3.exe : hello3.obj .AUTODEPEND
 @%write hello3.lk1 FIL hello3.obj
 @%append hello3.lk1
 *wlink name build\hello3 d all sys dos4g op m op maxe=25 op q op symf @hello3.lk1

hello4.obj : src\hello4.c .AUTODEPEND
 *wcc386 src\hello4.c -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -6r -bt=dos -fo=.obj -mf

build\hello4.exe : hello4.obj .AUTODEPEND
 @%write hello4.lk1 FIL hello4.obj
 @%append hello4.lk1
 *wlink name build\hello4 d all sys dos4g op m op maxe=25 op q op symf @hello4.lk1
