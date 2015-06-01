/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * Latch.c
 *
 * Functions related to MIDIWidget output latches.
 */

#include "MIDIWidget.h"

void WriteLatches()
{

    if ((OPERATION_FLAGS & (1<<DEFER_LATCH_WRITE)) != 0)
    {
        T0_INT_OFF();

        PORTB = LATCH[0] ^ LATCH0_INV;
        PORTD |= (1<<6);
        PORTD &= ~(1<<6);

        PORTB = LATCH[1] ^ LATCH1_INV;
        PORTC |= (1<<7);
        PORTC &= ~(1<<7);

        PORTB = LATCH[2] ^ LATCH2_INV;
        PORTC |= (1<<6);
        PORTC &= ~(1<<6);

        T0_INT_ON();
    }
}

void LatchInit()
{
    uint8_t i;
    uint8_t BITMASK;
    uint8_t TEMP;
    int CONFIG_OFFSET = 7;

    BITMASK = 0b00000001;
    TEMP = 0;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(OUTPUT_CONFIG+CONFIG_OFFSET) & 0b00000001)
        {
            TEMP |= BITMASK;
        }
        CONFIG_OFFSET += CONFIG_CHUNK_SIZE;
        BITMASK <<= 1;
    }
    LATCH[0] = 0;
    LATCH0_INV = TEMP;

    BITMASK = 0b00000001;
    TEMP = 0;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(OUTPUT_CONFIG+CONFIG_OFFSET) & 0b00000001)
        {
            TEMP |= BITMASK;
        }
        CONFIG_OFFSET += CONFIG_CHUNK_SIZE;
        BITMASK <<= 1;
    }
    LATCH[1] = 0;
    LATCH1_INV = TEMP;

    BITMASK = 0b00000001;
    TEMP = 0;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(OUTPUT_CONFIG+CONFIG_OFFSET) & 0b00000001)
        {
            TEMP |= BITMASK;
        }
        CONFIG_OFFSET += CONFIG_CHUNK_SIZE;
        BITMASK <<= 1;
    }
    LATCH[2] = 0x00;
    LATCH2_INV = TEMP;

    CONFIG_OFFSET = 0;
    for (i=0; i<NUM_OUTPUTS; i++)
    {
        // Load any configured pulse lengths
        DURATION[i] = 0;
        DURATION_CONFIG[i] = eeprom_read_byte(OUTPUT_CONFIG+CONFIG_OFFSET+4) * 128 + eeprom_read_byte(OUTPUT_CONFIG+CONFIG_OFFSET+5) << 3;
        // Init PWM 
        PWM[i] = 0xFF;
        CONFIG_OFFSET += CONFIG_CHUNK_SIZE;
    }
}

