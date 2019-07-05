//============================================================================
//
// Name        : Main.c
// Author      : Mahendra Gunawardena
// Date        : 12/26/2018
// Version     : Rev 0.01
// Copyright   : Your copyright notice
// Description : Main Program for MSP430F5529 + CC2650 BLE Demo
//             : On boot up both LED1 and LED2 are OFF
//             : When switch 1 (S1) is pressed LED1 is switched on and LED2 is switch off
//             : The LED1 ON and LED2 OFF will be transmitted to an app via the BLE
//             : When switch 2 (S2) is pressed LED2 is switched on and LED1 is switch off
//             : The LED1 OFF and LED2 ON will be transmitted to an app via the BLE
//             : When both switch 1 (S1) and switch 2 (S2) are pressed both LED1 & LED2 are  switched ON
//             : When LED1 and LED2 are ON a notification will be transmitted to an app via the BLE
//
//============================================================================
/*
 * Main.c
 * Implementation of BLE using a MSP430F5529 Launchpad and CC2650 BoosterXL
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
// ----------------------------------------------------------------------------
// main.c  ('F5529 Launchpad)
// ----------------------------------------------------------------------------

#include "driverlib.h"

//***** Header Files **********************************************************
#include <driverlib.h>
#include "myClocks.h"
#include "AP.h"
#include "AP_BLE.h"
#include "UART0.h"
#include "UART1.h"


//***** Prototypes ************************************************************
void initGPIO(void);
void Bluetooth_Init(void);

//***** Defines ***************************************************************

#define   ONE_SECOND    24000000    // MCLK_DESIRED_FREQUENCY_IN_KHZ * 1000;  Well, it's about a second  (MCLK = 24MHz)
#define   HALF_SECOND   12000000    // ONE_SECOND / 2


// MSP430_CC2650 UART ASSIGNMENT
#define MSP430_CC2650_UART_BASE_ADDR			USCI_A1_BASE		// }
#define MSP430_CC2650_UART_VECTOR				USCI_A1_VECTOR		// } The MSP430_CC2650 connection is to UART A1
#define MSP430_CC2650_UART_VECTOR_REGISTER		UCA1IV				// }

//***** Global Variables ******************************************************

//---------------- Global variables shared between tasks ----------------

volatile unsigned short button1;
volatile unsigned short button2;

static uint16_t LED1;
static uint16_t LED2;

static uint16_t LED1_Status;
static uint16_t LED2_Status;

static uint16_t LED1_LED2_Status;

int i;



// function prototypes
//******************************************************************************


//*****************************************************************************
// Main
//*****************************************************************************
void main (void)
{
	//Init Variables
	LED1_Status = 0;
	LED2_Status = 0;
	LED1_LED2_Status = 0;

    // Stop watchdog timer
    WDT_A_hold( WDT_A_BASE );

    // Initialize GPIO
    initGPIO();

    // Initialize clocks
    initClocks();

    SetupUart();

    __enable_interrupt();	// start allowing interrupts

	Bluetooth_Init();

    AP_BackgroundProcess();  // handle Bluetooth
	AP_SendNotification(0);  // if active, send notification

    while(1) {

    	for (i=0;i<8;i++){
        	button1 = GPIO_getInputPinValue(GPIO_PORT_P2,GPIO_PIN1);
        	button2 = GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1);
            __delay_cycles( HALF_SECOND );
    	}

    	if (((button1==0) && (button2==0)) == 1){
    				GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    				GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
    				LED1_LED2_Status = 1;
    				LED1_Status = 0;
    				LED2_Status = 0;
    				AP_SendNotification(0);  // if active, send notification
    				__delay_cycles(100);
    	} else if (button2==0) {
			GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
			GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
			LED2_Status = 1;
			LED1_Status = 0;
			LED1_LED2_Status = 0;
		} else if (button1==0){
			GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
			GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
			LED1_Status = 1;
			LED2_Status = 0;
			LED1_LED2_Status = 0;
		}
	    AP_BackgroundProcess();  // handle Bluetooth
		__delay_cycles(1000);
    }

}

//*****************************************************************************
// Initialize GPIO
//*****************************************************************************
void initGPIO(void) {

    // Set pin P1.0 to output direction and turn LED off
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);                   // Red LED (LED1)
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    // Set pin P4.7 to output direction and turn LED off
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);                   //  Red Green (LED2)
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);

    // Connect pins to clock crystals
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P5,
            GPIO_PIN5 +                                  // XOUT on P5.5
            GPIO_PIN4 +                                  // XIN  on P5.4
            GPIO_PIN3 +                                  // XT2OUT on P5.3
            GPIO_PIN2                                    // XT2IN  on P5.2
    );

	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);  //SRDY pull up resistor

	GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);	// RESET for BLE

	GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN3);	// MRDY for BLE

/**************************************************/

	// UART for BLE (MG)
	GPIO_setDriveStrength(GPIO_PORT_P3, GPIO_PIN3 + GPIO_PIN4,GPIO_FULL_OUTPUT_DRIVE_STRENGTH);
	// BLE UART
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN3);// TxD for UART 0
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN4);// RxD for UART 0

	// Set as input GPIO Port 4 Pin 5 and Port 1 Pin 1
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN1);

}

