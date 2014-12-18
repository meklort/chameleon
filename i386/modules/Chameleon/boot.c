/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").	You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.	 The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 *			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license	agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988, 1989 by Intel Corporation
 */

/*
 * Copyright 1993 NeXT Computer, Inc.
 * All rights reserved.
 */

/*
 * Completely reworked by Sam Streeper (sam_s@NeXT.com)
 * Reworked again by Curtis Galloway (galloway@NeXT.com)
 */

#include "boot.h"
#include <libsaio/bootstruct.h>
#include <libsaio/fake_efi.h>
#include "sl.h"
#include "libsa.h"
#include "ramdisk.h"
#include <libsaio/platform.h>
#include "modules.h"

/*
 * How long to wait (in seconds) to load the
 * kernel after displaying the "boot:" prompt.
 */
#define kBootErrorTimeout 5

bool		gOverrideKernel, gEnableCDROMRescan, gScanSingleDrive;
static bool	gUnloadPXEOnExit = false;

static char	gCacheNameAdler[64 + 256];
char		*gPlatformName = gCacheNameAdler;

char		gRootDevice[ROOT_DEVICE_SIZE];
char		gMKextName[512];
char		gMacOSVersion[8];
int		bvCount = 0, gDeviceCount = 0;
//int		menucount = 0;
long		gBootMode; /* defaults to 0 == kBootModeNormal */
BVRef		bvr, menuBVR, bvChain;

static bool				checkOSVersion(const char * version);
static void				getOSVersion();
static unsigned long	Adler32(unsigned char *buffer, long length);
//static void			selectBiosDevice(void);

/** options.c **/
extern char* msgbuf;
void showTextBuffer(char *buf, int size);


//==========================================================================
// Zero the BSS.

#if 0
static void zeroBSS(void)
{
	extern char*  __bss_stop;
	extern char*  __bss_start;
	bzero(&__bss_start, (&__bss_stop - &__bss_start));

}
#endif

//==========================================================================
// execKernel - Load the kernel image (mach-o) and jump to its entry point.

static int ExecKernel(void *binary)
{
	int			ret;
	entry_t		kernelEntry;

	bootArgs->kaddr = bootArgs->ksize = 0;
	execute_hook("ExecKernel", (void*)binary, NULL, NULL, NULL);

	ret = DecodeKernel(binary,
					   &kernelEntry,
					   (char **) &bootArgs->kaddr,
					   (int *)&bootArgs->ksize );

	if ( ret != 0 )
		return ret;

	// Reserve space for boot args
	reserveKernBootStruct();

	// Notify modules that the kernel has been decoded
	execute_hook("DecodedKernel", (void*)binary, (void*)bootArgs->kaddr, (void*)bootArgs->ksize, NULL);

	setupFakeEfi();

	// Load boot drivers from the specifed root path.
	//if (!gHaveKernelCache)
	LoadDrivers("/");

	execute_hook("DriversLoaded", (void*)binary, NULL, NULL, NULL);

    clearActivityIndicator();
    
	if (gErrors) {
		printf("Errors encountered while starting up the computer.\n");
		printf("Pausing %d seconds...\n", kBootErrorTimeout);
		sleep(kBootErrorTimeout);
	}

	md0Ramdisk();

	verbose("Starting Darwin %s\n",( archCpuType == CPU_TYPE_I386 ) ? "x86" : "x86_64");
	verbose("Boot Args: %s\n", bootArgs->CommandLine);

	// Cleanup the PXE base code.

	if ( (gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit ) {
		if ( (ret = nbpUnloadBaseCode()) != nbpStatusSuccess ) {
			printf("nbpUnloadBaseCode error %d\n", (int) ret);
			sleep(2);
		}
	}

	bool dummyVal;
	if (getBoolForKey(kWaitForKeypressKey, &dummyVal, &bootInfo->chameleonConfig) && dummyVal) {
		showTextBuffer(msgbuf, strlen(msgbuf));
	}
    
	setupBooterLog();

	finalizeBootStruct();

	// Jump to kernel's entry point. There's no going back now.
	if ((checkOSVersion("10.7")) || (checkOSVersion("10.8")) || (checkOSVersion("10.9")))
	{

		// Notify modules that the kernel is about to be started
		execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgs, NULL, NULL);

		// Masking out so that Lion doesn't doublefault
		outb(0x21, 0xff);	/* Maskout all interrupts Pic1 */
		outb(0xa1, 0xff);	/* Maskout all interrupts Pic2 */

		startprog( kernelEntry, bootArgs );
	} else {
		// Notify modules that the kernel is about to be started
		execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgsPreLion, NULL, NULL);

		startprog( kernelEntry, bootArgsPreLion );
	}

	// Not reached
	return 0;
}


