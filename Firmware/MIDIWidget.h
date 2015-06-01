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

#ifndef _AUDIO_OUTPUT_H_
#define _AUDIO_OUTPUT_H_

/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "Descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

/* Constants: */
// 1.0.0
// #define CURRENT_FIRMWARE_VERSION 0x01
// 1.0.1
// #define CURRENT_FIRMWARE_VERSION 0x02
// 1.1.0 
// #define CURRENT_FIRMWARE_VERSION 0x03
// 1.1.1
#define CURRENT_FIRMWARE_VERSION 0x04

// 1.1.x reserved numbers 0x05 - 0x0F
// 1.2.x reserved numbers 0x10 - 0x1F


#define NUM_LATCHES 3
#define NUM_OUTPUTS 24
#define NUM_PRESETS 128
#define CONFIG_CHUNK_SIZE 16
#define SERIAL_RX_BUFFER_SIZE 64
#define SERIAL_TX_BUFFER_SIZE 128
#define NUM_UNPARSED_STREAMS 2
#define STREAM_SERIAL_MIDI 0
#define STREAM_USB_MIDI 1

// 0x00 - 0x3F: logic only
#define OUTPUT_MODE_DISABLED 0x00
#define OUTPUT_MODE_NOTE 0x01
#define OUTPUT_MODE_NOTE_TOGGLE 0x02
#define OUTPUT_MODE_NOTE_FIXED 0x03
#define OUTPUT_MODE_NOTE_MATCH_ONLY 0x04
#define OUTPUT_MODE_CC 0x08
#define OUTPUT_MODE_CC_TOGGLE 0x09
#define OUTPUT_MODE_CC_FIXED 0x0A
#define OUTPUT_MODE_CC_MATCH_ONLY 0x0B
#define OUTPUT_MODE_PC_FIXED 0x10
#define OUTPUT_MODE_PC_MATCH_ONLY 0x11
#define OUTPUT_MODE_SYNC_RUN 0x18
#define OUTPUT_MODE_SYNC_RESET 0x19
#define OUTPUT_MODE_SYNC_24 0x1A
#define OUTPUT_MODE_SYNC_12 0x1B
#define OUTPUT_MODE_SYNC_8 0x1C
#define OUTPUT_MODE_SYNC_6 0x1D
#define OUTPUT_MODE_SYNC_4 0x1E
#define OUTPUT_MODE_SYNC_3 0x1F
#define OUTPUT_MODE_SYNC_2 0x20
#define OUTPUT_MODE_SYNC_1 0x21
// 0x4x: PWM only
#define OUTPUT_MODE_PWM_CC 0x40
#define OUTPUT_MODE_PWM_NOTE_VELOCITY 0x41
#define OUTPUT_MODE_PWM_NOTE_VELOCITY_FIXED 0x42
#define OUTPUT_MODE_PWM_NOTE_NUMBER 0x43
#define OUTPUT_MODE_PWM_NOTE_NUMBER_FIXED 0x44

/* EEData Variables: */
extern uint8_t OUTPUT_CONFIG[] EEMEM;
extern uint8_t PRESET_SLOTS[] EEMEM;
extern uint8_t CONFIG_PRESET_STORE_TYPE EEMEM;
extern uint8_t CONFIG_PRESET_STORE_CHANNEL EEMEM;
extern uint8_t CONFIG_PRESET_STORE_NUMBER EEMEM;
extern uint8_t CONFIG_PRESET_RECALL_TYPE EEMEM;
extern uint8_t CONFIG_PRESET_RECALL_CHANNEL EEMEM;
extern uint8_t CONFIG_OPERATION_FLAGS EEMEM;
#define PRESET_SLOT_SIZE 4

/* Global Variables: */
uint8_t OPERATION_FLAGS;
#define CONFIG_MODE_FLAG 0
#define DISABLE_SERIAL_MIDI_THRU_FLAG 1
#define DISABLE_USB_MIDI_THRU_FLAG 2
#define DEFER_LATCH_WRITE 3

extern uint8_t LATCH[NUM_LATCHES];
extern uint8_t LATCH0_INV;
extern uint8_t LATCH1_INV;
extern uint8_t LATCH2_INV;

uint8_t PRESET_STORE_CHANNEL;
uint8_t PRESET_STORE_TYPE;
uint8_t PRESET_STORE_NUMBER;
uint8_t PRESET_RECALL_CHANNEL;
uint8_t PRESET_RECALL_TYPE;
uint8_t CURRENT_PRESET;
#define MESSAGE_TYPE_NONE 0x00
#define MESSAGE_TYPE_PC 0x01
#define MESSAGE_TYPE_CC 0x02
#define MESSAGE_TYPE_NOTE 0x03
#define MIDI_CHANNEL_ANY 0x10
uint8_t LATCH0_PRESET_FLAGS;
uint8_t LATCH1_PRESET_FLAGS;
uint8_t LATCH2_PRESET_FLAGS;
extern uint8_t BLINK_COUNTER;

