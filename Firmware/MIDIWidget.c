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
 * MIDIWidget.c
 *
 * Main program loop.
 */

#include "MIDIWidget.h"

uint8_t LATCH[NUM_LATCHES];
uint8_t LATCH0_INV;
uint8_t LATCH1_INV;
uint8_t LATCH2_INV;
uint8_t BLINK_COUNTER;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    LatchInit();
    PortInit();
    WriteLatches();
    OutputEnable();

    STBYOn();
    ACTOn();

    USARTInit();
    Timer0Init();
    SetupUSB();

    GlobalVariableInit();
    ConfigVariableInit();

    GlobalInterruptEnable();

    for (;;)
    {
        MIDI_EventPacket_t ReceivedMIDIEvent;
        while (MIDI_Device_ReceiveEventPacket(&Keyboard_MIDI_Interface, &ReceivedMIDIEvent))
        {
            switch (ReceivedMIDIEvent.Event & 0b00001111)
            {
                case 0x08:
                    ProcessNoteOff(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    break;
                case 0x09:
                    ProcessNoteOn(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    break;
                case 0x0B:
                    if (ReceivedMIDIEvent.Data1 == 0xBF)
                    {
                        ProcessConfigController(ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    ProcessController(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    if (!(OPERATION_FLAGS & ((1<<DISABLE_USB_MIDI_THRU_FLAG) | (1<<CONFIG_MODE_FLAG))))
                    {
                        QueueMIDIEvent(3, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    break;
                case 0x0C:
                    ProcessProgramChange(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(2, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, 0x00);
                    }
                    break;
                case 0x0E:
                    ProcessPitchWheel(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    break;
                case 0x0A:
                    ProcessAftertouch(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
                    }
                    break;
                case 0x0D:
                    ProcessChannelPressure(ReceivedMIDIEvent.Data1 & 0x0F, ReceivedMIDIEvent.Data2);
                    if (!(OPERATION_FLAGS & (1<<DISABLE_USB_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(2, ReceivedMIDIEvent.Data1, ReceivedMIDIEvent.Data2, 0x00);
                    }
                    break;
                case 0x0F:
                    ParseMIDIByte(ReceivedMIDIEvent.Data1, STREAM_USB_MIDI);
                    break;
                default:
                    break;
            }
        }
        MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
        USB_USBTask();
        ProcessSerialMIDI();
        // every 256 cycles, update switch states
        // this will have debounce effect
        SWITCH_DEBOUNCE_COUNTER++;
        if (SWITCH_DEBOUNCE_COUNTER == 0)
        {
            ReadSwitches();
        }
    }
}