//==========================================================================
// LoadKernelCache - Try to load Kernel Cache.
// return the length of the loaded cache file or -1 on error
long LoadKernelCache(const char* cacheFile, void **binary)
{
	char		kernelCacheFile[512];
	char		kernelCachePath[512];
	long		flags, time, cachetime, kerneltime, exttime, ret=-1;
	unsigned long adler32;

	if((gBootMode & kBootModeSafe) != 0) {
		verbose("Kernel Cache ignored.\n");
		return -1;
	}

	// Use specify kernel cache file if not empty
	if (cacheFile[0] != 0)
	{
		strlcpy(kernelCacheFile, cacheFile, sizeof(kernelCacheFile));
	}
	else
	{
		// Lion, Mountain Lion and Mavericks prelink kernel cache file
		if ((checkOSVersion("10.7")) || (checkOSVersion("10.8")) || (checkOSVersion("10.9")))
		{
			snprintf(kernelCacheFile, sizeof(kernelCacheFile), "%skernelcache", kDefaultCachePathSnow);
		}
		// Snow Leopard prelink kernel cache file
		else if (checkOSVersion("10.6")) {
			snprintf(kernelCacheFile, sizeof(kernelCacheFile), "kernelcache_%s",
				(archCpuType == CPU_TYPE_I386) ? "i386" : "x86_64");
			int lnam = strlen(kernelCacheFile) + 9; //with adler32

			char* name;
			long prev_time = 0;

			struct dirstuff* cacheDir = opendir(kDefaultCachePathSnow);
			/* TODO: handle error? */
			if (cacheDir) {
				while(readdir(cacheDir, (const char**)&name, &flags, &time) >= 0) {
					if (((flags & kFileTypeMask) != kFileTypeDirectory) && time > prev_time
						&& strstr(name, kernelCacheFile) && (name[lnam] != '.')) {
						snprintf(kernelCacheFile, sizeof(kernelCacheFile), "%s%s", kDefaultCachePathSnow, name);
						prev_time = time;
					}
				}
			}
			closedir(cacheDir);
		} else {
			// Reset cache name.
			bzero(gCacheNameAdler + 64, sizeof(gCacheNameAdler) - 64);
			snprintf(gCacheNameAdler + 64, sizeof(gCacheNameAdler) - 64,
				"%s,%s",
				gRootDevice, bootInfo->bootFile);
			adler32 = Adler32((unsigned char *)gCacheNameAdler, sizeof(gCacheNameAdler));
			snprintf(kernelCacheFile, sizeof(kernelCacheFile), "%s.%08lX", kDefaultCachePathLeo, adler32);
		}
	}

	// Check if the kernel cache file exists
	ret = -1;

	// If boot from a boot helper partition check the kernel cache file on it
	if (gBootVolume->flags & kBVFlagBooter) {
		snprintf(kernelCachePath, sizeof(kernelCachePath), "com.apple.boot.P%s", kernelCacheFile);
		ret = GetFileInfo(NULL, kernelCachePath, &flags, &cachetime);
		if ((ret == -1) || ((flags & kFileTypeMask) != kFileTypeFlat)) {
			snprintf(kernelCachePath, sizeof(kernelCachePath), "com.apple.boot.R%s", kernelCacheFile);
			ret = GetFileInfo(NULL, kernelCachePath, &flags, &cachetime);
			if ((ret == -1) || ((flags & kFileTypeMask) != kFileTypeFlat)) {
				snprintf(kernelCachePath, sizeof(kernelCachePath), "com.apple.boot.S%s", kernelCacheFile);
				ret = GetFileInfo(NULL, kernelCachePath, &flags, &cachetime);
				if ((flags & kFileTypeMask) != kFileTypeFlat) {
					ret = -1;
				}
			}
		}
	}
	// If not found, use the original kernel cache path.
	if (ret == -1) {
		strlcpy(kernelCachePath, kernelCacheFile, sizeof(kernelCachePath));
		ret = GetFileInfo(NULL, kernelCachePath, &flags, &cachetime);
		if ((flags & kFileTypeMask) != kFileTypeFlat) {
			ret = -1;
		}
	}

	// Exit if kernel cache file wasn't found
	if (ret == -1) {
		verbose("No Kernel Cache File '%s' found\n", kernelCacheFile);
		return -1;
	}

	// Check if the kernel cache file is more recent (mtime)
	// than the kernel file or the S/L/E directory
	ret = GetFileInfo(NULL, bootInfo->bootFile, &flags, &kerneltime);
	// Check if the kernel file is more recent than the cache file
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeFlat)
		&& (kerneltime > cachetime)) {
		verbose("Kernel file (%s) is more recent than KernelCache (%s), ignoring KernelCache\n",
				bootInfo->bootFile, kernelCacheFile);
		return -1;
	}

	ret = GetFileInfo("/System/Library/", "Extensions", &flags, &exttime);
	// Check if the S/L/E directory time is more recent than the cache file
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)
		&& (exttime > cachetime)) {
		verbose("/System/Library/Extensions is more recent than KernelCache (%s), ignoring KernelCache\n",
				kernelCacheFile);
		return -1;
	}

	// Since the kernel cache file exists and is the most recent try to load it
	verbose("Loading kernel cache %s\n", kernelCachePath);

	ret = LoadThinFatFile(kernelCachePath, binary);
	return ret; // ret contain the length of the binary
}