uint8_t CLOCK_COUNT_24;
uint8_t CLOCK_COUNT_12;
uint8_t CLOCK_COUNT_8;
uint8_t CLOCK_COUNT_6;
uint8_t CLOCK_COUNT_4;
uint8_t CLOCK_COUNT_3;
uint8_t CLOCK_COUNT_2;

uint8_t SW_STATES;
uint8_t SW_STATES_PREV;
uint8_t SWITCH_DEBOUNCE_COUNTER;

uint16_t DURATION[NUM_OUTPUTS];
uint16_t DURATION_CONFIG[NUM_OUTPUTS];

uint8_t PWM_COUNTER;
uint8_t PWM[NUM_OUTPUTS];

uint8_t NRPN_ADDR_H, NRPN_ADDR_L;
uint8_t NRPN_MODE_H, NRPN_MODE_L;
extern uint8_t CONFIG_WRITE_BUFFER[CONFIG_CHUNK_SIZE];

extern uint8_t SERIAL_RX_BUFFER[SERIAL_RX_BUFFER_SIZE];
extern uint8_t SERIAL_RX_BUFFER_HEAD;
extern uint8_t SERIAL_RX_BUFFER_TAIL;
extern uint8_t SERIAL_RX_BUFFER_FILL;

extern uint8_t SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_SIZE];
extern uint8_t SERIAL_TX_BUFFER_HEAD;
extern uint8_t SERIAL_TX_BUFFER_TAIL;
extern uint8_t SERIAL_TX_BUFFER_FILL;

extern USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface;

/* Macros: */
#define OutputEnable() (PORTD &= ~(1<<7))
#define OutputDisable() (PORTD |= (1<<7))

#define STBYOff() (PORTC |= (1<<5))
#define STBYOn() (PORTC &= ~(1<<5))
#define STBYToggle() (PORTC ^= (1<<5))

#define ACTOff() (PORTC |= (1<<4))
#define ACTOn() (PORTC &= ~(1<<4))
#define ACTToggle() (PORTC ^= (1<<4))

#define BLINK_LENGTH 0xFF
#define ACTBlink() (BLINK_COUNTER = BLINK_LENGTH)

#define WRITE_LATCHES() \
    PORTB = LATCH[0] ^ LATCH0_INV; \
    PORTD |= (1<<6); \
    PORTD &= ~(1<<6); \
    PORTB = LATCH[1] ^ LATCH1_INV; \
    PORTC |= (1<<7); \
    PORTC &= ~(1<<7); \
    PORTB = LATCH[2] ^ LATCH2_INV; \
    PORTC |= (1<<6); \
    PORTC &= ~(1<<6)

#define T0_INT_ON() TIMSK0 |= (1<<TOIE0)
#define T0_INT_OFF() TIMSK0 &= ~(1<<TOIE0)

/* Function Prototypes: */
void LatchInit(void);
void PortInit(void);
void WriteLatches(void);
void SetupUSB(void);
void GlobalVariableInit(void);
void ConfigVariableInit(void);
void USARTInit(void);
void Timer0Init(void);
void ProcessSerialMIDI(void);
void ParseMIDIByte(uint8_t MIDI_BYTE, uint8_t stream);
void ProcessNoteOff(uint8_t CHANNEL, uint8_t NOTE_NUMBER);
void ProcessNoteOn(uint8_t CHANNEL, uint8_t NOTE_NUMBER, uint8_t VELOCITY);
void ProcessAftertouch(uint8_t CHANNEL, uint8_t NOTE_NUMBER, uint8_t VELOCITY);
void ProcessController(uint8_t CHANNEL, uint8_t NUMBER, uint8_t VALUE);
void ProcessProgramChange(uint8_t CHANNEL, uint8_t NUMBER);
void ProcessChannelPressure(uint8_t CHANNEL, uint8_t NUMBER);
void ProcessPitchWheel(uint8_t CHANNEL, uint8_t D0, uint8_t D1);
void ProcessMIDIClock(void);
void ProcessMIDIStart(void);
void ProcessMIDIContinue(void);
void ProcessMIDIStop(void);
void ProcessMIDIReset(void);
void ProcessMIDIRealTime(uint8_t status);
void ProcessConfigController(uint8_t NUMBER, uint8_t VALUE);
void QueueMIDIEvent(uint8_t bytes, uint8_t STATUS, uint8_t D0, uint8_t D1);
void ReadSwitches();
void RecallPreset(uint8_t number);
void StorePreset(void);
void updateNRPNMode(void);
void ConfigDump(void);
void RequestNextChunk(void);
void USBSendMIDIMessage(uint8_t Status, uint8_t D0, uint8_t D1);
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

#endif

