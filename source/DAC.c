/*
 *   Init DAC2
 *
 *  Important notes:
 *  DAC2 out put is Bottom Right Right if looking at charger. (J3 -> Pin2)
 *
 *  Created on: Feb 21, 2025
 *      Author: joshbenner
 */

/*************************************************************************
* Include Project Master Header File
************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "DAC.h"

/************************************************************************
* Project Defined Constants
************************************************************************/
#define DAC2_VREFPW_EN  0x41 // DAC2 and VREF power bit position enable

/****************************************************************************************
 * DAC2 initialization.
 * As simple as it gets, no FIFO, no triggering, no interrupts or DMA.
 * Note that VREF has to be powered up to provide ztc and ptat currents.
 * *************************************************************************************/

void DACInit(void) {
    SYSCON0->DAC[2].CLKDIV = 2; // Set clock divider (150/3 = 50MHz)
    // Enable clock for DAC2
    SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_DAC2(1);

    // Select PLL0 clock for DAC2
    SYSCON0->DAC[2].CLKSEL = SYSCON_DAC_CLKSEL_SEL(1);

    // Wait for DAC2 clock divider to stabilize
    while (SYSCON0->DAC[2].CLKDIV & SYSCON_DAC_CLKDIV_UNSTAB(1)) {
    }

    // Enable DAC2 and VREF power
    SPC0->ACTIVE_CFG1 |= SPC_ACTIVE_CFG1_SOC_CNTRL(DAC2_VREFPW_EN);

    // Reset DAC2
    SYSCON0->PRESETCTRLSET[3] = SYSCON_PRESETCTRL3_DAC2_RST(1);
    SYSCON0->PRESETCTRLCLR[3] = SYSCON_PRESETCTRL3_DAC2_RST(1);

    // Configure DAC2
    DAC2->GCR |=  HPDAC_GCR_BUF_EN(1);

    // Enable DAC2
    DAC2->GCR |= HPDAC_GCR_DACEN(1);

}

