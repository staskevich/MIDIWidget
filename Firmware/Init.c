/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * Init.c
 *
 * Variable and microcontroller initialization routines.
 */

#include "MIDIWidget.h"

void GlobalVariableInit()
{
    BLINK_COUNTER = 0;
    PWM_COUNTER = 0;
    SERIAL_RX_BUFFER_HEAD = 0;
    SERIAL_RX_BUFFER_TAIL = 0;
    SERIAL_RX_BUFFER_FILL = 0;
    SERIAL_TX_BUFFER_HEAD = 0;
    SERIAL_TX_BUFFER_TAIL = 0;
    SERIAL_TX_BUFFER_FILL = 0;
    OPERATION_FLAGS = 0;
    CLOCK_COUNT_24 = 0;
    CLOCK_COUNT_12 = 0;
    CLOCK_COUNT_8 = 0;
    CLOCK_COUNT_6 = 0;
    CLOCK_COUNT_4 = 0;
    CLOCK_COUNT_3 = 0;
    CLOCK_COUNT_2 = 0;
    CURRENT_PRESET = 0;
    SW_STATES = 0b00001111;
    SW_STATES_PREV = 0b00001111;
}

void ConfigVariableInit()
{
    uint8_t bitmask;
    uint8_t i;
    uint8_t * EEAddress;

    PRESET_STORE_CHANNEL = eeprom_read_byte(&CONFIG_PRESET_STORE_CHANNEL);
    PRESET_STORE_TYPE = eeprom_read_byte(&CONFIG_PRESET_STORE_TYPE);
    PRESET_STORE_NUMBER = eeprom_read_byte(&CONFIG_PRESET_STORE_NUMBER);
    PRESET_RECALL_CHANNEL = eeprom_read_byte(&CONFIG_PRESET_RECALL_CHANNEL);
    PRESET_RECALL_TYPE = eeprom_read_byte(&CONFIG_PRESET_RECALL_TYPE);

    // OPERATION_FLAGS = eeprom_read_byte(&CONFIG_OPERATION_FLAGS);
    // if this is running, not in config mode.
    // OPERATION_FLAGS &= ~(1<<CONFIG_MODE_FLAG);
    OPERATION_FLAGS = 0x00;

    LATCH0_PRESET_FLAGS = 0x00;
    LATCH1_PRESET_FLAGS = 0x00;
    LATCH2_PRESET_FLAGS = 0x00;
    bitmask = 0b00000001;
    EEAddress = OUTPUT_CONFIG + 6;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(EEAddress) != 0)
        {
            LATCH0_PRESET_FLAGS |= bitmask;
        }
        EEAddress += CONFIG_CHUNK_SIZE;
        bitmask <<= 1;
    }
    bitmask = 0b00000001;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(EEAddress) != 0)
        {
            LATCH1_PRESET_FLAGS |= bitmask;
        }
        EEAddress += CONFIG_CHUNK_SIZE;
        bitmask <<= 1;
    }
    bitmask = 0b00000001;
    for (i=0; i<8; i++)
    {
        if (eeprom_read_byte(EEAddress) != 0)
        {
            LATCH2_PRESET_FLAGS |= bitmask;
        }
        EEAddress += CONFIG_CHUNK_SIZE;
        bitmask <<= 1;
    }
}

void PortInit()
{
    // Port B is unidirectional data bus
    DDRB = 0b11111111;
    // Port C
    PORTC = 0b00000100;
    DDRC = 0b11110000;
    // Port D
    PORTD = 0b00110011;
    DDRD = 0b11000000;
}

void USARTInit()
{
    // USART Init
    // Set the baud rate
    // BAUD = fosc / (16 * (UBRR + 1))
    UBRR1 = 31;
    // Enable RX and TX
    UCSR1B = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1);
    // Format
    UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);

    // SOFTWARE_THRU = 0x01;
}

void Timer0Init()
{
    // scaling
    TCCR0B = (1<<CS01);
// 1ms period
//    TCCR0B = (1<<CS01)|(1<<CS00);
    // overflow interrupt
    TIMSK0 = (1<<TOIE0);
}

