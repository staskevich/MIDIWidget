/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * ProcessMIDI.c
 *
 * Parse MIDI data streams and handle MIDI events.
 */ 

#include "MIDIWidget.h"

uint8_t SERIAL_RX_BUFFER[SERIAL_RX_BUFFER_SIZE];
uint8_t SERIAL_RX_BUFFER_HEAD;
uint8_t SERIAL_RX_BUFFER_TAIL;
uint8_t SERIAL_RX_BUFFER_FILL;
uint8_t SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_SIZE];
uint8_t SERIAL_TX_BUFFER_HEAD;
uint8_t SERIAL_TX_BUFFER_TAIL;
uint8_t SERIAL_TX_BUFFER_FILL;

// Read MIDI bytes from the combined RX/USB queue and:
// 1. echo MIDI byte at serial port (if enabled)
// 2. update output states as necessary.
void ProcessSerialMIDI()
{
    uint8_t MIDI_BYTE;

    // check for empty FIFO
    if (SERIAL_RX_BUFFER_FILL != 0)
    {
        // pull byte from FIFO
        cli();
        MIDI_BYTE = SERIAL_RX_BUFFER[SERIAL_RX_BUFFER_HEAD++];
        SERIAL_RX_BUFFER_HEAD %= SERIAL_RX_BUFFER_SIZE;
        SERIAL_RX_BUFFER_FILL--;
        sei();

        ParseMIDIByte(MIDI_BYTE, STREAM_SERIAL_MIDI);
    }
}

void ParseMIDIByte(uint8_t MIDI_BYTE, uint8_t s)
{
    static uint8_t RUNNING_STATUS[NUM_UNPARSED_STREAMS] = {0x00, 0x00};
    static uint8_t BYTE_COUNT[NUM_UNPARSED_STREAMS] = {0x00, 0x00};
    static uint8_t DATA_0[NUM_UNPARSED_STREAMS] = {0x00, 0x00};
    // status byte or data byte?
    if (MIDI_BYTE & 0b10000000)
    {
        // status byte. handle in 3 categories: system real time, channel-specific, system common.
        if (MIDI_BYTE >= 0xF8)
        {
            ProcessMIDIRealTime(MIDI_BYTE);
            if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
            {
                QueueMIDIEvent(1, MIDI_BYTE, 0, 0);
            }
        }
        else if (MIDI_BYTE < 0xF0)
        {
            // channel-specific status.
            RUNNING_STATUS[s] = MIDI_BYTE;
            BYTE_COUNT[s] = 0;
        }
        else
        {
            // system common.
            RUNNING_STATUS[s] = 0x00;
            BYTE_COUNT[s] = 0;
        }
    } // status byte
    else
    {
        // data byte.
        // is this the first data byte in the message?
        if (BYTE_COUNT[s] == 0)
        {
            // check for program change (single data byte)
            switch (RUNNING_STATUS[s] & 0xF0)
            {
                case 0xC0:
                    ProcessProgramChange(RUNNING_STATUS[s] & 0x0F, MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(2, RUNNING_STATUS[s],MIDI_BYTE,0);
                    }
                    break;
                case 0xD0:
                    ProcessChannelPressure(RUNNING_STATUS[s] & 0x0F, MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(2, RUNNING_STATUS[s],MIDI_BYTE,0);
                    }
                    break;
                default:
                    // store the first data byte and exit.
                    DATA_0[s] = MIDI_BYTE;
                    BYTE_COUNT[s]++;
                    break;
            }
        }
        else // BYTE_COUNT > 0
        {
            // reset bytecount for next message.
            BYTE_COUNT[s] = 0;
            // check for message type and process data.
            switch (RUNNING_STATUS[s] & 0xF0)
            {
                case 0x80:
                    ProcessNoteOff(RUNNING_STATUS[s] & 0x0F, DATA_0[s]);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, RUNNING_STATUS[s],DATA_0[s],MIDI_BYTE);
                    }
                    break;
                case 0x90:
                    ProcessNoteOn(RUNNING_STATUS[s] & 0x0F, DATA_0[s], MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, RUNNING_STATUS[s],DATA_0[s],MIDI_BYTE);
                    }
                    break;
                case 0xB0:
                    ProcessController(RUNNING_STATUS[s] & 0x0F, DATA_0[s], MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, RUNNING_STATUS[s],DATA_0[s],MIDI_BYTE);
                    }
                    break;
                case 0xA0:
                    ProcessAftertouch(RUNNING_STATUS[s] & 0x0F, DATA_0[s], MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, RUNNING_STATUS[s],DATA_0[s],MIDI_BYTE);
                    }
                    break;
                case 0xE0:
                    ProcessPitchWheel(RUNNING_STATUS[s] & 0x0F, DATA_0[s], MIDI_BYTE);
                    if (! (OPERATION_FLAGS & (1<<DISABLE_SERIAL_MIDI_THRU_FLAG)))
                    {
                        QueueMIDIEvent(3, RUNNING_STATUS[s],DATA_0[s],MIDI_BYTE);
                    }
                    break;
            }
        }
    } // Data byte
} // ProcessSerialMIDIBytes()

