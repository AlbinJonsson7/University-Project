/* DMA.c]h
 *  Set up DMA to read from memory and transfer to DAC2
 *  Created on: Feb 27, 2025
 *      Author: joshbenner
 *
 */

#ifndef DMA_H_
#define DMA_H_

/*******************Public Functino Prototype*********************************/
/******************************************************************************
 * Init DMA and create  semaphore
 * INT16U *sineTable is an address to the varible sine wave table used to
 * generate sine output
 ******************************************************************************/
void DMAInit(INT16U *sineTable);
/******************************************************************************
 * DMAPend
 * Pend on the DMA ISR
 * Return the updated index
 ******************************************************************************/
INT32U DMAPend(OS_TICK tout, OS_ERR *os_err);

/* Define Variables  */
#define WAVE_SAMPLES_PER_BLOCK 6500
#define WAVE_BYTES_PER_SAMPLE 2
#define WAVE_BYTES_PER_BLOCK   (WAVE_SAMPLES_PER_BLOCK * WAVE_BYTES_PER_SAMPLE)
#define WAVE_DMA_CH 0
#define SIZE_CODE_16BIT 1

#endif /* DMA_H_ */
