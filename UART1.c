/*
 * UART1.c
 *
 *  Created on: Dec 21, 2018
 *      Author: Mahendra Gunawardena
 *
 * Runs on MSP430F5529
 * Use UART1 to implement bidirectional data transfer to and from another microcontroller
 * U1Rx P4-5 is RxD (input to this microcontroller)
 * U1Tx P4-4 is TxD (output of this microcontroller)
 * interrupts and FIFO used for receiver, busy-wait on transmit.
 */

#include <stdint.h>
#include "UART1.h"
#include "AP.h"
#include "common.h"
#include "driverlib.h"

/****************************************************************************/

#define NVIC_EN0_INT6           0x00000040  // Interrupt 6 enable
#define UART_FR_TXFE            0x00000080  // UART Transmit FIFO Empty
#define UART_FR_RXFF            0x00000040  // UART Receive FIFO Full
#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_FR_BUSY            0x00000008  // UART Transmit Busy
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable
#define UART_IFLS_RX1_8         0x00000000  // RX FIFO >= 1/8 full
#define UART_IFLS_TX1_8         0x00000000  // TX FIFO <= 1/8 full
#define UART_IM_RTIM            0x00000040  // UART Receive Time-Out Interrupt
// Mask
#define UART_IM_TXIM            0x00000020  // UART Transmit Interrupt Mask
#define UART_IM_RXIM            0x00000010  // UART Receive Interrupt Mask
#define UART_RIS_RTRIS          0x00000040  // UART Receive Time-Out Raw
// Interrupt Status
#define UART_RIS_TXRIS          0x00000020  // UART Transmit Raw Interrupt
// Status
#define UART_RIS_RXRIS          0x00000010  // UART Receive Raw Interrupt
// Status
#define UART_ICR_RTIC           0x00000040  // Receive Time-Out Interrupt Clear
#define UART_ICR_TXIC           0x00000020  // Transmit Interrupt Clear
#define UART_ICR_RXIC           0x00000010  // Receive Interrupt Clear

/****************************************************************************/

#define FIFOSIZE   256       // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1        // return value on success
#define FIFOFAIL    0        // return value on failure

#define HW_FIFOSIZE   256       // size of the FIFOs (must be power of 2)
#define HW_FIFOSUCCESS 1        // return value on success
#define HW_FIFOFAIL    0        // return value on failure

uint32_t RxPutI;      // should be 0 to SIZE-1
uint32_t RxGetI;      // should be 0 to SIZE-1
uint32_t RxFifoLost;  // should be 0
uint8_t RxFIFO[FIFOSIZE];

uint32_t HW_RxPutI;      // should be 0 to SIZE-1
uint32_t HW_RxGetI;      // should be 0 to SIZE-1
uint32_t HW_RxFifoLost;  // should be 0
uint32_t HW_count;  // should be 0
uint32_t read_data_count;  // should be 0
uint8_t msb;
uint8_t lsb;
uint32_t size;
uint8_t HW_RxFIFO[FIFOSIZE];

void RxFifo_Init(void) {
	RxPutI = 0;			// empty
	RxGetI = 0;			// empty
	RxFifoLost = 0; // occurs on overflow
}

int RxFifo_Put(uint8_t data) {
	if (((RxPutI + 1) & (FIFOSIZE - 1)) == RxGetI) {
		RxFifoLost++;
		return FIFOFAIL; // fail if full
	}
	RxFIFO[RxPutI] = data;                    // save in FIFO
	RxPutI = (RxPutI + 1) & (FIFOSIZE - 1);         // next place to put
	return FIFOSUCCESS;
}

int RxFifo_Get(uint8_t *datapt) {
	if (RxPutI == RxGetI)
		return 0;            // fail if empty
	*datapt = RxFIFO[RxGetI];                 // retrieve data
	RxGetI = (RxGetI + 1) & (FIFOSIZE - 1);         // next place to get
	return FIFOSUCCESS;
}

void HW_RxFifo_Init(void) {
	HW_RxPutI = 0;			// empty
	HW_RxGetI = 0;			// empty
	HW_RxFifoLost = 0;		// occurs on overflow
	HW_count = 0;
}

int HW_RxFifo_Get(uint8_t *datapt) {
	if (HW_RxPutI == HW_RxGetI)
		return 0;            // fail if empty
	*datapt = HW_RxFIFO[HW_RxGetI];                 	// retrieve data
	HW_RxGetI = (HW_RxGetI + 1) & (HW_FIFOSIZE - 1);	// next place to get
	return HW_FIFOSUCCESS;
}