//==========================================================================
// The 'main' function for the booter. Called by boot0 when booting
// from a block device, or by the network booter.
//
// arguments:
//	 biosdev - Value passed from boot1/NBP to specify the device
//			   that the booter was loaded from.
//
// If biosdev is kBIOSDevNetwork, then this function will return if
// booting was unsuccessful. This allows the PXE firmware to try the
// next boot device on its list.
void common_boot(int biosdev)
{
	bool	 		quiet;
	bool	 		firstRun = true;
	bool	 		instantMenu;
	bool	 		rescanPrompt;
	int				status;
	unsigned int	allowBVFlags = kBVFlagSystemVolume | kBVFlagForeignBoot;
	unsigned int	denyBVFlags = kBVFlagEFISystem;

	// Set reminder to unload the PXE base code. Neglect to unload
	// the base code will result in a hang or kernel panic.
	gUnloadPXEOnExit = true;

	// Initialize boot info structure.
	initKernBootStruct();

	initBooterLog();

	// Scan and record the system's hardware information.
	scan_platform();


	// Load boot.plist config file
	status = loadChameleonConfig(&bootInfo->chameleonConfig);

	if (getBoolForKey(kQuietBootKey, &quiet, &bootInfo->chameleonConfig) && quiet) {
		gBootMode |= kBootModeQuiet;
	}

	// Override firstRun to get to the boot menu instantly by setting "Instant Menu"=y in system config
	if (getBoolForKey(kInstantMenuKey, &instantMenu, &bootInfo->chameleonConfig) && instantMenu) {
		firstRun = false;
	}

	setBootGlobals(bvChain);

	// Parse args, load and start kernel.
	while (1)
	{
		bool		tryresume, tryresumedefault, forceresume;
		bool		useKernelCache = true; // by default try to use the prelinked kernel
		const char	*val;
		int			len, ret = -1;
		long		flags, sleeptime, time;
		void		*binary = (void *)kLoadAddr;

		char        bootFile[sizeof(bootInfo->bootFile)];
		char		bootFilePath[512];
		char		kernelCacheFile[512];

		// Initialize globals.
		sysConfigValid = false;
		gErrors		   = false;

		status = getBootOptions(firstRun);
		firstRun = false;
		if (status == -1) continue;

		status = processBootOptions();
		// Status == 1 means to chainboot
		if ( status ==	1 ) break;
		// Status == -1 means that the config file couldn't be loaded or that gBootVolume is NULL
		if ( status == -1 )
		{
			// gBootVolume == NULL usually means the user hit escape.
			if (gBootVolume == NULL)
			{
				freeFilteredBVChain(bvChain);
				
				if (gEnableCDROMRescan)
					rescanBIOSDevice(gBIOSDev);
				
				bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);
				setBootGlobals(bvChain);
			}
			continue;
		}

		// Other status (e.g. 0) means that we should proceed with boot.

        execute_hook("ClearScreen", NULL, NULL, NULL, NULL);

		// Find out which version mac os we're booting.
		getOSVersion();

		if (platformCPUFeature(CPU_FEATURE_EM64T)) {
			archCpuType = CPU_TYPE_X86_64;
		} else {
			archCpuType = CPU_TYPE_I386;
		}

		if (getValueForKey(karch, &val, &len, &bootInfo->chameleonConfig)) {
			if (strncmp(val, "i386", 4) == 0) {
				archCpuType = CPU_TYPE_I386;
			}
		}

		if (getValueForKey(kKernelArchKey, &val, &len, &bootInfo->chameleonConfig)) {
			if (strncmp(val, "i386", 4) == 0) {
				archCpuType = CPU_TYPE_I386;
			}
		}

		// Notify modules that we are attempting to boot
		execute_hook("PreBoot", NULL, NULL, NULL, NULL);

		if (!getBoolForKey (kWake, &tryresume, &bootInfo->chameleonConfig)) {
			tryresume = true;
			tryresumedefault = true;
		} else {
			tryresumedefault = false;
		}

		if (!getBoolForKey (kForceWake, &forceresume, &bootInfo->chameleonConfig)) {
			forceresume = false;
		}

		if (forceresume) {
			tryresume = true;
			tryresumedefault = false;
		}

		while (tryresume) {
			const char *tmp;
			BVRef bvr;
			if (!getValueForKey(kWakeImage, &val, &len, &bootInfo->chameleonConfig))
				val = "/private/var/vm/sleepimage";

			// Do this first to be sure that root volume is mounted
			ret = GetFileInfo(0, val, &flags, &sleeptime);

			if ((bvr = getBootVolumeRef(val, &tmp)) == NULL)
				break;

			// Can't check if it was hibernation Wake=y is required
			if (bvr->modTime == 0 && tryresumedefault)
				break;

			if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
				break;

			if (!forceresume && ((sleeptime+3)<bvr->modTime)) {
#if DEBUG
				printf ("Hibernate image is too old by %d seconds. Use ForceWake=y to override\n",
						bvr->modTime-sleeptime);
#endif
				break;
			}

			HibernateBoot((char *)val);
			break;
		}
		
		verbose("Loading Darwin %s\n", gMacOSVersion);

		getBoolForKey(kUseKernelCache, &useKernelCache, &bootInfo->chameleonConfig);
		if (useKernelCache) do {

			// Determine the name of the Kernel Cache
			if (getValueForKey(kKernelCacheKey, &val, &len, &bootInfo->bootConfig)) {
				if (val[0] == '\\')
				{
					len--;
					val++;
				}
				/* FIXME: check len vs sizeof(kernelCacheFile) */
				strlcpy(kernelCacheFile, val, len + 1);
			} else {
				kernelCacheFile[0] = 0; // Use default kernel cache file
			}

			if (gOverrideKernel && kernelCacheFile[0] == 0) {
				verbose("Using a non default kernel (%s) without specifying 'Kernel Cache' path, KernelCache will not be used\n",
						bootInfo->bootFile);
				useKernelCache = false;
				break;
			}
			if (gMKextName[0] != 0) {
				verbose("Using a specific MKext Cache (%s), KernelCache will not be used\n",
						gMKextName);
				useKernelCache = false;
				break;
			}
			if (gBootFileType != kBlockDeviceType)
				useKernelCache = false;

		} while(0);

		do {
			if (useKernelCache) {
				ret = LoadKernelCache(kernelCacheFile, &binary);
				if (ret >= 0)
					break;
			}

			bool bootFileWithDevice = false;
			// Check if bootFile start with a device ex: bt(0,0)/Extra/mach_kernel
			if (strncmp(bootInfo->bootFile,"bt(",3) == 0 ||
				strncmp(bootInfo->bootFile,"hd(",3) == 0 ||
				strncmp(bootInfo->bootFile,"rd(",3) == 0)
				bootFileWithDevice = true;

			// bootFile must start with a / if it not start with a device name
			if (!bootFileWithDevice && (bootInfo->bootFile)[0] != '/')
				snprintf(bootFile, sizeof(bootFile), "/%s", bootInfo->bootFile); // append a leading /
			else
				strlcpy(bootFile, bootInfo->bootFile, sizeof(bootFile));

			// Try to load kernel image from alternate locations on boot helper partitions.
			ret = -1;
			if ((gBootVolume->flags & kBVFlagBooter) && !bootFileWithDevice) {
				snprintf(bootFilePath, sizeof(bootFilePath), "com.apple.boot.P%s", bootFile);
				ret = GetFileInfo(NULL, bootFilePath, &flags, &time);
				if (ret == -1)
				{
					snprintf(bootFilePath, sizeof(bootFilePath), "com.apple.boot.R%s", bootFile);
					ret = GetFileInfo(NULL, bootFilePath, &flags, &time);
					if (ret == -1)
					{
						snprintf(bootFilePath, sizeof(bootFilePath), "com.apple.boot.S%s", bootFile);
						ret = GetFileInfo(NULL, bootFilePath, &flags, &time);
					}
				}
			}
			if (ret == -1) {
				// No alternate location found, using the original kernel image path.
				strlcpy(bootFilePath, bootFile, sizeof(bootFilePath));
			}

			verbose("Loading kernel %s\n", bootFilePath);
			ret = LoadThinFatFile(bootFilePath, &binary);
			if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
			{
				archCpuType = CPU_TYPE_I386;
				ret = LoadThinFatFile(bootFilePath, &binary);
			}
		} while (0);

        clearActivityIndicator();
        
#if DEBUG
		printf("Pausing...");
		sleep(8);
#endif

		if (ret <= 0) {
			printf("Can't find %s\n", bootFile);
			sleep(1);

			if (gBootFileType == kNetworkDeviceType) {
				// Return control back to PXE. Don't unload PXE base code.
				gUnloadPXEOnExit = false;
				break;
			}
			pause();

		} else {
			/* Won't return if successful. */
			ret = ExecKernel(binary);
		}
	}
	
	if ((gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit) {
		nbpUnloadBaseCode();
	}
}

