//============================================================================
// Name        : AP_BLE.c
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
 * AP_BLE.c
 *
 *  Created on: Dec 26, 2018
 *      Author: Mahendra Gunawardena
 */


#include <stdint.h>
#include "UART0.h"
#include "driverlib.h"
#include "AP_BLE.h"
#include "AP.h"
#include "common.h"

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

//****links into AP.c**************
//extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];



// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg) {

	uint8_t fcs;
	uint8_t data;
	uint32_t size;
	size = AP_GetSize(msg);
	fcs = 0;
	msg++;							 				//Skip SOF
	data = *msg;	fcs = fcs ^ data;	msg++;		// LSB length
	data = *msg;	fcs = fcs ^ data; 	msg++; 		// MSB length
	data = *msg; 	fcs = fcs ^ data; 	msg++;		// CMD0
	data = *msg; 	fcs = fcs ^ data; 	msg++;		// CMD1
	int i;
	for (i = 0; i < size; i++) {
		data = *msg;
		fcs = fcs ^ data;
		msg++; // payload
	}
	*msg = fcs;
}

//*************BuildGetStatusMsg**************
// Create a Get Status message, used MSP430F5529 + CC2650 BLE Demo
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg) {
	msg[0] = SOF;   // assign SOF then increment msg pointer
	msg[1] = 0x00;  // LSB length=0, no payload
	msg[2] = 0x00;  // MSB length
	msg[3] = 0x55; // CMD0 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	msg[4] = 0x06; // CMD1 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	SetFCS(msg);// msg is incremented. use pt w/c is unincremented to point back to compute fcs
}

//*************BLE_GetStatus**************
// Get status of connection, used in MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress

uint32_t BLE_GetStatus(void) {volatile int r; uint8_t sendMsg[8];
	  OutString("\n\rGet Status");
	  BuildGetStatusMsg(sendMsg);
	  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
	  return (((uint32_t)RecvBuf[4]<<24)+((uint32_t)RecvBuf[5]<<16)+((uint32_t)RecvBuf[6]<<8)+((uint32_t)RecvBuf[7]));
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used MSP430F5529 + CC2650 BLE Demo
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status

void BuildGetVersionMsg(uint8_t *msg){
// Note NPI_GetVersion in AP.c for details

 	msg[0]=SOF;   // assign SOF then increment msg pointer
	msg[1]=0x00;  // LSB length=0, no payload
	msg[2]=0x00;  // MSB length
	msg[3]=0x35;  // CMD0 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	msg[4]=0x03;  // CMD1 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}


//*************BLE_GetVersion**************
// Get version of the SNP application running on the CC2650, used MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: version
uint32_t BLE_GetVersion(void){volatile int r;uint8_t sendMsg[8];

	OutString("\n\rGet Version");
	BuildGetVersionMsg(sendMsg);
	r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
	return (RecvBuf[5]<<8)+(RecvBuf[6]);

}

//*************BuildAddServiceMsg**************
// Create an Add service message, used MSP430F5529 + CC2650 BLE Demo
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){

	*msg=SOF;   // assign SOF then increment msg pointer
	*(msg+1)=3;		  // LSB length=3, no payload
	*(msg+2)=0x00;  // MSB length
	*(msg+3)=0x35;  // CMD0 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x81;  // CMD1 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x01;  // Primary Service
	*(msg+6)=uuid&0xFF;
	*(msg+7)=uuid>>8;
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}

  //*************BLE_AddService**************
  // Add a service, used MSP430F5529 + CC2650 BLE Demo
  // Inputs uuid is 0xFFF0, 0xFFF1, ...
  // Output APOK if successful,
  //        APFAIL if SNP failure
int BLE_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
	OutString("\n\rAdd service");
    BuildAddServiceMsg(uuid,sendMsg);
    r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
    return r;
}

//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used MSP430F5529 + CC2650 BLE Demo
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){

 	msg[0]=SOF;   // assign SOF then increment msg pointer
	msg[1]=0x00;  // LSB length=0, no payload
	msg[2]=0x00;  // MSB length
	msg[3]=0x35;  // CMD0 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	msg[4]=0x84;  // CMD1 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}

//*************BLE_RegisterService**************
// Register a service, used MSP430F5529 + CC2650 BLE Demo
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int BLE_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}