//------------UART1_Init------------
// Initialize the UART1 for 115,200 baud rate (assuming 4 MHz clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART1_Init(void) {
	RxFifo_Init();                        // initialize empty FIFO
	HW_RxFifo_Init();
	USCI_A_UART_initParam param1 = { 0 };
	param1.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
	param1.clockPrescalar = 2;
	param1.firstModReg = 2;
	param1.secondModReg = 3;
	param1.parity = USCI_A_UART_NO_PARITY;
	param1.msborLsbFirst = USCI_A_UART_LSB_FIRST;
	param1.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
	param1.uartMode = USCI_A_UART_MODE;
	param1.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

	if (STATUS_FAIL == USCI_A_UART_init(BLE_BASE_ADDR, &param1)) {
		return;
	}

	//Enable UART module for operation
	USCI_A_UART_enable(BLE_BASE_ADDR);

	//Enable Receive Interrupt
	USCI_A_UART_clearInterrupt(BLE_BASE_ADDR, USCI_A_UART_RECEIVE_INTERRUPT);
	USCI_A_UART_enableInterrupt(BLE_BASE_ADDR, USCI_A_UART_RECEIVE_INTERRUPT);


}

//------------UART1_InStatus------------
// Returns how much data available for reading
// Input: none
// Output: number of elements in receive FIFO
uint32_t UART1_InStatus(void) {
	return ((RxPutI - RxGetI) & (FIFOSIZE - 1));
}

//------------UART1_HW_FIFO_InStatus------------
// Returns how much data available for reading in the pseudo HW FIFO
// Input: none
// Output: number of elements in received in pseudo Hardware FIFO

uint32_t UART1_HW_FIFO_InStatus(void) {
	return ((HW_RxPutI - HW_RxGetI) & (HW_FIFOSIZE - 1));
}

// copy from hardware RX FIFO to software RX FIFO
// stop when hardware RX FIFO is empty or software RX FIFO is full
void static copyHardwareToSoftware(void) {
	uint8_t letter;
	while (UART1_InStatus() < (FIFOSIZE - 1)) {
		letter = USCI_A_UART_receiveData(BLE_BASE_ADDR);
		RxFifo_Put(letter);
	}
}

// input ASCII character from UART
// spin if RxFifo is empty
uint8_t UART1_InChar(void) {
	uint8_t letter;
	uint8_t data;

	while (HW_RxFifo_Get(&data) == HW_FIFOSUCCESS) {
		RxFifo_Put(data);
	}
	while (RxFifo_Get(&letter) == FIFOFAIL) {
	};
	return (letter);
}
//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(uint8_t data) {
	while (USCI_A_UART_queryStatusFlags(BLE_BASE_ADDR, USCI_A_UART_BUSY)) {
			// Wait until complete
		}
	// Load data onto buffer
	USCI_A_UART_transmitData(BLE_BASE_ADDR, data);

}
// at least one of two things has happened:
// hardware RX FIFO goes from 1 to 2 or more items
// UART receiver has timed out

//******************************************************************************
//
//  Interrupt for BLE UART
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=BLE_UART_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(BLE_UART_VECTOR)))
#endif
void UART1_ISR(void) {
	uint8_t data;
	switch (__even_in_range(BLE_UART_VECTOR_REGISTER, 4)) {
	case USCI_UCRXIFG:	// received character interrupt
		data = USCI_A_UART_receiveData(BLE_BASE_ADDR);

		if (((HW_RxPutI + 1) & (HW_FIFOSIZE - 1)) == HW_RxGetI) {
			HW_RxFifoLost++;
		}
		HW_RxFIFO[HW_RxPutI] = data;
		HW_RxPutI = (HW_RxPutI + 1) & (HW_FIFOSIZE - 1);
		if (data == SOF) {
			HW_count++ ;
			read_data_count=0;
			size=0;
		} else if (read_data_count<1) {
			lsb = data;
			read_data_count++;
		} else if (read_data_count<2){
			size = (data << 8) + lsb;
			read_data_count++;
		} else if (read_data_count<(size+4)){
			read_data_count++;
		} else {
		  SetMRDY();
		}
		break;
	case USCI_UCTXIFG:	// transmitted character interrupt, not currently used
		break;

	default:	// shouldn't happen
		break;

	}
}

//------------UART1_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(uint8_t *pt) {
	while (*pt) {
		UART1_OutChar(*pt);
		pt++;
	}
}

//------------UART1_FinishOutput------------
// Wait for all transmission to finish
// Input: none
// Output: none

void UART1_FinishOutput(void) {

	while (USCI_A_UART_queryStatusFlags(BLE_BASE_ADDR, USCI_A_UART_BUSY)) {
		// Wait until complete
	}
	HW_RxFifo_Init();
	RxFifo_Init();
}

