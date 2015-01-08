/*
 * Copyright (c) 1999-2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2004 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
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

#include "boot.h"
#include "fdisk.h"
#include "term.h"
#include "embedded.h"
#include "modules.h"



// This is a very simplistic prompting scheme that just grabs two hex characters
// Eventually we need to do something more user-friendly like display a menu
// based off of the Multiboot device list

int selectAlternateBootDevice(int bootdevice)
{
    int key;
    int newbootdevice;
    int digitsI = 0;
    char *end;
    char digits[3] = {0,0,0};

    // We've already printed the current boot device so user knows what it is
    printf("Typical boot devices are 80 (First HD), 81 (Second HD)\n");
    printf("Enter two-digit hexadecimal boot device [%02x]: ", bootdevice);
    do {
        key = getchar();
        switch (ASCII_KEY(key)) {
        case KEY_BKSP:
            if (digitsI > 0) {
                int x, y, t;
                getCursorPositionAndType(&x, &y, &t);
                // Assume x is not 0;
                x--;
                setCursorPosition(x,y,0); // back up one char
                // Overwrite with space without moving cursor position
                putca(' ', 0x07, 1);
                digitsI--;
            } else {
                // TODO: Beep or something
            }
            break;

        case KEY_ENTER:
            digits[digitsI] = '\0';
            newbootdevice = strtol(digits, &end, 16);
            if (end == digits && *end == '\0') {
                // User entered empty string
                printf("\nUsing default boot device %x\n", bootdevice);
                key = 0;
            } else if(end != digits && *end == '\0') {
                bootdevice = newbootdevice;
                printf("\n");
                key = 0; // We gots da boot device
            } else {
                printf("\nCouldn't parse. try again: ");
                digitsI = 0;
            }
            break;

        default:
            if (isxdigit(ASCII_KEY(key)) && digitsI < 2) {
                putchar(ASCII_KEY(key));
                digits[digitsI++] = ASCII_KEY(key);
            } else {
                // TODO: Beep or something
            }
            break;
        };
    } while (key != 0);

    return bootdevice;
}

static int currentIndicator = 0;
static char indicator[] = {'-', '\\', '|', '/', '\0'};


void
spinActivityIndicator(int sectors)
{
    if (currentIndicator >= sizeof(indicator))
    {
        currentIndicator = 0;
    }
    putchar(indicator[currentIndicator++]);
    putchar('\b');
}
