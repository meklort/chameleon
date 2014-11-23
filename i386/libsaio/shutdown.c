#include "io_inline.h"
#include "modules.h"

void system_shutdown (void)
{
  // Notify clients that we are abuit to exit.
  execute_hook("Exit", NULL, NULL, NULL, NULL);

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

