/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * Config.c
 *
 * Functions for updating MIDIWidget configuration and communicating with MIDIWidget Configurator.
 */ 

#include "MIDIWidget.h"

uint8_t CONFIG_WRITE_BUFFER[CONFIG_CHUNK_SIZE];

void ProcessConfigController(uint8_t NUMBER, uint8_t VALUE)
{
    uint8_t * TEMPEEADR;
    if (NUMBER == 0x63)
    {
        NRPN_ADDR_H = VALUE;
        return;
    }
    else if (NUMBER == 0x62)
    {
        NRPN_ADDR_L = VALUE;
        return;
    }
    else if (NUMBER == 0x06)
    {
        if (NRPN_ADDR_H == 0x7F)
        {
            // this is a command
            switch (NRPN_ADDR_L)
            {
                case 0x7F:
                    NRPN_MODE_H = VALUE;
                    updateNRPNMode();
                    break;
                case 0x7E:
                    NRPN_MODE_L = VALUE;
                    updateNRPNMode();
                    break;
                case 0x7D:
                    if (OPERATION_FLAGS & (1<<CONFIG_MODE_FLAG))
                    {
                        ConfigDump();
                    }
                    break;			
            }
        }
        else if (OPERATION_FLAGS & (1<<CONFIG_MODE_FLAG))
        {
            // this is a config data byte
            CONFIG_WRITE_BUFFER[NRPN_ADDR_L % CONFIG_CHUNK_SIZE] = VALUE;
            if ((NRPN_ADDR_L % CONFIG_CHUNK_SIZE) == (CONFIG_CHUNK_SIZE - 1))
            {
                // last byte of chunk received.
                //cli();
                if (NRPN_ADDR_H < NUM_OUTPUTS)
                {
                    // write output config chunk
                    eeprom_update_block(CONFIG_WRITE_BUFFER, OUTPUT_CONFIG+(NRPN_ADDR_H*CONFIG_CHUNK_SIZE), CONFIG_CHUNK_SIZE);
                }
                else if (NRPN_ADDR_H < 0x20)
                {
                    // write preset data
                    TEMPEEADR = PRESET_SLOTS + ((NRPN_ADDR_H-0x18) * 64) + ((NRPN_ADDR_L / 16) * 8);
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[0] | (CONFIG_WRITE_BUFFER[1]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[2] | (CONFIG_WRITE_BUFFER[3]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[4] | (CONFIG_WRITE_BUFFER[5]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[6] | (CONFIG_WRITE_BUFFER[7]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[8] | (CONFIG_WRITE_BUFFER[9]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[10] | (CONFIG_WRITE_BUFFER[11]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[12] | (CONFIG_WRITE_BUFFER[13]<<4));
                    TEMPEEADR++;
                    eeprom_update_byte(TEMPEEADR, CONFIG_WRITE_BUFFER[14] | (CONFIG_WRITE_BUFFER[15]<<4));
                }
                else if ((NRPN_ADDR_H == 0x7E) && (NRPN_ADDR_L == 0x0F))
                {
                    // write global config stuff
                    eeprom_update_byte(&CONFIG_PRESET_RECALL_TYPE, CONFIG_WRITE_BUFFER[3]);
                    eeprom_update_byte(&CONFIG_PRESET_RECALL_CHANNEL, CONFIG_WRITE_BUFFER[4]);
                    eeprom_update_byte(&CONFIG_PRESET_STORE_TYPE, CONFIG_WRITE_BUFFER[5]);
                    eeprom_update_byte(&CONFIG_PRESET_STORE_CHANNEL, CONFIG_WRITE_BUFFER[6]);
                    eeprom_update_byte(&CONFIG_PRESET_STORE_NUMBER, CONFIG_WRITE_BUFFER[7]);
                }
                //sei();
                RequestNextChunk();
            }
        }
    }		
}

void updateNRPNMode()
{
    if ((NRPN_MODE_H == 0) && (NRPN_MODE_L == 0))
    {
        // begin normal operation.
        STBYOn();
        ACTBlink();
        TIMSK0 |= 1<<TOIE0;
        if (OPERATION_FLAGS & (1<<CONFIG_MODE_FLAG))
        {
            LatchInit();
            WriteLatches();
            ConfigVariableInit();
        }
        // redundant. happens in ConfigVariableInit().
        //OPERATION_FLAGS &= ~(1<<CONFIG_MODE_FLAG);
    }
    else if ((NRPN_MODE_H == 0x13) && (NRPN_MODE_L == 0x37))
    {
        TIMSK0 &= ~(1<<TOIE0);
        ACTOn();
        STBYOff();
        OPERATION_FLAGS |= 1<<CONFIG_MODE_FLAG;
    }
}

void ConfigDump()
{
    uint8_t TEMP_H;
    uint8_t TEMP_L;
    uint8_t * TEMPEEADR;
    uint8_t i, j;

    // disable timer0 interrupt
//    TIMSK0 &= ~(1<<TOIE0);
//    ACTOn();
//    STBYOff();

    // send firmware version as NRPN
    USBSendMIDIMessage(0xBF, 0x63, 0x7F);
    USBSendMIDIMessage(0xBF, 0x62, 0x00);
    USBSendMIDIMessage(0xBF, 0x06, CURRENT_FIRMWARE_VERSION);

    // send global config as NRPN
    USBSendMIDIMessage(0xBF, 0x63, 0x7E);
    USBSendMIDIMessage(0xBF, 0x62, 0x02);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_OPERATION_FLAGS));
    USBSendMIDIMessage(0xBF, 0x62, 0x03);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_PRESET_RECALL_TYPE));
    USBSendMIDIMessage(0xBF, 0x62, 0x04);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_PRESET_RECALL_CHANNEL));
