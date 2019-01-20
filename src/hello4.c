/*
 *  TODO (task) - create 2-level trivial paging (4K pages)
 * 		- map pagetables at 0xC0000000 and use theese addresses to change them
 *		- create gap between 0x80000000 and 0xBFFFFFFF
 *		- if pagefault occurs in the gap restore P bit
 */

#include "stdio.h"
#include "stdlib.h"

#define EXC_PF 14

#define CR0_PE 0
#define CR0_PG 31
#define CR4_PS 4

#define PDE_TRIVIAL     0x087  // PS=1  D=0  A=0 PCD=0
                               //PWT=0 US=1 RW=1 P=1
#define PDE_TRIVIAL_NP  0x086  //same but P=0
// same but PS=1
#define PDE_NON_TRIVIAL 0x0C7
// same but PS=1 P=0
#define PDE_NON_TRIVIAL_NP 0x0C6

#define SPECIAL_ADDRESS 0xDEADFACE
// R=1 P=0
#define PTE_FLAGS 0x6
#define PTE_FLAGS_P 0x7

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int bool_t;

#pragma pack(1)

typedef union _gate_t
{
    struct {
        uint32_t raw_lo;
        uint32_t raw_hi;
    };
    struct {
        uint16_t offset_low;        //offset my_pf_handler (lo)
        uint16_t selector;          //seg my_pf_handler
        uint8_t zero;               //0
        uint8_t access_rights;      //11101111 0xEF 0x8E
        /*
            uint8_t type:4;             //T=1111    T=1110
            uint8_t S:1;                //S=0       S=0
            uint8_t DPL:2;              //DPL=3     DPL=0
            uint8_t P:1;                //P=1       P=1
        */
        uint16_t offset_high;       //offset my_pf_handler (hi)
    };
} gate_t;

typedef union _descriptor_t
{
    struct {
        uint32_t raw_low;
        uint32_t raw_high;
    };
    struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_mid;
        union {
            uint16_t access_rights;
            struct {
                uint8_t type:4;
                uint8_t S:1;
                uint8_t DPL:2;
                uint8_t P:1;
                uint8_t limit_high:4;
                uint8_t AVL:1;
                uint8_t L:1;
                uint8_t DB:1;
                uint8_t G:1;
            };
        };
        uint8_t base_high;
    };
} descriptor_t;

typedef struct _tr_t
{
    uint16_t limit;
    uint32_t base;
    uint16_t padding;
} tr_t;

char* gstr = "Peace, DOS!\n$";

uint16_t old_pf_seg, new_pf_seg;
uint32_t old_pf_ofs, new_pf_ofs;
uint32_t pf_counter = 0;

uint32_t* pPageDirectory;
uint32_t* pPageEntries;

void printhex(uint32_t num) {
    printf("0x%08X\n", num);
}

void reset_present_bit() {
    uint32_t addr;
    uint32_t pde_idx;
    uint32_t *pte_addr;
    uint32_t pte_idx;

    addr = SPECIAL_ADDRESS;
    pde_idx = (SPECIAL_ADDRESS & 0xFFC00000) >> 22;
    printhex(pde_idx);
    printhex((uint32_t) &pPageDirectory[pde_idx]);
    printhex(            pPageDirectory[pde_idx]);

    pte_addr = (uint32_t*) (pPageDirectory[pde_idx] & 0xFFFFF000);
    pte_idx = (SPECIAL_ADDRESS >> 12) & 0x3FF;
    printhex((uint32_t) &pte_addr[pte_idx]);
    printhex(            pte_addr[pte_idx]);

    pPageDirectory[pde_idx] |= 1;
    pte_addr[pte_idx] |= 1;
    ++pf_counter;

    __asm {
        invlpg addr
    }

    getchar();
}

