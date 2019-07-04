//============================================================================
// Name        : AP.h
// Author      : Mahendra Gunawardena
// Date        : 12/26/2018
// Version     : Rev 0.01
// Copyright   : Your copyright notice
// Description : Applicaiton Progamming Interface for the MSP430F5529 + CC2650 BLE Demo
//============================================================================
/*
 * AP_BLE.c
 * Implementation of Applicaiton Program  MSP430F5529 Launchpad and CC2650 BoosterXL.
 *
 * The source code is based on Real-Time Bluetooth Networks Shape the World edx.org MOOC's course
 *
 *
 * Copyright Mahendra Gunawardena, Mitisa LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL I
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * AP.h
 *
 *  Created on: Dec 19, 2018
 *      Author: Mahendra Gunawardena
 */


#include <stdint.h>

// NPI symbols
#define SOF  254
// return parameters
#define APFAIL 0
#define APOK   1
#define APDEBUG  1

//------------AP_Init------------
// Initialize serial link and GPIO to Bluetooth module
// See InitMiscGPIO in main file for hardware connections
// This routine resets the Bluetooth module and initialize connection
// Input: none
// Output: APOK on success, APFAIL on timeout
int AP_Init(void);

// Called from Main.c
// Initialize MRDY and Reset
void Init_BLE(void);

//------------ReadSRDY------------
// Read Slave Ready line
// Input: none
// Output: none
uint8_t ReadSRDY(void);

//------------SetMRDY------------
// Set the Master Ready line
// Pull high the Master Ready GPIO pin
// Input: none
// Output: none
void SetMRDY(void);

//------------AP_Reset------------
// reset the Bluetooth module
// with MRDY high, clear RESET low for 10 ms
// Input: none
// Output: none
void AP_Reset(void);

// ------------Clock_Delay1ms------------
// Simple delay function which delays about n milliseconds.
// Inputs: n, number of msec to wait
// Outputs: none
void Clock_Delay1ms(uint32_t n);

//------------AP_RecvStatus------------
// check to see if Bluetooth module wishes to send packet
// Inputs: none
// Outputs: 0 if no communication needed,
//          nonzero for communication ready
uint32_t AP_RecvStatus(void);


//------------AP_RecvMessage------------
// receive a message from the Bluetooth module
// 1) receive NPI package
// 2) Wait for entire message to be received
// Input: pointer to empty buffer into which data is returned
//        maximum size (discard data beyond this limit)
// Output: APOK if ok, APFAIL on error (timeout or fcs error)
int AP_RecvMessage(uint8_t *pt, uint32_t max);

//------------AP_SendMessageResponse------------
// send a message to the Bluetooth module
// and receive a response from the Bluetooth module
// 1) send outgoing message
// 2) wait at least 10ms to receive NPI package
// 3) Wait for entire message to be received
// Input: msgPt points to message to send
//        responsePt points to empty buffer into which data is returned
//        maximum size (discard data beyond this limit)
// Output: APOK if ok, APFAIL on error (timeout or fcs error)
int AP_SendMessageResponse(uint8_t *msgPt, uint8_t *responsePt,uint32_t max);


//*********AP_GetNotifyCCCD*******
// Return notification CCCD from the communication interface
// this does not perform BLU communication, it reads current CCCD value of notify characteristic
// Inputs:  index into which notify characteristic to return
// Outputs: 16-bit CCCD value of the notify characteristic
uint16_t AP_GetNotifyCCCD(uint32_t i);

//*************AP_SendNotification**************
// Send a notification (will skip if CCCD is 0)
// Input:  index into notify characteristic to send
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int AP_SendNotification(uint32_t i);

//------------AP_SendMessage------------
// sends a message to the Bluetooth module
// calculates/sends FCS at end
// FCS is the 8-bit EOR of all bytes except SOF and FCS itself
// 1) Send NPI package (it will calculate fcs)
// 2) Wait for entire message to be sent
// Input: pointer to NPI encoded array
// Output: APOK on success, APFAIL on timeout
int AP_SendMessage(uint8_t *pt);

//***********AP_GetSize***************
// returns the size of an NPI message
// Inputs:  pointer to NPI message
// Outputs: size of the message
// extracted little Endian from byte 1 and byte 2
uint32_t AP_GetSize(uint8_t *pt);


// *****AP_EchoReceived**************
// for debugging, sends RecvBuf from SNP to UART0
// Inputs:  result APOK or APFAIL
// Outputs: none
void AP_EchoReceived(int response);

// *****AP_EchoSendMessage**************
// For debugging, sends message to UART0
// Inputs:  pointer to message
// Outputs: none
void AP_EchoSendMessage(uint8_t *sendMsg);

// ****AP_BackgroundProcess****
// handle incoming SNP frames
// Inputs:  none
// Outputs: none
void AP_BackgroundProcess(void);

//***********AP_GetSize***************
// returns the size of an NPI message
// Inputs:  pointer to NPI message
// Outputs: size of the message
// extracted little Endian from byte 1 and byte 2
uint32_t AP_GetSize(uint8_t *pt);
