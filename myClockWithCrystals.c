// ----------------------------------------------------------------------------
// myClocksWithCrystals.c

// This routine sets up the Low Frequency crystal (LFXT) and high-freq
// internal clock source (DCO). Then configures ACLK, SMCLK, and MCLK:
//    ACLK  = 32KHz
//    SMCLK =  4MHz
//    MCLK  =  24MHz
// ----------------------------------------------------------------------------

//***** Header Files **********************************************************
#include <stdbool.h>
#include <driverlib.h>
#include "myClocks.h"


//***** Defines ***************************************************************
#define LF_CRYSTAL_FREQUENCY_IN_HZ     32768                           // 32KHz
#define HF_CRYSTAL_FREQUENCY_IN_HZ     4000000                         // 4MHz

#define MCLK_DESIRED_FREQUENCY_IN_KHZ  24000                           // 8MHz -> 24MHz
#define MCLK_FLLREF_RATIO              MCLK_DESIRED_FREQUENCY_IN_KHZ / ( HF_CRYSTAL_FREQUENCY_IN_HZ / 4000)


#define XT_TIMEOUT                     50000


//***** Global Variables ******************************************************
uint32_t myACLK  = 0;
uint32_t mySMCLK = 0;
uint32_t myMCLK  = 0;

uint8_t  returnValue = 0;
bool     bReturn     = STATUS_FAIL;


//***** initClocks ************************************************************
void initClocks(void) {
     // Set core voltage level to handle 8MHz clock rate
     PMM_setVCore( PMM_CORE_LEVEL_3); //Change from 1 to 3

    //**************************************************************************
    // Configure Oscillators
    //**************************************************************************
    // Initialize the XT1 and XT2 crystal frequencies being used
    //  so driverlib knows how fast they are
    UCS_setExternalClockSource(
            LF_CRYSTAL_FREQUENCY_IN_HZ,
            HF_CRYSTAL_FREQUENCY_IN_HZ
    );

    // Verify if the default clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();

    // Initialize XT1. Returns STATUS_SUCCESS if initializes successfully.
    bReturn = UCS_turnOnLFXT1WithTimeout(
                  UCS_XT1_DRIVE_0,
                  UCS_XCAP_3,
                  XT_TIMEOUT
              );

    if ( bReturn == STATUS_FAIL )
    {
        while( 1 );
    }

    // Initializes the XT2 crystal oscillator with no timeout.
    // In case of failure, code hangs here.
    // For time-out instead of code hang use UCS_turnOnXT2WithTimeout().
    UCS_turnOnXT2( UCS_XT2_DRIVE_4MHZ_8MHZ );

    //**************************************************************************
    // Configure Clocks
    //**************************************************************************
    // Select XT1 as ACLK source (32KHz)
    UCS_initClockSignal(
            UCS_ACLK,                                    // Clock you're configuring
            UCS_XT1CLK_SELECT,                           // Clock source
            UCS_CLOCK_DIVIDER_1                          // Divide down clock source by this much
    );

    // Select XT2 as SMCLK source
    UCS_initClockSignal(
            UCS_SMCLK,                                   // Clock you're configuring
            UCS_XT2CLK_SELECT,                           // Clock source
            UCS_CLOCK_DIVIDER_1                          // Divide down clock source by this much
    );


   UCS_initClockSignal(
             UCS_FLLREF,         // The reference for Frequency Locked Loop
             UCS_XT2CLK_SELECT,  // Select XT2
             UCS_CLOCK_DIVIDER_4 // The FLL reference will be 1 MHz (4MHz XT2/4)
       );

    // Configure the FLL's frequency and set MCLK & SMCLK to use the FLL as their source (8MHz)
    UCS_initFLLSettle(
            MCLK_DESIRED_FREQUENCY_IN_KHZ,               // MCLK frequency
            MCLK_FLLREF_RATIO                            // Ratio between MCLK and FLL's reference clock source
    );

    // Verify that the modified clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();

    // Select XT2 as SMCLK source
    // Re-doing this call ... if you use UCS_initFLLSettle to
    //   setup MCLK, it also configures SMCLK; therefore, you
    //   should call this function afterwards
    UCS_initClockSignal(
            UCS_SMCLK,
            UCS_XT2CLK_SELECT,
            UCS_CLOCK_DIVIDER_1
    );

    // Verify that the modified clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();
}