void __declspec(naked) my_pf_handler(void)
{
    /*
     *                          // old_pf_ofs
     *                          // old_pf_seg
     * [esp  ] - error code
     * [esp-4] - eip
     * [esp-8] - cs         
     * [esp-C] - eflags
     * //[esp-10]- esp     //TODO need to check its rpl to guess if there is ss/esp
     * //[esp-14]- ss
     */
    __asm {
        //push 10
        //call printhex
        //add esp, 4

        push eax
        push ebx
        push edx

        mov ax, ds
        mov es, ax
        mov ah, 09h
        mov edx, gstr
        int 21h

        mov eax, cr2

        mov ebx, pPageDirectory

        //push ebx
        //call printhex
        //add esp, 4

        cmp eax, SPECIAL_ADDRESS
        jz new
old:
        pop edx
        pop ebx
        pop eax
        mov eax, [esp]
        push dword ptr old_pf_seg	//need to push 4 bytes
        push old_pf_ofs
        retf
new:
       //  inc pf_counter
       //  mov ebx, cr3

       //  push ebx
       //  call printhex
       //  add esp, 4

       //  and eax, 0xFFC00000
       //  shr eax, 20
       //  add ebx, eax    //eax = pPageDirectory[pdi]

       //  push ebx
       //  call printhex
       //  add esp, 4

       //  push [ebx]
       //  call printhex
       //  add esp, 4

       //  or [ebx], 1
       //  mov eax, [ebx]
       //  mov ebx, eax
       //  and ebx, 0xFFFFF000
       //  mov eax, cr2
       //  and eax, 0x003FF000
       //  shr eax, 10
       //  add ebx, eax   //eax = pPageEntries[pei]

       //  push ebx
       //  call printhex
       //  add esp, 4

       //  push [ebx]
       //  call printhex
       //  add esp, 4

       //  or [ebx], 1

       //  mov ah, 01h
       //  int 21h
        call reset_present_bit

        pop edx
        pop ebx
        pop eax

        add esp, 4
        iretd
    }
}