//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used MSP430F5529 + CC2650 BLE Demo
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,
  uint8_t permission, uint8_t properties, uint8_t *msg){
// set RFU to 0 and
// set the maximum length of the attribute value=512

	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=8;					  // LSB length=3, no payload
	*(msg+2)=0x00;			  // MSB length
	*(msg+3)=0x35;			  // CMD0 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x82;			  // CMD1 from API table, page 11 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=permission;  // Permission
	*(msg+6)=properties;	// Properties
	*(msg+7)=0x00;				// Properties
	*(msg+8)=0x00;				// RFU
	*(msg+9)=0x00;				// Length LSB
	*(msg+10)=0x02;				// Length MSB Maximum length of the attribute value=512
	*(msg+11)=uuid&0xFF;
	*(msg+12)=uuid>>8;
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in  BLE
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 32 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read

	int i;
	i=0;
  while((i<20)&&(name[i])){
    *(msg+11+i) = name[i]; i++;
  }
 	*(msg+11+i)=0;i++;		// Add null terminator

 	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=6+i;					// LSB length,
	*(msg+2)=0x00;			  // MSB length
	*(msg+3)=0x35;			  // CMD0 from API table, page  of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x83;			  // CMD1 from API table, page  of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x80;			  // User Description String
	*(msg+6)=0x01;				// GATT Read Permissions
	*(msg+7)=i;						// Maximum Possible length of the user description string
	*(msg+8)=0x00;				//
	*(msg+9)=i;						// Initial length of the user description string
	*(msg+10)=0x00;				//
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}


//*************BLE_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in MSP430F5529 + CC2650 BLE Demo
//        for notify properties, call AP_AddNotifyCharacteristic
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int BLE_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle;
  uint8_t sendMsg[32];
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
}

//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in MSP430F5529 + CC2650 BLE Demo
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write

	int i;
	i=0;
  while((i<19)&&(name[i])){
    *(msg+12+i) = name[i]; i++;
  }
 	*(msg+12+i)=0;i++;		// Add null terminator

 	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=7+i;					// LSB length,
	*(msg+2)=0x00;			  // MSB length
	*(msg+3)=0x35;			  // CMD0 from API table, page  of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x83;			  // CMD1 from API table, page  of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x84;			  // User Description String and CCCD permissions
	*(msg+6)=0x03;				// CCCD parameters read+write
	*(msg+7)=0x01;				// GATT Read Permissions
	*(msg+8)=i;						// Maximum Possible length of the user description string
	*(msg+9)=0x00;				//
	*(msg+10)=i;						// Initial length of the user description string
	*(msg+11)=0x00;				//
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}

//*************BLE_AddNotifyCharacteristic**************
// Add a notify characteristic, used in MSP430F5529 + CC2650 BLE Demo
//        for read, write, or read/write characteristic, call AP_AddCharacteristic
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int BLE_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle;
  uint8_t sendMsg[36];
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK
}

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in MSP430F5529 + CC2650 BLE Demo
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
	int i;
	i=0;
  while((i<24)&&(name[i])){
    *(msg+8+i) = name[i]; i++;
  }
