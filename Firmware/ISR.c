/*
 * MIDIWidget
 * www.midiwidget.com
 * copyright John Staskevich, 2015
 * john@codeandcopper.com
 *
 * This work is licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 *
 * ISR.c
 *
 * Interrupt service routines.
 */ 

#include "MIDIWidget.h"

ISR(TIMER0_OVF_vect)
{
// Easy way to observe timing of interrupt
//    STBYToggle();
	BLINK_COUNTER--;
    PWM_COUNTER = (PWM_COUNTER + 1) & 0b01111111;
	if (BLINK_COUNTER == 0)
	{
		ACTOff();
	}
    if (DURATION[0] != 0)
    {
        DURATION[0]--;
        if (DURATION[0] == 0)
        {
            LATCH[0] &= 0b11111110;
            PWM[0] = 0xFF;
        }
    }
    if (PWM[0] != 0xFF)
    {
        if (PWM[0] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11111110;
        }
        else
        {
            LATCH[0] |= 0b00000001;
        }
    }

    if (DURATION[1] != 0)
    {
        DURATION[1]--;
        if (DURATION[1] == 0)
        {
            LATCH[0] &= 0b11111101;
            PWM[1] = 0xFF;
        }
    }
    if (PWM[1] != 0xFF)
    {
        if (PWM[1] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11111101;
        }
        else
        {
            LATCH[0] |= 0b00000010;
        }
    }

    if (DURATION[2] != 0)
    {
        DURATION[2]--;
        if (DURATION[2] == 0)
        {
            LATCH[0] &= 0b11111011;
            PWM[2] = 0xFF;
        }
    }
    if (PWM[2] != 0xFF)
    {
        if (PWM[2] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11111011;
        }
        else
        {
            LATCH[0] |= 0b00000100;
        }
    }

    if (DURATION[3] != 0)
    {
        DURATION[3]--;
        if (DURATION[3] == 0)
        {
            LATCH[0] &= 0b11110111;
            PWM[3] = 0xFF;
        }
    }
    if (PWM[3] != 0xFF)
    {
        if (PWM[3] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11110111;
        }
        else
        {
            LATCH[0] |= 0b00001000;
        }
    }

    if (DURATION[4] != 0)
    {
        DURATION[4]--;
        if (DURATION[4] == 0)
        {
            LATCH[0] &= 0b11101111;
            PWM[4] = 0xFF;
        }
    }
    if (PWM[4] != 0xFF)
    {
        if (PWM[4] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11101111;
        }
        else
        {
            LATCH[0] |= 0b00010000;
        }
    }

    if (DURATION[5] != 0)
    {
        DURATION[5]--;
        if (DURATION[5] == 0)
        {
            LATCH[0] &= 0b11011111;
            PWM[5] = 0xFF;
        }
    }
    if (PWM[5] != 0xFF)
    {
        if (PWM[5] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b11011111;
        }
        else
        {
            LATCH[0] |= 0b00100000;
        }
    }

    if (DURATION[6] != 0)
    {
        DURATION[6]--;
        if (DURATION[6] == 0)
        {
            LATCH[0] &= 0b10111111;
            PWM[6] = 0xFF;
        }
    }
    if (PWM[6] != 0xFF)
    {
        if (PWM[6] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b10111111;
        }
        else
        {
            LATCH[0] |= 0b01000000;
        }
    }

    if (DURATION[7] != 0)
    {
        DURATION[7]--;
        if (DURATION[7] == 0)
        {
            LATCH[0] &= 0b01111111;
            PWM[7] = 0xFF;
        }
    }
    if (PWM[7] != 0xFF)
    {
        if (PWM[7] <= PWM_COUNTER)
        {
            LATCH[0] &= 0b01111111;
        }
        else
        {
            LATCH[0] |= 0b10000000;
        }
    }

    if (DURATION[8] != 0)
    {
        DURATION[8]--;
        if (DURATION[8] == 0)
        {
            LATCH[1] &= 0b11111110;
        }
    }
    if (PWM[8] != 0xFF)
    {
        if (PWM[8] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11111110;
        }
        else
        {
            LATCH[1] |= 0b00000001;
        }
    }

    if (DURATION[9] != 0)
    {
        DURATION[9]--;
        if (DURATION[9] == 0)
        {
            LATCH[1] &= 0b11111101;
        }
    }
    if (PWM[9] != 0xFF)
    {
        if (PWM[9] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11111101;
        }
        else
        {
            LATCH[1] |= 0b00000010;
        }
    }

    if (DURATION[10] != 0)
    {
        DURATION[10]--;
        if (DURATION[10] == 0)
        {
            LATCH[1] &= 0b11111011;
        }
    }
    if (PWM[10] != 0xFF)
    {
        if (PWM[10] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11111011;
        }
        else
        {
            LATCH[1] |= 0b00000100;
        }
    }

    if (DURATION[11] != 0)
    {
        DURATION[11]--;
        if (DURATION[11] == 0)
        {
            LATCH[1] &= 0b11110111;
        }
    }
    if (PWM[11] != 0xFF)
    {
        if (PWM[11] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11110111;
        }
        else
        {
            LATCH[1] |= 0b00001000;
        }
    }

    if (DURATION[12] != 0)
    {
        DURATION[12]--;
        if (DURATION[12] == 0)
        {
            LATCH[1] &= 0b11101111;
        }
    }
    if (PWM[12] != 0xFF)
    {
        if (PWM[12] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11101111;
        }
        else
        {
            LATCH[1] |= 0b00010000;
        }
    }

    if (DURATION[13] != 0)
    {
        DURATION[13]--;
        if (DURATION[13] == 0)
        {
            LATCH[1] &= 0b11011111;
        }
    }
    if (PWM[13] != 0xFF)
    {
        if (PWM[13] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b11011111;
        }
        else
        {
            LATCH[1] |= 0b00100000;
        }
    }

    if (DURATION[14] != 0)
    {
        DURATION[14]--;
        if (DURATION[14] == 0)
        {
            LATCH[1] &= 0b10111111;
        }
    }
    if (PWM[14] != 0xFF)
    {
        if (PWM[14] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b10111111;
        }
        else
        {
            LATCH[1] |= 0b01000000;
        }
    }

    if (DURATION[15] != 0)
    {
        DURATION[15]--;
        if (DURATION[15] == 0)
        {
            LATCH[1] &= 0b01111111;
        }
    }
    if (PWM[15] != 0xFF)
    {
        if (PWM[15] <= PWM_COUNTER)
        {
            LATCH[1] &= 0b01111111;
        }
        else
        {
            LATCH[1] |= 0b10000000;
        }
    }

    if (DURATION[16] != 0)
    {
        DURATION[16]--;
        if (DURATION[16] == 0)
        {
            LATCH[2] &= 0b11111110;
        }
    }
    if (PWM[16] != 0xFF)
    {
        if (PWM[16] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11111110;
        }
        else
        {
            LATCH[2] |= 0b00000001;
        }
    }

    if (DURATION[17] != 0)
    {
        DURATION[17]--;
        if (DURATION[17] == 0)
        {
            LATCH[2] &= 0b11111101;
        }
    }
    if (PWM[17] != 0xFF)
    {
        if (PWM[17] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11111101;
        }
        else
        {
            LATCH[2] |= 0b00000010;
        }
    }

    if (DURATION[18] != 0)
    {
        DURATION[18]--;
        if (DURATION[18] == 0)
        {
            LATCH[2] &= 0b11111011;
        }
    }
    if (PWM[18] != 0xFF)
    {
        if (PWM[18] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11111011;
        }
        else
        {
            LATCH[2] |= 0b00000100;
        }
    }

    if (DURATION[19] != 0)
    {
        DURATION[19]--;
        if (DURATION[19] == 0)
        {
            LATCH[2] &= 0b11110111;
        }
    }
    if (PWM[19] != 0xFF)
    {
        if (PWM[19] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11110111;
        }
        else
        {
            LATCH[2] |= 0b00001000;
        }
    }

    if (DURATION[20] != 0)
    {
        DURATION[20]--;
        if (DURATION[20] == 0)
        {
            LATCH[2] &= 0b11101111;
        }
    }
    if (PWM[20] != 0xFF)
    {
        if (PWM[20] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11101111;
        }
        else
        {
            LATCH[2] |= 0b00010000;
        }
    }

    if (DURATION[21] != 0)
    {
        DURATION[21]--;
        if (DURATION[21] == 0)
        {
            LATCH[2] &= 0b11011111;
        }
    }
    if (PWM[21] != 0xFF)
    {
        if (PWM[21] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b11011111;
        }
        else
        {
            LATCH[2] |= 0b00100000;
        }
    }

    if (DURATION[22] != 0)
    {
        DURATION[22]--;
        if (DURATION[22] == 0)
        {
            LATCH[2] &= 0b10111111;
        }
    }
    if (PWM[22] != 0xFF)
    {
        if (PWM[22] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b10111111;
        }
        else
        {
            LATCH[2] |= 0b01000000;
        }
    }

    if (DURATION[23] != 0)
    {
        DURATION[23]--;
        if (DURATION[23] == 0)
        {
            LATCH[2]&= 0b01111111;
        }
    }
    if (PWM[23] != 0xFF)
    {
        if (PWM[23] <= PWM_COUNTER)
        {
            LATCH[2] &= 0b01111111;
        }
        else
        {
            LATCH[2] |= 0b10000000;
        }
    }

    WRITE_LATCHES();
}

ISR(USART1_RX_vect)
{
	uint8_t TEMP;
	
	if (SERIAL_RX_BUFFER_FILL < SERIAL_RX_BUFFER_SIZE)
	{
		// put byte into FIFO
		SERIAL_RX_BUFFER[SERIAL_RX_BUFFER_TAIL++] = UDR1;
		SERIAL_RX_BUFFER_TAIL %= SERIAL_RX_BUFFER_SIZE;
		SERIAL_RX_BUFFER_FILL++;
	}
	else
	{
		// FIFO is full. drop the byte.
		TEMP = UDR1;
		// avoid a compiler warning.
		TEMP = TEMP;
	}
}

ISR(USART1_UDRE_vect)
{
    // send a byte from the buffer
    UDR1 = SERIAL_TX_BUFFER[SERIAL_TX_BUFFER_HEAD++];
    SERIAL_TX_BUFFER_HEAD %= SERIAL_TX_BUFFER_SIZE;
    SERIAL_TX_BUFFER_FILL--;
    // if tx buffer is empty, prevent additional interrupts
    if (SERIAL_TX_BUFFER_FILL==0)
    {
        UCSR1B &= ~(1<<UDRIE1);
    }
}
