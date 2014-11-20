/*
* Copyright 2014 Evan Lojewski. All rights reserved.
 *
 *
 * Original code moved from libsaio/pci_setup.h
 */

#include <modules.h>
#include <libsaio/pci.h>
#include "gma.h"
#include "nvidia.h"
#include "ati.h"

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3);


void GraphicsEnabler_init()
{
	register_hook_callback("PCIDevice", &found_pci_device);
}

static void found_pci_device(void* device, void* arg1, void* arg2, void* arg3)
{
	bool doit, do_gfx_devprop = false;
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->chameleonConfig);
	if(!do_gfx_devprop) return;

	pci_dt_t *current = device;
	if(current->class_id == PCI_CLASS_DISPLAY_VGA)
	{
		switch (current->vendor_id)
		{
			case PCI_VENDOR_ID_ATI:
				if (getBoolForKey(kSkipAtiGfx, &doit, &bootInfo->chameleonConfig) && doit)
				{
					verbose("Skip ATi/AMD gfx device!\n");
				}
				else
				{
					setup_ati_devprop(current);
				}
				break;

			case PCI_VENDOR_ID_INTEL:
				if (getBoolForKey(kSkipIntelGfx, &doit, &bootInfo->chameleonConfig) && doit)
				{
					verbose("Skip Intel gfx device!\n");
				}
				else
				{
					setup_gma_devprop(current);
				}
				break;

			case PCI_VENDOR_ID_NVIDIA:
				if (getBoolForKey(kSkipNvidiaGfx, &doit, &bootInfo->chameleonConfig) && doit)
				{
					verbose("Skip Nvidia gfx device!\n");
				}
					else
				{
					setup_nvidia_devprop(current);
				}
				break;
		}
	}

}