void ProcessNoteOff(uint8_t CHANNEL, uint8_t NOTE_NUMBER)
{
    uint8_t i, j;
    uint8_t output = 0;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    ACTBlink();
    ACTOn();
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            if ((eeprom_read_byte(CHUNK_ADDRESS+1) == CHANNEL) || (eeprom_read_byte(CHUNK_ADDRESS+1) == 0x10))
            {
                if (eeprom_read_byte(CHUNK_ADDRESS+2) == NOTE_NUMBER)
                /*
                if ((eeprom_read_byte(CHUNK_ADDRESS+2) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+8) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+9) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+10) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+11) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+12) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+13) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+14) == NOTE_NUMBER) ||
                    (eeprom_read_byte(CHUNK_ADDRESS+15) == NOTE_NUMBER))
                    */
                {
                    switch (eeprom_read_byte(CHUNK_ADDRESS))
                    {
                        case OUTPUT_MODE_NOTE:
                            T0_INT_OFF();
                            LATCH[j] &= (~BITMASK);
                            T0_INT_ON();
                            break;
                        case OUTPUT_MODE_PWM_NOTE_VELOCITY:
                            PWM[output] = 0xFF;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    switch (eeprom_read_byte(CHUNK_ADDRESS))
                    {
                        case OUTPUT_MODE_PWM_NOTE_NUMBER:
                            PWM[output] = 0xFF;
                            break;
                        default:
                            break;
                    }
                }
            }
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
            output++;
        }
    }
    WriteLatches();
}

