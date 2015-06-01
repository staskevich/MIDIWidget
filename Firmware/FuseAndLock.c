/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * FuseAndLock.c
 *
 * Microcontroller fuses and lock bits. Values may not appear in .hex file.
 */ 

#include "MIDIWidget.h"

FUSES =
{
	// low-power crystal osc, 16MHz, 65ms power-up delay
	.low = 0xFF,
	// ICSP enabled, bootloader size 2048 words, don't run bootloader at reset
	.high = (FUSE_SPIEN & FUSE_BOOTRST),
	// BOD at 3.0V, HWBE disabled
	.extended = (FUSE_BODLEVEL1 & FUSE_BODLEVEL0)
};

LOCKBITS = (LB_MODE_1 & BLB0_MODE_1 & BLB1_MODE_3);
