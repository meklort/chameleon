/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#include <cstdlib>
#include <modules>
#include "libsaio.h"

extern "C"
{
    void module_init_start();
}

void system_shutdown (void);


void module_init_start()
{
	printf("Hooking 'ExecKernel'\n");
	system_shutdown();
}

void system_shutdown (void)
{
  asm volatile ("cli");
  for (;;)
    {
      // (phony) ACPI shutdown (http://forum.osdev.org/viewtopic.php?t=16990)
      // Works for qemu and bochs.
      outw (0xB004, 0x2000);

      // Magic shutdown code for bochs and qemu.
      for (const char *s = "Shutdown"; *s; ++s)
        outb (0x8900, *s);

      // Magic code for VMWare. Also a hard lock.
      asm volatile ("cli; hlt");
    }
}

