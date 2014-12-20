/*
 * Supplemental ramdisk functions for the multiboot ramdisk driver.
 * Copyright 2009 Tamas Kosarszky. All rights reserved.
 *
 */

#include "boot.h"
#include <libsaio/bootstruct.h>
#include "multiboot.h"
#include "ramdisk.h"

// Notify OS X that a ramdisk has been setup. XNU with attach this to /dev/md0
void md0Ramdisk()
{
	RAMDiskParam ramdiskPtr;
	char filename[512];
	const char* override_filename = 0;
	int fh = -1;
	int len;
	
	if(getValueForKey(kMD0Image, &override_filename, &len,  
				   &bootInfo->chameleonConfig))
	{
		// Use user specified md0 file
		snprintf(filename, sizeof(filename), "%s", override_filename);
		fh = open(filename, 0);
		
		if(fh < 0)
		{
			snprintf(filename, sizeof(filename), "rd(0,0)/Extra/%s", override_filename);
			fh = open(filename, 0);

			if(fh < 0)
			{
				snprintf(filename, sizeof(filename), "/Extra/%s", override_filename);
				fh = open(filename, 0);
			}
		}		
	}

	if(fh < 0)
	{
		sprintf(filename, "rd(0,0)/Extra/Postboot.img");
		fh = open(filename, 0);

		if(fh < 0)
		{
			sprintf(filename, "/Extra/Postboot.img");	// Check /Extra if not in rd(0,0)
			fh = open(filename, 0);
		}
	}		

	if (fh >= 0)
	{
		verbose("Enabling ramdisk %s\n", filename);

		ramdiskPtr.size  = file_size(fh);
		ramdiskPtr.base = AllocateKernelMemory(ramdiskPtr.size);

		if(ramdiskPtr.size && ramdiskPtr.base)
		{
			// Read new ramdisk image contents in kernel memory.
			if (read(fh, (char*) ramdiskPtr.base, ramdiskPtr.size) == ramdiskPtr.size)
			{
				AllocateMemoryRange("RAMDisk", ramdiskPtr.base, ramdiskPtr.size, kBootDriverTypeInvalid);
				Node* node = DT__FindNode("/chosen/memory-map", false);
				if(node != NULL)
				{
					DT__AddProperty(node, "RAMDisk", sizeof(RAMDiskParam),  (void*)&ramdiskPtr);		
				}
				else
				{
					verbose("Unable to notify Mac OS X of the ramdisk %s.\n", filename);
				}
			}
			else
			{
				verbose("Unable to read md0 image %s.\n", filename);
			}			
		}
		else
		{
			verbose("md0 image %s is empty.\n", filename);
		}

		close(fh);

	}
}