/*!
	Selects a new BIOS device, taking care to update the global state appropriately.
 */
/*
static void selectBiosDevice(void)
{
	struct DiskBVMap *oldMap = diskResetBootVolumes(gBIOSDev);
	CacheReset();
	diskFreeMap(oldMap);
	oldMap = NULL;
	
	int dev = selectAlternateBootDevice(gBIOSDev);
	
	BVRef bvchain = scanBootVolumes(dev, 0);
	BVRef bootVol = selectBootVolume(bvchain);
	gBootVolume = bootVol;
	setRootVolume(bootVol);
	gBIOSDev = dev;
}
*/

bool checkOSVersion(const char * version) 
{
	return ((gMacOSVersion[0] == version[0]) && (gMacOSVersion[1] == version[1])
			&& (gMacOSVersion[2] == version[2]) && (gMacOSVersion[3] == version[3]));
}

static void getOSVersion()
{
	strncpy(gMacOSVersion, gBootVolume->OSVersion, sizeof(gMacOSVersion));
}

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5000
// NMAX (was 5521) the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define DO1(buf, i)	{s1 += buf[i]; s2 += s1;}
#define DO2(buf, i)	DO1(buf, i); DO1(buf, i + 1);
#define DO4(buf, i)	DO2(buf, i); DO2(buf, i + 2);
#define DO8(buf, i)	DO4(buf, i); DO4(buf, i + 4);
#define DO16(buf)	DO8(buf, 0); DO8(buf, 8);

unsigned long Adler32(unsigned char *buf, long len)
{
	unsigned long s1 = 1; // adler & 0xffff;
	unsigned long s2 = 0; // (adler >> 16) & 0xffff;
	unsigned long result;
	int k;

	while (len > 0) {
		k = len < NMAX ? len : NMAX;
		len -= k;
		while (k >= 16) {
			DO16(buf);
			buf += 16;
			k -= 16;
		}
		if (k != 0) do {
			s1 += *buf++;
			s2 += s1;
		} while (--k);
		s1 %= BASE;
		s2 %= BASE;
	}
	result = (s2 << 16) | s1;
	return OSSwapHostToBigInt32(result);
}
