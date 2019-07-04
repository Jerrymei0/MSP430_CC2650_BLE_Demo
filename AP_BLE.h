//============================================================================
// Name        : AP_BLE.h
// Author      : Mahendra Gunawardena
// Date        : 12/26/2018
// Version     : Rev 0.01
// Copyright   : Your copyright notice
// Description : Applicaiton Progamming Interface for the MSP430F5529 + CC2650 BLE Demo
//============================================================================
/*
 * AP_BLE.h
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
 * AP_BLE.h
 *
 *  Created on: Dec 26, 2018
 *      Author: Mahendra Gunawardena
 */

// BLE prototypes

//*************BLE_GetStatus**************
// Get status of connection, used MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress

uint32_t BLE_GetStatus(void);


//*************BLE_GetVersion**************
// Get version of the SNP application running on the CC2650, used in MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: version
uint32_t BLE_GetVersion(void);


//*************BLE_AddService**************
// Add a service, used in MSP430F5529 + CC2650 BLE Demo
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int BLE_AddService(uint16_t uuid);

//*************BLE_RegisterService**************
// Register a service, used MSP430F5529 + CC2650 BLE Demo
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int BLE_RegisterService(void);

//*************BLE_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used MSP430F5529 + CC2650 BLE Demo
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

int BLE_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission, uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void));

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
  char name[], void(*CCCDfunc)(void));


//*************BLE_StartAdvertisement**************
// Start advertisement, used in MSP430F5529 + CC2650 BLE Demo
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int BLE_StartAdvertisement(void);

