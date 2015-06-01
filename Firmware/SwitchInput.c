/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * SwitchInput.c
 *
 * Polling switch inputs SW0..SW3.
 */

#include "MIDIWidget.h"

void ReadSwitches()
{
    uint8_t CHANGES;

    // read in the switch states
    SW_STATES = 0x0F;
    if ((PIND & 0b00010000) == 0)
    {
        SW_STATES &= 0b11111110;
    }
    if ((PIND & 0b00000010) == 0)
    {
        SW_STATES &= 0b11111101;
    }
    if ((PIND & 0b00000001) == 0)
    {
        SW_STATES &= 0b11111011;
    }
    if ((PINC & 0b00000100) == 0)
    {
        SW_STATES &= 0b11110111;
    }

    // identify falling edge changes
    CHANGES = (~(SW_STATES ^ SW_STATES_PREV)) | SW_STATES;

    // handle new switch closures
    if ((CHANGES & 0b00000001) == 0)
    {
        // store preset
        StorePreset();
        ACTBlink();
        ACTOn();
    }
    if ((CHANGES & 0b00000010) == 0)
    {
    }
    if ((CHANGES & 0b00000100) == 0)
    {
    }
    if ((CHANGES & 0b00001000) == 0)
    {
    }

    // store states for next compare
    SW_STATES_PREV = SW_STATES;
}
