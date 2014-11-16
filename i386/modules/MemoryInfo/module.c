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

#include "spd.h"
#include "dram_controllers.h"

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3);
void scan_mem(pci_dt_t* dev);

void MemoryInfo_init()
{
    register_hook_callback("PCIDevice", &found_pci_device);
}

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3)
{
    pci_dt_t *current = device;

    if(current->class_id == PCI_CLASS_BRIDGE_HOST)
    {
        if (current->dev.addr == PCIADDR(0, 0, 0))
        {
            // TODO: ensure this always gets called.
            scan_mem(current);
        }
    }
}



/** scan mem for memory autodection purpose */
void scan_mem(pci_dt_t* dev)
{
    bool memDetect;
    static bool done = false;
    if (done) {
        return;
    }
    
    /* our code only works on Intel chipsets so make sure here */
    if (pci_config_read16(PCIADDR(0, 0x00, 0), 0x00) != 0x8086) {
        memDetect = false;
    } else {
        memDetect = true;
    }
    /* manually */
    getBoolForKey(kUseMemDetect, &memDetect, &bootInfo->chameleonConfig);
    
    if (memDetect) {
        if (dev != NULL) {
            scan_dram_controller(dev); // Rek: pci dev ram controller direct and fully informative scan ...
        }
        scan_spd(&Platform);
    }
    done = true;
}
