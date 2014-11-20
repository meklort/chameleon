/*
 * Copyright 2014 Evan Lojewski. All rights reserved.
 *
 *
 * Original code moved from libsaio/spd.[c,h]
 */

#include <modules.h>
#include <libsaio/pci.h>
#include <boot.h>
#include <libsaio/bootstruct.h>

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3);
static void fix_usb(void* device, void* arg1, void* arg2, void* arg3);

/*
 * usb.c
 */
extern int usb_loop();
extern void notify_usb_dev(pci_dt_t *pci_dev);

void USBFix_init()
{
    register_hook_callback("PCIDevice", &found_pci_device);
    register_hook_callback("Kernel Start", &fix_usb);
}

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3)
{
    pci_dt_t *current = device;
    
    if(current->class_id == PCI_CLASS_SERIAL_USB)
    {
        notify_usb_dev(current);
    }
}
static void fix_usb(void* device, void* arg1, void* arg2, void* arg3)
{
    usb_loop();
}