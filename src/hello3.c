/*
 * TODO (task)
 * extend descriptor_t union to support system descriptors such as gates
 * use fprintf to dump GDT/IDT descriptors in human readible format
 */

#include "stdio.h"

#define CR0_PE 0
#define CR0_PG 31

#pragma pack (push, 1)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int bool_t;

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
        uint8_t type:4;             //T=1111    T=1110
        uint8_t S:1;                //S=0       S=0
        uint8_t DPL:2;              //DPL=3     DPL=0
        uint8_t P:1;                //P=1       P=1
        uint16_t offset_high;       //offset my_pf_handler (hi)
    };
} gate_t;

typedef union _selector_t
{
    uint16_t raw;
    struct {
        uint16_t rpl:2;
        uint16_t table:1;
        uint16_t index:13;
    };
} selector_t;

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

    //Print string through DOS syscallX
    printf("Hello, C! \n\r");
    __asm {
        mov ax, ds
        mov es, ax
        mov ah, 09h
        mov edx, gstr
        int 21h
    }

    {
        tr_t _gdt, _idt;
        selector_t sel;
        uint16_t _cs, _ds;
        descriptor_t* pDesc;
        uint32_t base;
        uint32_t limit;
        gate_t *idt_base;
        uint16_t idt_count;
        uint16_t idx;

        __asm {
            mov ax, cs
            mov _cs, ax
            mov ax, ds
            mov _ds, ax            
            sgdt [_gdt]
            sidt [_idt]
        }
        printf("GDT: Base=0x%08X Limit=0x%04X \n", _gdt.base, _gdt.limit);
        printf("IDT: Base=0x%08X Limit=0x%04X \n", _idt.base, _idt.limit);

        sel.raw = _cs;
        printf("SEL: raw=0x%04X index=%x table=%x rpl=%x \n", sel.raw, sel.index, sel.table, sel.rpl);

        pDesc = ((descriptor_t*)_gdt.base) + sel.index;
        printf("pDesc = 0x%08x \n", pDesc);

        base = pDesc->base_low | (pDesc->base_mid << 16) | (pDesc->base_high << 24);
        limit = pDesc->limit_low | (pDesc->limit_high << 16);
        if (pDesc->G) limit = (limit<<12) + 0xFFF;
        printf("DESC: raw=0x%08X'%08X base=0x%08X limit=0x%08X L=%x DB=%x AR=0x%04X \n",
            pDesc->raw_high, pDesc->raw_low,
            base, limit,
            pDesc->L,
            pDesc->DB,
            pDesc->access_rights);

        pDesc = (descriptor_t*) _gdt.base;
        idt_count = _gdt.limit / sizeof(descriptor_t);

        // for (idx = 0; idx < idt_count; ++idx) {
        //     printf("DESC: raw=0x%08X'%08X base=0x%08X limit=0x%08X L=%x DB=%x AR=0x%04X \n",
        //         pDesc[idx].raw_high, pDesc[idx].raw_low,
        //         base, limit,
        //         pDesc[idx].L,
        //         pDesc[idx].DB,
        //         pDesc[idx].access_rights);
        //     getchar();
        // }

        sel.raw = _ds;
        printf("SEL: raw=0x%04X index=%x table=%x rpl=%x \n", sel.raw, sel.index, sel.table, sel.rpl);

        pDesc = ((descriptor_t*)_gdt.base) + sel.index;
        printf("pDesc = 0x%08x \n", pDesc);

        base = pDesc->base_low | (pDesc->base_mid << 16) | (pDesc->base_high << 24);
        limit = pDesc->limit_low | (pDesc->limit_high << 16);
        if (pDesc->G) limit = (limit<<12) + 0xFFF;
        printf("DESC: raw=0x%08X'%08X base=0x%08X limit=0x%08X L=%x DB=%x AR=0x%04X \n",
            pDesc->raw_high, pDesc->raw_low,
            base, limit,
            pDesc->L,
            pDesc->DB,
            pDesc->access_rights);

        idt_base = (gate_t*) _idt.base;
        idt_count = _idt.limit / sizeof(gate_t);
        for (idx = 0; idx < idt_count; ++idx) {
            printf("GATE %h: offset_low=0x%04X selector=0x%04X, type=0x%X, S=%d, DPL=%d, P=%d, offset_high=0x%04X\n",
            idt_base[idx].offset_low,
            idt_base[idx].selector,
            idt_base[idx].type,
            idt_base[idx].S,
            idt_base[idx].DPL,
            idt_base[idx].P,
            idt_base[idx].offset_high
                );
            getchar();
        }
    }

    printf("EXPECTED PANIC \n");
    //Do panic
    ///*
     __asm {
        mov ax, 0
        mov cs, ax
    }
    //*/
}
