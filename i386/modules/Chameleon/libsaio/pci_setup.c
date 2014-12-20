#include "libsaio.h"
#include "boot.h"
#include <libsaio/bootstruct.h>
#include <libsaio/pci.h>
#include <libsaio/hda.h>
#include "modules.h"


extern bool setup_hda_devprop(pci_dt_t *hda_dev);
extern void set_eth_builtin(pci_dt_t *eth_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool do_eth_devprop, do_hda_devprop;
	pci_dt_t *current = pci_dt;

	do_eth_devprop = do_hda_devprop = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kHDAEnabler, &do_hda_devprop, &bootInfo->chameleonConfig);

	while (current)
	{
		devicepath = get_pci_dev_path(current);

		switch (current->class_id)
		{
			case PCI_CLASS_NETWORK_ETHERNET: 
				if (do_eth_devprop)
				{
					set_eth_builtin(current);
				}
				break;

			case PCI_CLASS_MULTIMEDIA_AUDIO_DEV:
				if (do_hda_devprop)
				{
					setup_hda_devprop(current);
				}
				break;
		}
		
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		
		setup_pci_devs(current->children);
		current = current->next;
	}
}