//    _delay_ms(10);
    USBSendMIDIMessage(0xBF, 0x62, 0x05);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_PRESET_STORE_TYPE));
    USBSendMIDIMessage(0xBF, 0x62, 0x06);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_PRESET_STORE_CHANNEL));
    USBSendMIDIMessage(0xBF, 0x62, 0x07);
    USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(&CONFIG_PRESET_STORE_NUMBER));
//    _delay_ms(10);

    TEMP_H = 0;
    TEMPEEADR = OUTPUT_CONFIG;
    // send output configs as NRPNs
    for (i=0; i<NUM_OUTPUTS; i++)
    {
        TEMP_L = 0;
        USBSendMIDIMessage(0xBF, 0x63, TEMP_H);

        for (j=0; j<CONFIG_CHUNK_SIZE; j++)
        {
            USBSendMIDIMessage(0xBF, 0x62, TEMP_L);
            USBSendMIDIMessage(0xBF, 0x06, eeprom_read_byte(TEMPEEADR));

            TEMPEEADR++;
            TEMP_L++;
//            _delay_ms(10);
        }
        TEMP_H++;
    }

    TEMPEEADR = PRESET_SLOTS;
    // send preset data as NRPNs
    for (i=0; i<NUM_PRESETS; i++)
    {
        TEMP_L = (i%16)*8;
        USBSendMIDIMessage(0xBF, 0x63, 0x18 + i/16);

        for (j=0; j<PRESET_SLOT_SIZE; j++)
        {
            USBSendMIDIMessage(0xBF, 0x62, TEMP_L);
            TEMP_L++;
            USBSendMIDIMessage(0xBF, 0x06, (eeprom_read_byte(TEMPEEADR)) & 0b00001111);
//           _delay_ms(10);
            USBSendMIDIMessage(0xBF, 0x62, TEMP_L);
            TEMP_L++;
            USBSendMIDIMessage(0xBF, 0x06, (eeprom_read_byte(TEMPEEADR)) >> 4);
//            _delay_ms(10);
            TEMPEEADR++;
        }
    }


    // send "config dump complete" message
    USBSendMIDIMessage(0xBF, 0x63, 0x7F);
    USBSendMIDIMessage(0xBF, 0x62, 0x7C);
    USBSendMIDIMessage(0xBF, 0x06, 0x7F);

//    ACTBlink();
    // enable timer0 interrupt
//    TIMSK0 |= 1<<TOIE0;
}

void RequestNextChunk()
{
    USBSendMIDIMessage(0xBF, 0x63, 0x7F);
    USBSendMIDIMessage(0xBF, 0x62, 0x7B);
    USBSendMIDIMessage(0xBF, 0x06, 0x7F);
}
