/*
 * Copyright 2014 Evan Lojewski. All rights reserved.
 *
 *
 * Original code moved from libsaio/spd.[c,h]
 */

#include <modules.h>
#include <pci.h>
#include <boot.h>
#include <bootstruct.h>

#include "hpet.h"

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3);

void MemoryInfo_init()
{
    register_hook_callback("PCIDevice", &found_pci_device);
}

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3)
{
    bool do_enable_hpet = false;
    pci_dt_t *current = device;

    
    if(current->class_id == PCI_CLASS_BRIDGE_ISA)
    {
        getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->chameleonConfig);

        if (do_enable_hpet)
        {
            force_enable_hpet(current);
        }
    }
}