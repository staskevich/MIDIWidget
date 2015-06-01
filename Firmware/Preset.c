/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * Preset.c
 *
 * Preset-related functions.
 */ 

#include "MIDIWidget.h"

void RecallPreset(uint8_t number)
{
    uint8_t * PRESET_ADDRESS = PRESET_SLOTS + ((number%NUM_PRESETS)*PRESET_SLOT_SIZE);
    T0_INT_OFF();
    LATCH[0] = (LATCH[0] & (~LATCH0_PRESET_FLAGS)) | (LATCH0_PRESET_FLAGS & eeprom_read_byte(PRESET_ADDRESS++));
    LATCH[1] = (LATCH[1] & (~LATCH1_PRESET_FLAGS)) | (LATCH1_PRESET_FLAGS & eeprom_read_byte(PRESET_ADDRESS++));
    LATCH[2] = (LATCH[2] & (~LATCH2_PRESET_FLAGS)) | (LATCH2_PRESET_FLAGS & eeprom_read_byte(PRESET_ADDRESS));
    T0_INT_ON();
    WriteLatches();
    CURRENT_PRESET = number;
}

void StorePreset()
{
    uint8_t snapshot0, snapshot1, snapshot2;
    uint8_t * TEMPEEADR = PRESET_SLOTS + ((CURRENT_PRESET%NUM_PRESETS) * PRESET_SLOT_SIZE);

    T0_INT_OFF();
    snapshot0 = LATCH[0];
    snapshot1 = LATCH[1];
    snapshot2 = LATCH[2];
    T0_INT_ON();
    eeprom_update_byte(TEMPEEADR, snapshot0);
    TEMPEEADR++;
    eeprom_update_byte(TEMPEEADR, snapshot1);
    TEMPEEADR++;
    eeprom_update_byte(TEMPEEADR, snapshot2);
}
