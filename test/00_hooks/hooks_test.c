/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#include <stdlib.h>
#include <modules.h>
#include "libsaio.h"

int gHook1;
int gHook2;

void hooks_test_start();

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

void exec_hook1(void* arg1, void* arg2, void* arg3, void* arg4)
{
    gHook1++;
}

void exec_hook2(void* arg1, void* arg2, void* arg3, void* arg4)
{
    gHook2++;
}

void hooks_test_start()
{
    gHook1 = 0;
    gHook2 = 0;
    
    init_serial();
    printf("Replacing 'bios_putchar' with 0x%x\n", &write_serial);
    replace_function("_bios_putchar", (void*)&write_serial);

    register_hook_callback("Hook1", exec_hook1);
    register_hook_callback("Hook2", exec_hook2);
    register_hook_callback("Hook1", exec_hook1);
    
    execute_hook("Hook1", NULL, NULL, NULL, NULL);
    execute_hook("Hook2", NULL, NULL, NULL, NULL);
    printf("gHook1 is %d\n", gHook1);
    printf("gHook2 is %d\n", gHook2);

    if((gHook1 == 2) && (gHook2 == 1)) printf("PASS\n");
    else printf("FAIL\n");
    system_shutdown();
}
