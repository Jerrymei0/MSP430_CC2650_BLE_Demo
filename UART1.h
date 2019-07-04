/*
 * UART1.h
 *
 *  Created on: Dec 21, 2018
 *      Author: Mahendra Gunawardena
 *
 * Runs on MSP430F5529
 * Use UART1 to implement bidirectional data transfer to and from another microcontroller
 * U1Rx P4-5 is RxD (input to this microcontroller)
 * U1Tx P4-4 is TxD (output of this microcontroller)
 * interrupts and FIFO used for receiver, busy-wait on transmit.
 *
 */

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

//------------UART1_Init------------
// Initialize the UART1 for 115,200 baud rate (assuming 24 MHz clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART1_Init(void);

//------------UART1_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART1_InChar(void);

//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(uint8_t data);

//------------UART1_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(uint8_t *pt);

//------------UART1_FinishOutput------------
// Wait for all transmission to finish
// Input: none
// Output: none
void UART1_FinishOutput(void);

//------------UART1_InStatus------------
// Returns how much data available for reading
// Input: none
// Output: number of elements in receive FIFO
uint32_t UART1_InStatus(void);

//------------UART1_HW_FIFO_InStatus------------
// Returns how much data available for reading in the pseudo HW FIFO
// Input: none
// Output: number of elements in received in pseudo Hardware FIFO

uint32_t UART1_HW_FIFO_InStatus(void);
