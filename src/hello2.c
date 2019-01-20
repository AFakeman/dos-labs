#include "stdio.h"

#define CR0_PE 0
#define CR0_PG 31

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int bool_t;

char* gstr = "Hello, DOS!\n$";

void main()
{
    uint16_t _cs;
    uint32_t _cr0;
    bool_t bProtected;
    bool_t bPaging;
    char nRing;

    //Read system registers
    __asm {
        mov eax, cr0
        mov _cr0, eax
        mov ax, cs
        mov _cs, ax
    }

    //Calculate modes
    bProtected = (_cr0 & (1<<CR0_PE));
    bPaging = (_cr0 & (1<<CR0_PG));
    nRing = (_cs & 0x3);

    //Print modes
    printf("CR0: 0x%08X \n", _cr0);
    printf("CS: 0x%08X \n", _cs);
    printf("Protected mode: %s \n", bProtected ? "ON" : "OFF");
    printf("Paging mode: %s \n", bPaging ? "ON" : "OFF");
    printf("Ring: %d %s \n", nRing, (nRing==0)? "KERNEL" : "NO!!!");

    //Print string through DOS syscall
    printf("Hello, C! \n");
    __asm {
        mov ax, ds
        mov es, ax
        mov ah, 09h
        mov edx, gstr
        int 21h
    }
 
    //Do panic
//    __asm {
//        mov ax, 0
//        mov cs, ax
//    }
}
