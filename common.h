/*
 * common.h
 *
 *  Created on: Dec 12, 2018
 *      Author: Mahendra Gunawardena
 */

#ifndef MSP430_CC2650_R1_COMMON_H_
#define MSP430_CC2650_R1_COMMON_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#ifdef MAIN_PROC	// see description above
#define EXTERN
#else
#define EXTERN extern
#endif

#define RECVSIZE 128

// Misc defines, used in various places
#define ASCII_CR	0x0d			// ASCII value for CR
#define ASCII_LF	0x0a			// ASCII value for LF

#define EOL_STRING	"\r\n"			// CR+LF, the end of line string to be used for responses to the Tester
#define MSP430_CC2650_CMD_PREFIX	"10:"	// Cmd prefix (in or out). After this it's a series of <register:value> pairs
#define MSP430_CC2650_CMD_PREFIX_1	"12:"	// Cmd prefix (in or out). After this it's a series of <register:value> pairs

// MSP430_CC2650 UART ASSIGNMENT
#define MSP430_CC2650_UART_BASE_ADDR			USCI_A1_BASE		// }
#define MSP430_CC2650_UART_VECTOR				USCI_A1_VECTOR		// } The MSP430_CC2650 connection is to UART A1
#define MSP430_CC2650_UART_VECTOR_REGISTER		UCA1IV				// }

// BLE UART ASSIGNMENT
#define BLE_BASE_ADDR					USCI_A0_BASE		// }
#define BLE_UART_VECTOR					USCI_A0_VECTOR		// } The BLE connection is to UART A0
#define BLE_UART_VECTOR_REGISTER		UCA0IV				// }



#endif /* MSP430_CC2650_COMMON_H_ */
