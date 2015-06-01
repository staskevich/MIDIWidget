/*
   LUFA Library
   Copyright (C) Dean Camera, 2014.

   dean [at] fourwalledcubicle [dot] com
   www.lufa-lib.org
*/

/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 */

#include "MIDIWidget.h"

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface =
{
    .Config =
    {
        .StreamingInterfaceNumber = INTERFACE_ID_AudioStream,
        .DataINEndpoint           =
        {
            .Address          = MIDI_STREAM_IN_EPADDR,
            .Size             = MIDI_STREAM_EPSIZE,
            .Banks            = 1,
        },
        .DataOUTEndpoint          =
        {
            .Address          = MIDI_STREAM_OUT_EPADDR,
            .Size             = MIDI_STREAM_EPSIZE,
            .Banks            = 1,
        },
    },
};


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupUSB(void)
{
#if (ARCH == ARCH_AVR8)
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
    /* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
    XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
    XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

    /* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
    XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
    XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

    PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

    /* Hardware Initialization */
    USB_Init();
}

void USBSendMIDIMessage(uint8_t Status, uint8_t d0, uint8_t d1)
{
    MIDI_EventPacket_t MIDIEvent = (MIDI_EventPacket_t)
    {
//        .Event       = MIDI_EVENT(0, Status & 0b11110000),
        .Event = Status >> 4,
        .Data1 = Status,
        .Data2 = d0,
        .Data3 = d1,
    };
    MIDI_Device_SendEventPacket(&Keyboard_MIDI_Interface, &MIDIEvent);
    MIDI_Device_Flush(&Keyboard_MIDI_Interface);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}

