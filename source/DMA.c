/*
 * DMA.c
 *  Set up DMA to read from memory and transfer to DAC2
 *  Created on: Feb 27, 2025
 *      Author: joshbenner
 *
 */

/*************************************************************************
* Include Project Master Header File
************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "os.h"
#include "app_cfg.h"
#include "assert.h"
#include "DMA.h"
#include "DAC.h"
#include "UserSettings.h"

/***************************************************************************
 * Variables
 ***************************************************************************/
typedef struct {
    INT32U index;
    OS_SEM flag;
}DMA_BUFFER;

static DMA_BUFFER dmaBuffer;

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void EDMA_0_CH0_IRQHandler(void);

/******************************************************************************
 * Init DMA and create  semaphore
 * INT16U *sineTable is an address to the varible sine wave table used to
 * generate sine output
 ******************************************************************************/
void DMAInit(INT16U *sineTable) {

    // Initialize buffer index
    OS_ERR os_err;
    OSSemCreate(&(dmaBuffer.flag), "DMA Semaphore", 1, &os_err);
    assert(os_err == OS_ERR_NONE);

    // Enable DMA0 clock
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_DMA0(1);

    /**** START: DMA config */
    // Set initial source address to the first buffer block
    DMA0->CH[WAVE_DMA_CH].TCD_SADDR = DMA_TCD_SADDR_SADDR(sineTable);

    // Offset for source
    DMA0->CH[WAVE_DMA_CH].TCD_SOFF = DMA_TCD_SOFF_SOFF(WAVE_BYTES_PER_SAMPLE);

    // Source size is 2 bytes, destination size is 2 bytes
    DMA0->CH[WAVE_DMA_CH].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(SIZE_CODE_16BIT)
                                   | DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(SIZE_CODE_16BIT);

    // Do not reset source address
    //After major loop is finished set the address back to the first byte of the block
   DMA0->CH[WAVE_DMA_CH].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(-(WAVE_BYTES_PER_BLOCK)*2);

    // Set destination address to DAC register
    DMA0->CH[WAVE_DMA_CH].TCD_DADDR = DMA_TCD_DADDR_DADDR(&DAC2->DATA);

    // No change in destination address
    DMA0->CH[WAVE_DMA_CH].TCD_DOFF = DMA_TCD_DOFF_DOFF(0);

    // No adjustment to destination address
    DMA0->CH[WAVE_DMA_CH].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);

    // Minor loop size
    DMA0->CH[WAVE_DMA_CH].TCD_NBYTES_MLOFFNO = DMA_TCD_NBYTES_MLOFFNO_NBYTES(WAVE_BYTES_PER_SAMPLE);

    // Set minor loop iteration counters
    DMA0->CH[WAVE_DMA_CH].TCD_CITER_ELINKNO = DMA_TCD_CITER_ELINKNO_ELINK(0) |
                                              DMA_TCD_CITER_ELINKNO_CITER(WAVE_SAMPLES_PER_BLOCK*2);
    DMA0->CH[WAVE_DMA_CH].TCD_BITER_ELINKNO = DMA_TCD_BITER_ELINKNO_ELINK(0) |
                                              DMA_TCD_BITER_ELINKNO_BITER(WAVE_SAMPLES_PER_BLOCK*2);

    // Enable half and full transfer interrupts
    DMA0->CH[WAVE_DMA_CH].TCD_CSR = DMA_TCD_CSR_BWC(3) | DMA_TCD_CSR_INTHALF(1) | DMA_TCD_CSR_INTMAJOR(1);

    // Configure timer trigger
    SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_TIMER2(1);
    SYSCON0->CTIMERCLKSEL[2] = SYSCON_CTIMERCLKSEL_SEL(1);
    SYSCON0->CTIMERCLKDIV[2] = 0;
    while (SYSCON0->CTIMERCLKDIV[2] & SYSCON_CTIMERXCLKDIV_CTIMERCLKDIV_UNSTAB(1)) {
    }
    CTIMER2->TCR = CTIMER_TCR_CEN(0);
    CTIMER2->MCR = CTIMER_MCR_MR0R(1);
    CTIMER2->PR = CTIMER_PR_PRVAL(0);
    CTIMER2->MR[0] = CTIMER_MR_MATCH(781 - 1);
    CTIMER2->TCR |= CTIMER_TCR_CEN(1);

    // Set DMA trigger source
    DMA0->CH[WAVE_DMA_CH].CH_MUX = DMA_CH_MUX_SRC(11);

    // Enable DMA channel
    DMA0->CH[WAVE_DMA_CH].CH_CSR = DMA_CH_CSR_ERQ(1);

    // Enable DMA interrupts in NVIC
    NVIC_ClearPendingIRQ(EDMA_0_CH0_IRQn);
    NVIC_EnableIRQ(EDMA_0_CH0_IRQn);
}

/******************************************************************************
 * DMA Interrupt handler
 * Toggle index and post semaphore
 ******************************************************************************/
void EDMA_0_CH0_IRQHandler(void) {
    OS_ERR os_err;
    OSIntEnter();

    // Clear interrupt flag
    DMA0->CH[WAVE_DMA_CH].CH_INT = DMA_CH_INT_INT(1);

    DB0_TURN_ON();

    // Toggle buffer index
    dmaBuffer.index ^= 1u;

    // Signal task that data is ready
    (void)OSSemPost(&(dmaBuffer.flag), OS_OPT_POST_1, &os_err);
    assert(os_err == OS_ERR_NONE);

    DB0_TURN_OFF();

    OSIntExit();
}

/******************************************************************************
 * DMAPend
 * Pend on the DMA ISR
 * Return the updated index
 ******************************************************************************/
INT32U DMAPend(OS_TICK tout, OS_ERR *os_err){
    OSSemPend(&(dmaBuffer.flag),tout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err);
    return(dmaBuffer.index);
}
