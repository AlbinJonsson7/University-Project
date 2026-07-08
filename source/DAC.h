/*
 * DAC.h
 *
 *   Init DAC2
 *
 *  Important notes:
 *  DAC2 out put is Bottom Right Right if looking at charger. (J3 -> Pin2)
 *
 *  Created on: Feb 21, 2025
 *      Author: joshbenner
 */

#ifndef DAC_H_
#define DAC_H_
/* *********************** Public Function  Prototype ************************/
/****************************************************************************************
 * DAC2 initialization.
 * As simple as it gets, no FIFO, no triggering, no interrupts or DMA.
 * Note that VREF has to be powered up to provide ztc and ptat currents.
 * *************************************************************************************/
void DACInit(void);

#endif /* DAC_H_ */
