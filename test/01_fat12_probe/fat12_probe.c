/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#include <stdlib.h>
#include <modules.h>
#include "libsaio.h"

void fat12_prone_start();

void system_shutdown (void);


#define PORT 0x2f8   /* COM2 */

void init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}



int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}

void fat12_probe_start()
{
    extern BVRef bvChain;
    init_serial();
    printf("Replacing 'bios_putchar' with 0x%x\n", &write_serial);
    replace_function("bios_putchar", (void*)&write_serial);

    if(!bvChain) printf("PASS\n"); // NO file system sshould hav ebeen found.
    system_shutdown();
}
