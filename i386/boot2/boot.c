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
#include "sl.h"
#include "libsa.h"
#include "modules.h"
#include "vers.h"
/*
 * How long to wait (in seconds) to load the
 * kernel after displaying the "boot:" prompt.
 */
#define kBootErrorTimeout 5

bool		gOverrideKernel, gEnableCDROMRescan, gScanSingleDrive;

char		gRootDevice[ROOT_DEVICE_SIZE];
char		gMKextName[512];
char		gMacOSVersion[8];
int		bvCount = 0, gDeviceCount = 0;
//int		menucount = 0;
long		gBootMode; /* defaults to 0 == kBootModeNormal */
BVRef		bvr, menuBVR, bvChain;


//==========================================================================
// Zero the BSS.

static void zeroBSS(void)
{
	extern char*  __bss_stop;
	extern char*  __bss_start;
	bzero(&__bss_start, (&__bss_stop - &__bss_start));
}

typedef void (*linker_function_t)();
static void callInit(void)
{
    extern linker_function_t __init_array_start;
    extern linker_function_t __init_array_end;
    
    linker_function_t *fn = &__init_array_start;
    linker_function_t *end = &__init_array_end;
    while(fn < end)
    {
        fn[0]();
        fn++;
    }
    
}


//==========================================================================
// Malloc error function

static void malloc_error(char *addr, size_t size, const char *file, int line)
{
	stop("\nMemory allocation error! Addr: 0x%x, Size: 0x%x, File: %s, Line: %d\n",
									 (unsigned)addr, (unsigned)size, file, line);
}

//==========================================================================
//Initializes the runtime. Right now this means zeroing the BSS and initializing malloc.
//
void initialize_runtime(void)
{
	zeroBSS();
    malloc_init(0, 0, 0, malloc_error);
    // init needs malloc
    callInit();
}
//==========================================================================
// This is the entrypoint from real-mode which functions exactly as it did
// before. Multiboot does its own runtime initialization, does some of its
// own things, and then calls common_boot.
void boot(int biosdev)
{
	initialize_runtime();
	// Enable A20 gate before accessing memory above 1Mb.
	enableA20();
	common_boot(biosdev);
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

	// Record the device that the booter was loaded from.
	gBIOSDev = biosdev & kBIOSDevMask;

	// Setup VGA text mode.
#if DEBUG
	printf("before video_mode\n");
#endif
	video_mode( 2 );  // 80x25 mono text mode.
#if DEBUG
	printf("after video_mode\n");
#endif

	// First get info for boot volume.
	scanBootVolumes(gBIOSDev, 0);
	bvChain = getBVChainForBIOSDev(gBIOSDev);
	setBootGlobals(bvChain);

    // Scan boot partition only
    scanBootVolumes(gBIOSDev, &bvCount);
    // Scan all found disks
    //scanDisks(gBIOSDev, &bvCount);
	
	// Create a separated bvr chain using the specified filters.
	bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);

	gBootVolume = selectBootVolume(bvChain);

	// Intialize module system 
	init_module_system();

    execute_hook("InitDone", NULL, NULL, NULL, NULL);
    printf("Chameleon %s build %s\n", I386BOOT_CHAMELEONVERSION, I386BOOT_CHAMELEONREVISION);
    
    extern void llvm_do_exit();
    llvm_do_exit();
    while(1) execute_hook("Main", (void*) biosdev, NULL, NULL, NULL);
}