// 	*(msg+8+i)=0;i++;		// Add null terminator

 	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=3+i;					// LSB length,
	*(msg+2)=0x00;			  // MSB length
	*(msg+3)=0x35;			  // CMD0 from API table, page 34 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x8C;			  // CMD1 from API table, page 34 of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x01;			  // Generic Access Service
	*(msg+6)=0x00;				// GATT Read Permissions
	*(msg+7)=0x00;				// GATT Read Permissions
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in MSP430F5529 + CC2650 BLE Demo
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){
// Non-connectable Advertisement Data
// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
// Texas Instruments Company ID 0x000D
// TI_ST_DEVICE_ID = 3
// TI_ST_KEY_DATA_ID
// Key state=0

	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=11;					// LSB length=11
	*(msg+2)=0x00;			  // MSB length
												// SNP Set Advertisement Data
	*(msg+3)=0x55;			  // CMD0 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x43;			  // CMD1 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x01;  			// Not connected Advertisement Data
	*(msg+6)=0x02;				// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
	*(msg+7)=0x01;				// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
	*(msg+8)=0x06;				// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
	*(msg+9)=0x06;				// Length LSB , length, manufacturer specific
	*(msg+10)=0xFF;				// Length MSB Maximum length of the attribute value=512
	*(msg+11)=0x0D;				// Texas Instruments Company ID
	*(msg+12)=0x00;
	*(msg+13)=0x03;				// TI_ST_DEVICE_ID
	*(msg+14)=0x00;				// TI_ST_KEY_DATA_ID
	*(msg+15)=0x00;				// Key state
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in MSP430F5529 + CC2650 BLE Demo
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){

	int i;
	i=0;
  while((i<24)&&(name[i])){
    *(msg+8+i) = name[i]; i++;
	}

 	*msg=SOF;							// assign SOF then increment msg pointer
	*(msg+1)=3+i+9;;				// LSB length=11:  3+i+9;
	*(msg+2)=0x00;			  // MSB length
												// SNP Set Advertisement Data
	*(msg+3)=0x55;			  // CMD0 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x43;			  // CMD1 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x00;  			// Scan Response Data
	*(msg+6)=i+1;					// length, i start at 0, therefore need to add 1
	*(msg+7)=0x09;				// type=LOCAL_NAME_COMPLETE
	// connection interval range
	*(msg+7+i+1)=0x05;		// length of this data
	*(msg+7+i+2)=0x12;		// GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
	*(msg+7+i+3)=0x50;		// DEFAULT_DESIRED_MIN_CONN_INTERVAL
	*(msg+7+i+4)=0x00;		// DEFAULT_DESIRED_MIN_CONN_INTERVAL
	*(msg+7+i+5)=0x20;		// DEFAULT_DESIRED_MAX_CONN_INTERVAL
	*(msg+7+i+6)=0x03;		// DEFAULT_DESIRED_MAX_CONN_INTERVAL
	// Tx power level
	*(msg+7+i+7)=0x02;		// length of this data
	*(msg+7+i+8)=0x0A;		// GAP_ADTYPE_POWER_LEVEL
	*(msg+7+i+9)=0x00;		// 0dBm
	SetFCS(msg);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in MSP430F5529 + CC2650 BLE Demo
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){

	*msg=SOF;									// assign SOF then increment msg pointer
	*(msg+1)=14;								// LSB length=11
	*(msg+2)=0x00;			  					// MSB length
												// SNP Start Advertisement
	*(msg+3)=0x55;			  					// CMD0 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+4)=0x42;			  					// CMD1 from API table, page of 41 SimpleLink™ Bluetooth® Low Energy CC2640 Wireless MCU
	*(msg+5)=0x00;  							// Connectable Undirected Advertisements
	*(msg+6)=0x00;								// Advertise infinitely
	*(msg+7)=0x00;								// Advertise infinitely
	*(msg+8)=interval&0xFF;						// Advertising Interval
	*(msg+9)=interval>>8;						// Advertising Interval
	*(msg+10)=0x00;								// Filter Policy RFU
	*(msg+11)=0x00;								// Initiator Address Type RFU
	*(msg+12)=0x00;								// RFU
	*(msg+13)=0x01;								// RFU
	*(msg+14)=0x00;								// RFU
	*(msg+15)=0x00;								// RFU
	*(msg+16)=0x00;								// RFU
	*(msg+17)=0xC5;								// RFU
	*(msg+18)=0x02;								// Advertising will restart with connectable advertising when a connection is terminated
	SetFCS(msg);								// msg is incremented. use pt w/c is unincremented to point back to compute fcs

}


//*************BLE_StartAdvertisement**************
// Start advertisement, used in MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int BLE_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("MSP430 CC2650 BLE",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("MSP430 CC2650 BLE",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