void SetupUart() {

	//UART0_Init();
	UART1_Init();

}

//******************************************************************************
//
//  Interrupt for MSP430_CC2650 UART
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=MSP430_CC2650_UART_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(MSP430_CC2650_UART_VECTOR)))
#endif
void UART_ISR(void) {
//	char rcvdChar;
	switch (__even_in_range(MSP430_CC2650_UART_VECTOR_REGISTER, 4)) {
	case USCI_UCRXIFG:	// received character interrupt

		break;
	case USCI_UCTXIFG:	// transmitted character interrupt, not currently used
		break;
	default:	// shouldn't happen
		break;
	}
}

// ********OutValue**********
// Debugging dump of a data value to virtual serial port to PC
// data shown as 1 to 8 hexadecimal characters
// Inputs:  response (number returned by last AP call)
// Outputs: none
void OutValue(char *label,uint32_t value){
  UART0_OutString(label);
  UART0_OutUHex(value);
}

void Bluetooth_LED1(void){ // called on a SNP Characteristic Read Indication for characteristic Time
  OutValue("\n\rRead LED 1=",LED1);
}

void Bluetooth_LED2(void){ // called on a SNP Characteristic Read Indication for characteristic Sound
  OutValue("\n\rRead LED 2=",LED2);
}

void Bluetooth_Notification(void){ // called on SNP CCCD Updated Indication
  OutValue("\n\rCCCD=",AP_GetNotifyCCCD(0));
}

void Bluetooth_Init(void){
	volatile int r;
	Clock_Delay1ms(10);
	r = AP_Init();
	BLE_GetStatus();  // optional
	BLE_GetVersion(); // optional
	BLE_AddService(0xFFF0);
/*	BLE_AddCharacteristic(0xFFF1, 1, &PlotState, 0x03, 0x0A, "PlotState", &Bluetooth_ReadPlotState, &Bluetooth_WritePlotState); */
	BLE_AddCharacteristic(0xFFF1, 2, &LED1_Status, 0x01, 0x02, "LED 1", &Bluetooth_LED1, 0);
	BLE_AddCharacteristic(0xFFF2, 4, &LED2_Status, 0x01, 0x02, "LED 2", &Bluetooth_LED2, 0);
/*	BLE_AddCharacteristic(0xFFF5, 4, &LightData, 0x01, 0x02, "Light", &Bluetooth_ReadLight, 0);*/
	BLE_AddNotifyCharacteristic(0xFFF3,2,&LED1_LED2_Status,"Both LED's",&Bluetooth_Notification);
	BLE_RegisterService();
	BLE_StartAdvertisement();
	BLE_GetStatus();
}