void ProcessNoteOn(uint8_t CHANNEL, uint8_t NOTE_NUMBER, uint8_t VELOCITY)
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    if (VELOCITY == 0x00)
    {
        ProcessNoteOff(CHANNEL, NOTE_NUMBER);
    }
    else
    {
        ACTBlink();
        ACTOn();
        if ((PRESET_STORE_TYPE==MESSAGE_TYPE_NOTE) && ((CHANNEL==PRESET_STORE_CHANNEL) || (PRESET_STORE_CHANNEL==MIDI_CHANNEL_ANY)) && (NOTE_NUMBER==PRESET_STORE_NUMBER))
        {
            StorePreset();
        }
        else if ((PRESET_RECALL_TYPE==MESSAGE_TYPE_NOTE) && ((CHANNEL==PRESET_RECALL_CHANNEL) || (PRESET_RECALL_CHANNEL==MIDI_CHANNEL_ANY)))
        {
            RecallPreset(NOTE_NUMBER);
        }
        output = 0;
        for (j = 0; j < 3; j++)
        {
            BITMASK = 0b00000001;
            for (i = 0; i < 8; i++)
            {
                if ((eeprom_read_byte(CHUNK_ADDRESS+1) == CHANNEL) || (eeprom_read_byte(CHUNK_ADDRESS+1) == 0x10))
                {
                    if (eeprom_read_byte(CHUNK_ADDRESS+3) <= VELOCITY)
                    {
                        if (eeprom_read_byte(CHUNK_ADDRESS+2) == NOTE_NUMBER)
                        /*
                        if ((eeprom_read_byte(CHUNK_ADDRESS+2) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+8) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+9) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+10) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+11) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+12) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+13) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+14) == NOTE_NUMBER) ||
                            (eeprom_read_byte(CHUNK_ADDRESS+15) == NOTE_NUMBER)) */
                        {
                            switch (eeprom_read_byte(CHUNK_ADDRESS))
                            {
                                case OUTPUT_MODE_NOTE:
                                case OUTPUT_MODE_NOTE_MATCH_ONLY:
                                    T0_INT_OFF();
                                    LATCH[j] |= BITMASK;
                                    T0_INT_ON();
                                    break;
                                case OUTPUT_MODE_NOTE_TOGGLE:
                                    T0_INT_OFF();
                                    LATCH[j] ^= BITMASK;
                                    T0_INT_ON();
                                    break;
                                case OUTPUT_MODE_NOTE_FIXED:
                                    T0_INT_OFF();
                                    LATCH[j] |= BITMASK;
                                    DURATION[output] = DURATION_CONFIG[output];
                                    T0_INT_ON();
                                    break;
                                case OUTPUT_MODE_PWM_NOTE_VELOCITY:
                                    PWM[output] = VELOCITY;
                                    break;
                                case OUTPUT_MODE_PWM_NOTE_VELOCITY_FIXED:
                                    PWM[output] = VELOCITY;
                                    DURATION[output] = DURATION_CONFIG[output];
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (eeprom_read_byte(CHUNK_ADDRESS))
                            {
                                case OUTPUT_MODE_NOTE_MATCH_ONLY:
                                    T0_INT_OFF();
                                    LATCH[j] &= (~BITMASK);
                                    T0_INT_ON();
                                    break;
                                case OUTPUT_MODE_PWM_NOTE_NUMBER:
                                    PWM[output] = NOTE_NUMBER;
                                    break;
                                case OUTPUT_MODE_PWM_NOTE_NUMBER_FIXED:
                                    PWM[output] = NOTE_NUMBER;
                                    DURATION[output] = DURATION_CONFIG[output];
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                output++;
                BITMASK <<= 1;
                CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
            }
        }
        WriteLatches();
    }
}

void ProcessAftertouch(uint8_t CHANNEL, uint8_t NOTE_NUMBER, uint8_t VELOCITY)
{
    ACTBlink();
    ACTOn();
}


void ProcessController(uint8_t CHANNEL, uint8_t CC_NUMBER, uint8_t VALUE)
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    // config messages can only come via USB? (what about the unparsed stream)
    //    if (CHANNEL == 0x0F)
    //    {
    //        ProcessConfigController(NUMBER,VALUE);
    //    }

    // if (OPERATION_FLAGS != 0)
    if ((OPERATION_FLAGS & (1<<CONFIG_MODE_FLAG)) != 0)
    {
        return;
    }

    ACTBlink();
    ACTOn();

    if ((PRESET_STORE_TYPE==MESSAGE_TYPE_CC) && ((CHANNEL==PRESET_STORE_CHANNEL) || (PRESET_STORE_CHANNEL==MIDI_CHANNEL_ANY)) && (CC_NUMBER==PRESET_STORE_NUMBER) && (VALUE>=0x40))
    {
        StorePreset();
    }
    else if ((PRESET_RECALL_TYPE==MESSAGE_TYPE_CC) && ((CHANNEL==PRESET_RECALL_CHANNEL) || (PRESET_RECALL_CHANNEL==MIDI_CHANNEL_ANY)) && (VALUE>=0x40))
    {
        RecallPreset(CC_NUMBER);
    }

    output = 0;

    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            // channel match?
            if ((eeprom_read_byte(CHUNK_ADDRESS+1) == CHANNEL) || (eeprom_read_byte(CHUNK_ADDRESS+1) == 0x10))
            {
                // CC number match?
                if (eeprom_read_byte(CHUNK_ADDRESS+2) == CC_NUMBER)
                {
                    // CC value greater than or equal to threshold?
                    if (eeprom_read_byte(CHUNK_ADDRESS+3) <= VALUE)
                    {
                        switch (eeprom_read_byte(CHUNK_ADDRESS))
                        {
                            case OUTPUT_MODE_CC:
                            case OUTPUT_MODE_CC_MATCH_ONLY:
                                T0_INT_OFF();
                                LATCH[j] |= BITMASK;
                                T0_INT_ON();
                                break;
                            case OUTPUT_MODE_CC_TOGGLE:
                                T0_INT_OFF();
                                LATCH[j] ^= BITMASK;
                                T0_INT_ON();
                                break;
                            case OUTPUT_MODE_CC_FIXED:
                                T0_INT_OFF();
                                LATCH[j] |= BITMASK;
                                DURATION[output] = DURATION_CONFIG[output];
                                T0_INT_ON();
                                break;
                            case OUTPUT_MODE_PWM_CC:
                                PWM[output] = VALUE;
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        switch (eeprom_read_byte(CHUNK_ADDRESS))
                        {
                            case OUTPUT_MODE_CC:
                                T0_INT_OFF();
                                LATCH[j] &= ~BITMASK;
                                T0_INT_ON();
                                break;
                            default:
                                break;
                        }
                    }
                }
                else if ((eeprom_read_byte(CHUNK_ADDRESS)==OUTPUT_MODE_CC_MATCH_ONLY) && (eeprom_read_byte(CHUNK_ADDRESS+3) <= VALUE))
                {
                    T0_INT_OFF();
                    LATCH[j] &= ~BITMASK;
                    T0_INT_ON();
                }
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();
}

void ProcessProgramChange(uint8_t CHANNEL, uint8_t PROGRAM_NUMBER)
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    ACTBlink();
    ACTOn();

    if ((PRESET_STORE_TYPE==MESSAGE_TYPE_PC) && ((CHANNEL==PRESET_STORE_CHANNEL) || (PRESET_STORE_CHANNEL==MIDI_CHANNEL_ANY)) && (PROGRAM_NUMBER==PRESET_STORE_NUMBER))
    {
        StorePreset();
    }
    else if ((PRESET_RECALL_TYPE==MESSAGE_TYPE_PC) && ((CHANNEL==PRESET_RECALL_CHANNEL) || (PRESET_RECALL_CHANNEL==MIDI_CHANNEL_ANY)))
    {
        RecallPreset(PROGRAM_NUMBER);
    }

    output = 0;
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            if ((eeprom_read_byte(CHUNK_ADDRESS+1) == CHANNEL) || (eeprom_read_byte(CHUNK_ADDRESS+1) == 0x10))
            {
                if (eeprom_read_byte(CHUNK_ADDRESS+2) == PROGRAM_NUMBER)
                {
                    switch (eeprom_read_byte(CHUNK_ADDRESS))
                    {
                        case OUTPUT_MODE_PC_FIXED:
                            T0_INT_OFF();
                            LATCH[j] |= BITMASK;
                            DURATION[output] = DURATION_CONFIG[output];
                            T0_INT_ON();
                            break;
                        case OUTPUT_MODE_PC_MATCH_ONLY:
                            T0_INT_OFF();
                            LATCH[j] |= BITMASK;
                            T0_INT_ON();
                            break;
                    }
                }
                else if (eeprom_read_byte(CHUNK_ADDRESS) == OUTPUT_MODE_PC_MATCH_ONLY)
                {
                    T0_INT_OFF();
                    LATCH[j] &= (~BITMASK);
                    T0_INT_ON();
                }
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();
}

void ProcessChannelPressure(uint8_t CHANNEL, uint8_t NUMBER)
{
    ACTBlink();
    ACTOn();
}

void ProcessPitchWheel(uint8_t CHANNEL, uint8_t D0, uint8_t D1)
{
    ACTBlink();
    ACTOn();
}

void ProcessMIDIClock()
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    if (CLOCK_COUNT_24 == 0)
    {
        ACTBlink();
        ACTOn();
    }
    output = 0;
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            switch (eeprom_read_byte(CHUNK_ADDRESS))
            {
                case OUTPUT_MODE_SYNC_24:
                    T0_INT_OFF();
                    LATCH[j] |= BITMASK;
                    DURATION[output] = DURATION_CONFIG[output];
                    T0_INT_ON();
                    break;
                case OUTPUT_MODE_SYNC_12:
                    if (CLOCK_COUNT_2 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_8:
                    if (CLOCK_COUNT_3 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_6:
                    if (CLOCK_COUNT_4 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_4:
                    if (CLOCK_COUNT_6 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_3:
                    if (CLOCK_COUNT_8 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_2:
                    if (CLOCK_COUNT_12 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
                case OUTPUT_MODE_SYNC_1:
                    if (CLOCK_COUNT_24 == 0)
                    {
                        T0_INT_OFF();
                        LATCH[j] |= BITMASK;
                        DURATION[output] = DURATION_CONFIG[output];
                        T0_INT_ON();
                    }
                    break;
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();

    CLOCK_COUNT_2 ^= 0b00000001;
    CLOCK_COUNT_4++;
    CLOCK_COUNT_4 &= 0b00000011;
    CLOCK_COUNT_8++;
    CLOCK_COUNT_8 &= 0b00000111;

    CLOCK_COUNT_3++;
    if (CLOCK_COUNT_3 == 3)
    {
        CLOCK_COUNT_3 = 0;
    }
    CLOCK_COUNT_6++;
    if (CLOCK_COUNT_6 == 6)
    {
        CLOCK_COUNT_6 = 0;
    }
    CLOCK_COUNT_12++;
    if (CLOCK_COUNT_12 == 12)
    {
        CLOCK_COUNT_12 = 0;
    }
    CLOCK_COUNT_24++;
    if (CLOCK_COUNT_24 == 24)
    {
        CLOCK_COUNT_24 = 0;
    }
}

void ProcessMIDIStart()
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    ACTBlink();
    ACTOn();
    output = 0;
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            switch (eeprom_read_byte(CHUNK_ADDRESS))
            {
                case OUTPUT_MODE_SYNC_RUN:
                    T0_INT_OFF();
                    LATCH[j] |= BITMASK;
                    T0_INT_ON();
                    break;
                case OUTPUT_MODE_SYNC_RESET:
                    T0_INT_OFF();
                    LATCH[j] |= BITMASK;
                    T0_INT_ON();
                    DURATION[output] = DURATION_CONFIG[output];
                    break;
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();

    CLOCK_COUNT_24 = 0;
    CLOCK_COUNT_12 = 0;
    CLOCK_COUNT_8 = 0;
    CLOCK_COUNT_6 = 0;
    CLOCK_COUNT_4 = 0;
    CLOCK_COUNT_3 = 0;
    CLOCK_COUNT_2 = 0;
}

void ProcessMIDIContinue()
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    ACTBlink();
    ACTOn();
    output = 0;
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            if (eeprom_read_byte(CHUNK_ADDRESS) == OUTPUT_MODE_SYNC_RUN)
            {
                T0_INT_OFF();
                LATCH[j] |= BITMASK;
                T0_INT_ON();
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();
}

void ProcessMIDIStop()
{
    uint8_t i, j;
    uint8_t output;
    uint8_t BITMASK;
    uint8_t * CHUNK_ADDRESS = OUTPUT_CONFIG;

    ACTBlink();
    ACTOn();
    output = 0;
    for (j = 0; j < 3; j++)
    {
        BITMASK = 0b00000001;
        for (i = 0; i < 8; i++)
        {
            if (eeprom_read_byte(CHUNK_ADDRESS) == OUTPUT_MODE_SYNC_RUN)
            {
                T0_INT_OFF();
                LATCH[j] &= ~BITMASK;
                T0_INT_ON();
            }
            output++;
            BITMASK <<= 1;
            CHUNK_ADDRESS += CONFIG_CHUNK_SIZE;
        }
    }
    WriteLatches();
}

void ProcessMIDIReset()
{
    ACTBlink();
    ACTOn();
    LatchInit();
    WriteLatches();
}

void ProcessMIDIRealTime(uint8_t status)
{
    // real time messages
    switch (status)
    {
        case 0xF8:
            ProcessMIDIClock();
            break;
        case 0xFA:
            ProcessMIDIStart();
            break;
        case 0xFB:
            ProcessMIDIContinue();
            break;
        case 0xFC:
            ProcessMIDIStop();
            break;
        case 0xFF:
            ProcessMIDIReset();
            break;
    }
}

void QueueMIDIEvent(uint8_t num_bytes, uint8_t status, uint8_t d0, uint8_t d1)
{
    static uint8_t running_status = 0x00;
    cli();

    if (SERIAL_TX_BUFFER_FILL <= SERIAL_TX_BUFFER_SIZE-num_bytes)
    {
        // check here for status that does not change running status (real time, etc)
        if (num_bytes == 1)
        {
            SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_TAIL++] = status;
            SERIAL_TX_BUFFER_TAIL %= SERIAL_TX_BUFFER_SIZE;
            SERIAL_TX_BUFFER_FILL++;
        }
        // status byte, only if required
        else
        {
            if ((SERIAL_TX_BUFFER_FILL==0) || (running_status!=status))
            {
                SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_TAIL++] = status;
                SERIAL_TX_BUFFER_TAIL %= SERIAL_TX_BUFFER_SIZE;
                SERIAL_TX_BUFFER_FILL++;
                running_status = status;
            }

            // d0 byte 
            SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_TAIL++] = d0;
            SERIAL_TX_BUFFER_TAIL %= SERIAL_TX_BUFFER_SIZE;
            SERIAL_TX_BUFFER_FILL++;

            // d1 byte, only if required 
            if (num_bytes > 2)
            {
                SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_TAIL++] = d1;
                SERIAL_TX_BUFFER_TAIL %= SERIAL_TX_BUFFER_SIZE;
                SERIAL_TX_BUFFER_FILL++;
            }
        }
        // enable emptry TX register interrupt
        UCSR1B |= 1<<UDRIE1;
    }

    sei();
}