void set_new_pf_handler()
{
    tr_t _idt, _gdt;
    gate_t* pIdt;
    descriptor_t* pGdt;
    uint32_t base_pf, addr_pf;
    uint8_t* p;

    __asm {
        sidt [_idt]
        sgdt [_gdt]
    }
    printf("IDT: Base=0x%08X Limit=0x%04X \n", _idt.base, _idt.limit);
    printf("GDT: Base=0x%08X Limit=0x%04X \n", _gdt.base, _gdt.limit);
    pIdt = (gate_t*) _idt.base;
    pGdt = (descriptor_t*) _gdt.base;
    old_pf_seg = pIdt[EXC_PF].selector;
    old_pf_ofs = pIdt[EXC_PF].offset_low | (pIdt[EXC_PF].offset_high << 16);
    printf("GATE PF old!: 0x%08X'%08X \n", pIdt[EXC_PF].raw_hi, pIdt[EXC_PF].raw_lo);
    printf("old PF handler!: 0x%04X:%08X \n", old_pf_seg, old_pf_ofs);

    __asm {
        mov ax, seg my_pf_handler
        mov new_pf_seg, ax
        mov eax, offset my_pf_handler
        mov new_pf_ofs, eax
    }

    printf("new handler!: 0x%04X:%08X \n", new_pf_seg, new_pf_ofs);

    //disable interrupts
    __asm cli    
    pIdt[EXC_PF].selector = new_pf_seg;
    pIdt[EXC_PF].offset_low = new_pf_ofs & 0xFFFF;
    pIdt[EXC_PF].offset_high = (new_pf_ofs >> 16) & 0xFFFF;
    pIdt[EXC_PF].zero = 0;
    pIdt[EXC_PF].access_rights = 0x8E;
    //enable interrupts
    __asm sti
    
    base_pf = pGdt[(old_pf_seg>>3)].base_low |
       (pGdt[(old_pf_seg>>3)].base_mid << 16) |
       (pGdt[(old_pf_seg>>3)].base_high << 24);
    addr_pf = base_pf + old_pf_ofs;
    printf("BASE PF old!: 0x%08X \n", base_pf);
    printf("ADDR PF old!: 0x%08X \n", addr_pf);
    p = (uint8_t*)addr_pf;
    printf("*addr = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    
    printf("GATE new!: 0x%08X'%08X \n", pIdt[EXC_PF].raw_hi, pIdt[EXC_PF].raw_lo);
}

void main()
{
    uint16_t _cs;
    uint32_t _cr0;
    bool_t bProtected;
    bool_t bPaging;
    char nRing;
    
    uint32_t i,p, j;

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

    //initialize paging (trivial for first 2GB)
    p = (uint32_t) malloc(8*1024);
#define BIG_PAGE_SHIFT  12
#define BIG_PAGE_SIZE   (1 << BIG_PAGE_SHIFT)   // 4kb = 0x00004000
#define BIG_PAGE_MASK1  (BIG_PAGE_SIZE-1)       //       0x00003FFF
#define BIG_PAGE_MASK2  (~BIG_PAGE_MASK1)       //       0xFFFFC000
#define BIG_PAGE_ALIGN(addr) (((addr)+BIG_PAGE_MASK1)&BIG_PAGE_MASK2)
    pPageDirectory = (uint32_t*) BIG_PAGE_ALIGN(p);

    p = (uint32_t) malloc(8*1024*1024);
    if (!p) {
        printf("BAD MALLOC\n");
        getchar();
    }
    pPageEntries = (uint32_t*) BIG_PAGE_ALIGN(p);

    //any page table is a 4K block with 1024*4b entries
    for (i=0; i<1024; i++)
    {
        pPageDirectory[i] = ((uint32_t)&pPageEntries[i << 10])
                                | ((i<512) ? PDE_NON_TRIVIAL : PDE_NON_TRIVIAL);
        for (j=0; j < 1024; ++j) {
            pPageEntries[(i << 10) | j] = (i<<22) | (j<<12) | PTE_FLAGS;
        }
    }

    /*{
        uint32_t pte_idx;
        uint32_t pde_idx;
        uint32_t block_addr = SPECIAL_ADDRESS & 0xFFFFF000;
        pde_idx = SPECIAL_ADDRESS >> 22;
        pPageDirectory[pde_idx] = ((uint32_t) pPageEntries) | PDE_NON_TRIVIAL;
        printf("PDE_ADR=0x%08X\n", &pPageDirectory[pde_idx]);
        printf("PDE_VAL=0x%08X\n",  pPageDirectory[pde_idx]);
        pte_idx = (SPECIAL_ADDRESS >> 12) & 0x3FF;
        pPageEntries[pte_idx] = block_addr | PTE_FLAGS_P;
        printf("PTE_ADR=0x%08X\n", &pPageEntries[pte_idx]);
        printf("PTE_VAL=0x%08X\n",  pPageEntries[pte_idx]);
    }*/

    __asm {
        mov eax, pPageDirectory
        mov cr3, eax
    }
    
    //enable paging
    __asm {
        cli                     //disable interrupt
        mov eax, cr4
        or  eax, 10h            //enable 4Mb (1 << CR4_PS)
        mov cr4, eax        
        mov eax, cr0
        or  eax, 80000000h      //enable paging (1 << CR0_PG)
        mov cr0, eax            //enable them back
        mov eax, cr3
        mov cr3, eax            //reset paging
        sti
    }

    printf("PAGING IS ON \n");

    set_new_pf_handler();
    
    //get pagefault at non-present address
    {
        uint8_t* pByte1 = (uint8_t*)100;            //100 < 2Gb => ok
        uint8_t* pByte2 = (uint8_t*)SPECIAL_ADDRESS;     //    > 2Gb => pagefault
        uint8_t* pByte3 = (uint8_t*)0xDEADFACF;     //    > 2Gb => pagefault
        uint8_t b = *pByte1;
        printf("[100]=%x \n", b);
        printf("PF counter = %d \n", pf_counter);
        b = *pByte2;
        printf("[0xDEADFACE]=%x \n", b);
        printf("PF counter = %d \n", pf_counter);
        b = *pByte3;
        printf("[0xDEADFACF]=%x \n", b);
        printf("PF counter = %d \n", pf_counter);
    }
    
}
