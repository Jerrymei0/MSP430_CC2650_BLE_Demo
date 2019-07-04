//============================================================================
// Name        : AP.c
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
 * AP.c
 *
 *  Created on: Dec 19, 2018
 *      Author: Mahendra Gunawardena
 *
 */

#include "AP.h"
#include "common.h"
#include <stdint.h>
#include <stdio.h>
#include "driverlib.h"
#include "UART0.h"
#include "UART1.h"

uint8_t RecvBuf[RECVSIZE];

uint32_t fcserr;      // debugging counts of errors
uint32_t TimeOutErr;  // debugging counts of no response errors
uint32_t NoSOFErr;    // debugging counts of no SOF errors

//#define APTIMEOUT 40000   // 10 ms TM4C
#define APTIMEOUT 140000   // 10 ms (Approx) MSP430 24MHz clock
#define CONST_MAXCHARACTERISTICS 10
#define CONST_NOTIFYMAXCHARACTERISTICS 4


//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif


//*************message and message fragments**********
const uint8_t HCI_EXT_ResetSystemCmd[] = { SOF, 0x03, 0x00, 0x55, 0x04, 0x1D, 0xFC, 0x01, 0xB2 };

uint8_t NPI_WriteConfirmation[] = {
  SOF,0x03,0x00,  // length = 3
  0x55,0x88,      // SNP Characteristic Write Confirmation
  0x00,           // Success
  0x00,0x00,      // handle of connection always 0
  0xDE};          // FCS (calculated by AP_SendMessageResponse)

uint8_t NPI_ReadConfirmation[] = {
  SOF,0x08,0x00,  // length = 8 (7+data length, filled in dynamically)
  0x55,0x87,      // SNP Characteristic Read Confirmation (0x87)
  0x00,           // Success
  0x00,0x00,      // handle of connection always 0
  0x00,0x00,      // Handle of the characteristic value attribute being read (filled in dynamically
  0x00,0x00,      // offset, ignored, assumes small chucks of data
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // actual data (filled in dynamically)
  0x00};          // FCS (calculated by AP_SendMessageResponse)

uint8_t NPI_CCCDUpdatedConfirmation[] = {
  SOF,0x03,0x00,  // length = 3
  0x55,0x8B,      // SNP CCCD Updated Confirmation (0x8B)
  0x00,           // Success
  0x00,0x00,      // handle of connection always 0
  0xDD};          // FCS (calculated by AP_SendMessageResponse)

uint8_t NPI_SendNotificationIndication[] = {
  SOF,0x07,0x00,  // length = 7 to 14 depending on data size
  0x55,0x89,      // SNP Send Notification Indication (0x89))
  0x00,0x00,      // handle of connection always 0
  0x00,0x00,      // Handle of the characteristic value attribute to notify / indicate (filled in dynamically
  0x00,           // RFU
  0x01,           // Indication Request type
  0x00,0,0,0,0,0,0,0, // 1 to 8 bytes of data filled in dynamically
  0xDD};      // FCS (calculated by AP_SendMessageResponse)

//------------AP_Init------------
// Initialize serial link and GPIO to Bluetooth module
// See InitMiscGPIO in main file for hardware connections
// This routine resets the Bluetooth module and initialize connection
// Input: none
// Output: APOK on success, APFAIL on timeout

int AP_Init(void) {
	int bwaiting;
	long count;

	Init_BLE(); 	// MRDY, SRDY, reset

	fcserr = 0;		// number of packets with FCS errors
	TimeOutErr = 0;	// debugging counts of no response error
	NoSOFErr = 0;	// debugging counts of no SOF error
	bwaiting = 1;	// waiting for reset
	count = 0;

	while (bwaiting) {
		AP_Reset();
		count = 0; // should get SNP power up within 30 ms (duration is arbitrary and 'count' value is uncalibrated)
		while (count < 600000) {
			if (AP_RecvStatus()) {
				AP_RecvMessage(RecvBuf, RECVSIZE);
				if ((RecvBuf[3] == 0x55) && (RecvBuf[4] == 0x01)) {
					count = 600000;
					bwaiting = 0; // success
				}
			}
			count = count + 1;
		}
	}
	UART1_FinishOutput();
	AP_SendMessageResponse((uint8_t*) HCI_EXT_ResetSystemCmd, RecvBuf, RECVSIZE);
	count = 0; // should get SNP power up within 120 ms (duration is arbitrary and 'count' value is uncalibrated)
	bwaiting = 1; // waiting for SNP power up
	//Clock_Delay1ms(5);
	while (count < 6000000) {
		if (AP_RecvStatus()) {
			AP_RecvMessage(RecvBuf, RECVSIZE);
			if ((RecvBuf[3] == 0x55) && (RecvBuf[4] == 0x01)) {
				count = 6000000;
				bwaiting = 0; // success
			}
		}
		count = count + 1;
	}
	UART1_FinishOutput();
	if (bwaiting) {
		TimeOutErr++;  // no response error
		return APFAIL;
	}

	return APOK;
}

