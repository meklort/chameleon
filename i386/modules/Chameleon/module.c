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

static void main_app(void* device, void* arg1, void* arg2, void* arg3);

void Chameleon_init()
{
    register_hook_callback("Main", &main_app);
}

static void main_app(void* device, void* arg1, void* arg2, void* arg3)
{
    int dev = (int)device;
    extern void common_boot(int biosdev);
    common_boot(dev);
    
}