//
// Created by root on 9/5/24.
//

#ifndef GLADOS_KDEF_H
#define GLADOS_KDEF_H

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		/* End-of-interrupt command code */

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define INTERRUPT_GENERAL_PROTECTION_FAULT 13
#define INTERRUPT_PAGE_FAULT 14
#define INTERRUPT_TIMER 32
#define INTERRUPT_KEYBOARD 33
#define INTERRUPT_RTC 40

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define TSS_SELECTOR 0x18

#define IDT_ENTRIES 256

#endif  //GLADOS_KDEF_H