//------------ClearMRDY------------
// Clear the Master Ready line
// Pull Low the Master Ready GPIO pin
// Input: none
// Output: none
void ClearMRDY(void) {
	GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
}

//------------SetMRDY------------
// Set the Master Ready line
// Pull high the Master Ready GPIO pin
// Input: none
// Output: none
void SetMRDY(void) {
	GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN3);
}

//------------ClearReset------------
// Clear the Reset line
// Pull Low the Reset GPIO pin
// Input: none
// Output: none
void ClearReset(void) {
	GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

//------------SetReset------------
// Set the Reset line
// Pull High the Reset GPIO pin
// Input: none
// Output: none
void SetReset(void) {
	GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

//------------ReadSRDY------------
// Read Slave Ready line
// Input: none
// Output: none
uint8_t ReadSRDY(void) {

	return (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4));

}

//------------AP_Reset------------
// reset the Bluetooth module
// with MRDY high, clear RESET low for 10 ms
// Input: none
// Output: none
void AP_Reset(void) {
	ClearReset();   // RESET=0
	SetMRDY();      // MRDY=1
	Clock_Delay1ms(10);
	SetReset();     // RESET=1
	//Clock_Delay1ms(10);
}

// ------------Clock_Delay1ms------------
// Simple delay function which delays about n milliseconds.
// Inputs: n, number of msec to wait
// Outputs: none
void Clock_Delay1ms(uint32_t n) {
	while (n) {
		__delay_cycles(24000);  // 1 msec, tuned at 24 MHz
		//__delay_cycles(8192);  // 1 msec, tuned at 24 MHz
		n--;
	}
}

//------------AP_RecvStatus------------
// check to see if Bluetooth module wishes to send packet
// Inputs: none
// Outputs: 0 if no communication needed,
//          nonzero for communication ready
uint32_t AP_RecvStatus(void) {
	return (ReadSRDY() == 0);
}

//------------AP_RecvMessage------------
// receive a message from the Bluetooth module
// 1) receive NPI package
// 2) Wait for entire message to be received
// Input: pointer to empty buffer into which data is returned
//        maximum size (discard data beyond this limit)
// Output: APOK if ok, APFAIL on error (timeout or fcs error)
int AP_RecvMessage(uint8_t *pt, uint32_t max) {
	uint8_t fcs;
	uint32_t waitCount;
	uint8_t data;
	uint8_t cmd0;
	uint8_t cmd1;
	uint8_t msb;
	uint8_t lsb;
	uint32_t size;
	uint32_t count;
	uint32_t SOFcount;
	uint32_t HW_FIFO_Count;
	uint32_t SW_FIFO_Count;

	// SOFcount=50;
	SOFcount=200;
	data=NULL;
// 1) wait for SRDY to be low
	waitCount = 0;
	//Clock_Delay1ms(1);
	while (ReadSRDY()) {
		waitCount++;
		if (waitCount > APTIMEOUT) {
			TimeOutErr++;  // no response error
			//return 10;
			return APFAIL; // timeout??
		}
	}
// 2) Make MRDY=0
	Clock_Delay1ms(1);
	ClearMRDY();
	//Clock_Delay1ms(1);
// 3) receive NPI package
	// wait for SOF

	do {
		HW_FIFO_Count = UART1_HW_FIFO_InStatus();
		SW_FIFO_Count = UART1_InStatus();
		if ((HW_FIFO_Count>0)||(SW_FIFO_Count>0)) {
			data = UART1_InChar();
		}
		SOFcount--;
		if (SOFcount == 0) {
			SetMRDY();     //   MRDY=1
			NoSOFErr++;    // no SOF error
			return APFAIL;
			//return 11;
		}
	} while (data != SOF);
	*pt = data;
	pt++;
	fcs = 0;
// get size, once we get SOF, it is highly likely for the rest to come
	while ( (UART1_HW_FIFO_InStatus()==0) && (UART1_InStatus()==0) ) {
	}
	lsb = UART1_InChar();
	*pt = lsb;
	fcs = fcs ^ lsb;
	pt++;
	while ((UART1_HW_FIFO_InStatus()==0)&&(UART1_InStatus()==0)) {
	}
	msb = UART1_InChar();
	*pt = msb;
	fcs = fcs ^ msb;
	pt++;
// get command
	while ((UART1_HW_FIFO_InStatus()==0)&&(UART1_InStatus()==0)) {}
	cmd0 = UART1_InChar();
	*pt = cmd0;
	fcs = fcs ^ cmd0;
	pt++;
	while ((UART1_HW_FIFO_InStatus()==0)&&(UART1_InStatus()==0)) {}
	cmd1 = UART1_InChar();
	*pt = cmd1;
	fcs = fcs ^ cmd1;
	pt++;
	count = 5;
	size = (msb << 8) + lsb;
// get payload
	while ((UART1_HW_FIFO_InStatus()+UART1_InStatus())<size) {}
	int i;
	for (i = 0; i < size; i++) {
		data = UART1_InChar();
		fcs = fcs ^ data;
		count++;
		if (count <= max) {
			*pt = data;
			pt++; // payload
		}
	}
// get FCB
	while ((UART1_HW_FIFO_InStatus()==0)&&(UART1_InStatus()==0)) {}
	data = UART1_InChar();
	count++;
	if (count <= max) {
		*pt = data;
	}
	if (data != fcs) {
		fcserr++;
		SetMRDY();        //   MRDY=1
		return APFAIL;
		//return 12;
	}
// 4) Make MRDY=1
	SetMRDY();        //   MRDY=1
// 5) wait for SRDY to be high
	waitCount = 0;
	while (ReadSRDY() == 0) {
		waitCount++;
	}
	return APOK;
}

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
int AP_SendMessageResponse(uint8_t *msgPt, uint8_t *responsePt, uint32_t max) {
	int result;
//  uint32_t timeout;
	result = AP_SendMessage(msgPt);
	if (result == APFAIL) {
		return APFAIL;
	}

	result = AP_RecvMessage(responsePt, max);
#ifdef APDEBUG
	AP_EchoSendMessage(msgPt);  // debugging
	AP_EchoReceived(result);// debugging
#endif
	if (result  == APFAIL) {
		return APFAIL;
	}
	return APOK;
}


typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
const uint32_t MAXCHARACTERISTICS=CONST_MAXCHARACTERISTICS;
uint32_t CharacteristicCount=0;
characteristic_t CharacteristicList[CONST_MAXCHARACTERISTICS];
//characteristic_t CharacteristicList[10];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
const uint32_t NOTIFYMAXCHARACTERISTICS=CONST_NOTIFYMAXCHARACTERISTICS;
uint32_t NotifyCharacteristicCount=0;
NotifyCharacteristic_t NotifyCharacteristicList[CONST_NOTIFYMAXCHARACTERISTICS];
//NotifyCharacteristic_t NotifyCharacteristicList[4];


//*********AP_GetNotifyCCCD*******
// Return notification CCCD from the communication interface
// this does not perform BLU communication, it reads current CCCD value of notify characteristic
// Inputs:  index into which notify characteristic to return
// Outputs: 16-bit CCCD value of the notify characteristic
uint16_t AP_GetNotifyCCCD(uint32_t i){
  return (NotifyCharacteristicList[i].CCCDvalue);
}

//*************AP_SendNotification**************
// Send a notification (will skip if CCCD is 0)
// Input:  index into notify characteristic to send
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int AP_SendNotification(uint32_t i){ uint16_t handle; uint32_t j;uint8_t thedata;
  int r1; uint32_t s;
  if(i>= NotifyCharacteristicCount) return APFAIL;   // not valid
  if(NotifyCharacteristicList[i].CCCDvalue){         // send only if active
    handle = NotifyCharacteristicList[i].theHandle;
    if(handle == 0) return APFAIL; // not open
    NPI_SendNotificationIndication[1] = 6+NotifyCharacteristicList[i].size;      // 1 to 8 bytes
    OutString("\n\rSend data=");
    s = NotifyCharacteristicList[i].size;
    for(j=0; j<s; j++){
      thedata = NotifyCharacteristicList[i].pt[s-j-1]; // fetch data from user little endian to SNP big endian
      OutUHex(thedata); OutString(", ");
      NPI_SendNotificationIndication[11+j] = thedata;    // copy into message, big endian
    }
    NPI_SendNotificationIndication[7] = handle&0x0FF; // handle
    NPI_SendNotificationIndication[8] = handle>>8;
    r1=AP_SendMessageResponse(NPI_SendNotificationIndication,RecvBuf,RECVSIZE);
  }else{
    r1 = APOK; // no need to notify
  }
  return r1; // OK or fail depending on SendNotificationIndication
}

//------------AP_SendMessage------------
// sends a message to the Bluetooth module
// calculates/sends FCS at end
// FCS is the 8-bit EOR of all bytes except SOF and FCS itself
// 1) Send NPI package (it will calculate fcs)
// 2) Wait for entire message to be sent
// Input: pointer to NPI encoded array
// Output: APOK on success, APFAIL on timeout
int AP_SendMessage(uint8_t *pt) {
	uint8_t fcs;
	uint32_t waitCount;
	uint8_t data;
	uint32_t size;
// 1) Make MRDY=0
	ClearMRDY();
// 2) wait for SRDY to be low
	waitCount = 0;
	while (ReadSRDY()) {
		waitCount++;
		if (waitCount > APTIMEOUT) {
			TimeOutErr++;  // no response error
			return APFAIL; // timeout??
		}
	}
	//Clock_Delay1ms(5);
// 3) Send NPI package
	size = AP_GetSize(pt);
	fcs = 0;
	UART1_OutChar(SOF);
	pt++;
	data = *pt;	UART1_OutChar(data); fcs = fcs ^ data;	pt++;   // LSB length
	data = *pt;	UART1_OutChar(data); fcs = fcs ^ data;	pt++;   // MSB length
	data = *pt;	UART1_OutChar(data); fcs = fcs ^ data;	pt++;   // CMD0
	data = *pt;	UART1_OutChar(data); fcs = fcs ^ data;	pt++;   // CMD1
	int i;
	for (i = 0; i < size; i++) {
		data = *pt;
		UART1_OutChar(data);
		fcs = fcs ^ data;
		pt++; // payload
	}
	UART1_OutChar(fcs);                                  // FCS

// 4) Wait for entire message to be sent
	UART1_FinishOutput();
// 5) Make MRDY=1
	SetMRDY();        //   MRDY=1
// 6) wait for SRDY to be high
	waitCount = 0;
	while (ReadSRDY() == 0) {
		waitCount++;
		if (waitCount > APTIMEOUT) {
			TimeOutErr++;  // no response error
			return APFAIL; // timeout??
		}
	}
	return APOK;
}

//***********AP_GetSize***************
// returns the size of an NPI message
// Inputs:  pointer to NPI message
// Outputs: size of the message
// extracted little Endian from byte 1 and byte 2
uint32_t AP_GetSize(uint8_t *pt) {
	uint8_t msb, lsb;
	uint32_t size;
	lsb = (uint8_t) pt[1];
	msb = (uint8_t) pt[2];
	size = (msb << 8) + lsb;
	return size;
}

#ifdef APDEBUG
// *****AP_EchoSendMessage**************
// For debugging, sends message to UART0
// Inputs:  pointer to message
// Outputs: none
void AP_EchoSendMessage(uint8_t *sendMsg){ int i;uint8_t fcs;
  uint32_t size=AP_GetSize(sendMsg);
  fcs = 0;
  for(i=1;i<size+5;i++)fcs = fcs^sendMsg[i];
  OutString("\n\rLP->SNP ");
  for(i=0; i<=(4+size); i++){
    OutUHex2(sendMsg[i]); OutChar(',');
  }
  OutUHex2(fcs); //  FCS, calculated and not in messsage
}
// *****AP_EchoReceived**************
// for debugging, sends RecvBuf from SNP to UART0
// Inputs:  result APOK or APFAIL
// Outputs: none
void AP_EchoReceived(int response){ uint32_t size; int i;
  if(response==APOK){
    OutString("\n\rSNP->LP ");
    size = AP_GetSize(RecvBuf);
    for(i=0; i<=(4+size); i++){
      OutUHex2(RecvBuf[i]); OutChar(',');
    }
    OutUHex2(RecvBuf[i]); // FCS
  }else{
    OutString("\n\rfrom SNP fail");
  }
}
#else
#define AP_EchoSendMessage(MESSAGE)
#define AP_EchoReceived(R)
#endif


void Init_BLE(void)
{
	ClearReset();     // RESET=0
	SetMRDY();        //   MRDY=1
	SetReset();     // RESET=0
}

// ****AP_BackgroundProcess****
// handle incoming SNP frames
// Inputs:  none
// Outputs: none
void AP_BackgroundProcess(void){
  int count; uint16_t h; int i,j;
  uint32_t s; // size of user data 1,2,4,8
  uint32_t d; // difference between packet size and user data size
  uint8_t responseNeeded;

  if(AP_RecvStatus()){
    if(AP_RecvMessage(RecvBuf,RECVSIZE)==APOK){
      OutString("\n\rRecvMessage");
      AP_EchoReceived(APOK);
      if((RecvBuf[3]==0x55)&&(RecvBuf[4]==0x88)){// SNP Characteristic Write Indication (0x88)
        h = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this characteristic
        responseNeeded = RecvBuf[9];
        i = 0;
        while(i<MAXCHARACTERISTICS){
          if(CharacteristicList[i].theHandle == h){
            count = RecvBuf[1]-7;   // number of bytes in message
            s = CharacteristicList[i].size;
            if(count>s)count=s;   // truncate to size
            d = s-count;
            for(j=0;j<s;j++){     // if message is smaller than size
              CharacteristicList[i].pt[j] = 0; // fill MSbytes with 0
            }
            for(j=0;j<count;j++){ // write data
              CharacteristicList[i].pt[s-j-1-d] = RecvBuf[12+j];
            }
            (*CharacteristicList[i].callBackWrite)(); // process Characteristic Write Indication
            i = MAXCHARACTERISTICS;
          }else{
            i++;
          }
        }
        if(responseNeeded){
          AP_SendMessage(NPI_WriteConfirmation);
          AP_EchoSendMessage(NPI_WriteConfirmation);
        }
      }
      if((RecvBuf[3]==0x55)&&(RecvBuf[4]==0x87)){// SNP Characteristic Read Indication (0x87)
        h = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this characteristic
        i = 0;
        while(i<MAXCHARACTERISTICS){
          if(CharacteristicList[i].theHandle == h){
            (*CharacteristicList[i].callBackRead)(); // process Characteristic Read Indication
            NPI_ReadConfirmation[1] = 7+CharacteristicList[i].size;
            s = CharacteristicList[i].size;
            for(j=0;j<s;j++){ // write data
              NPI_ReadConfirmation[j+12]=CharacteristicList[i].pt[s-j-1];
            }
            i = MAXCHARACTERISTICS;
          }else{
            i++;
          }
        }
        NPI_ReadConfirmation[8] = RecvBuf[7]; // handle
        NPI_ReadConfirmation[9] = RecvBuf[8];
        AP_SendMessage(NPI_ReadConfirmation);
        AP_EchoSendMessage(NPI_ReadConfirmation);
      }
      if((RecvBuf[3]==0x55)&&(RecvBuf[4]==0x8B)){// SNP CCCD Updated Indication (0x8B)
        h = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this characteristic
        responseNeeded = RecvBuf[9];
        for(i=0; i<NOTIFYMAXCHARACTERISTICS;i++){
          if(NotifyCharacteristicList[i].CCCDhandle == h){  // to do
            NotifyCharacteristicList[i].CCCDvalue = (RecvBuf[11]<<8)+RecvBuf[10];
            NotifyCharacteristicList[i].callBackCCCD();
          }
        }
        if(responseNeeded){
          AP_SendMessage(NPI_CCCDUpdatedConfirmation);
          AP_EchoSendMessage(NPI_CCCDUpdatedConfirmation);
        }
      }
    }
  }
}


