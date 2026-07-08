/*
 * SineWave.c
 * Create a task that fills in an array with sinewave values
 *  Created on: Mar 7, 2025
 *      Author: joshbenner
 */


#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "os.h"
#include "app_cfg.h"
#include "assert.h"
#include "SineWave.h"
#include "DMA.h"
#include "DAC.h"
#include "UserSettings.h"


/************* Variables ***************/
static INT16U  sineTable[2][WAVE_SAMPLES_PER_BLOCK];

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/
static void SwSineTask(void *p_arg);

/*******************************************************************************
* Allocate task control blocks
*******************************************************************************/
static OS_TCB sineTaskTCB;
/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK sineTaskStk[APP_CFG_SINETASK_STK_SIZE];

/******************************************************************************
 * Init the sine wave task and all dependencies
 *****************************************************************************/
void SwSineInit(void){

    OS_ERR os_err;
    /*  DAC and DMA init */
    DACInit();
    //Create the switch task
    OSTaskCreate(&sineTaskTCB,
                "sine Task ",
                SwSineTask,
                (void *) 0,
                APP_CFG_SINETASK_PRIO,
                &sineTaskStk[0],
                (APP_CFG_SW_TASK_STK_SIZE / 10u),
                APP_CFG_SINETASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                &os_err);
    assert(os_err == OS_ERR_NONE);
    DMAInit(&sineTable[0][0]);
}

/*******************************************************************************
 * Task to generate sine wave.
 * Pend on DMA ISR half flag and full flag.
 * get index from DMA Pend
 * calculate sine table entries in opposite index that DMA is reading
 ******************************************************************************/
static void SwSineTask(void *p_arg) {
    OS_ERR os_err;
    INT32U index = 0;
    const INT32U N_q31 = 11181;
    q31_t xArg = 0;
    INT32S a_step=0;
    INT32U frequency = 0;
    INT32S amplitude = 0;

    (void)p_arg;

    while (1) {
        DB2_TURN_OFF();  // Debug bit off while waiting
        index = DMAPend(0, &os_err);  // Wait for DMA completion
        assert(os_err == OS_ERR_NONE);
        DB2_TURN_ON();  // Debug bit on when running
        a_step = getSineAmp();
        frequency = getSineFreq();

        amplitude = 360*a_step;

        // Write to the buffer NOT being used by DMA
       INT32U i = 0;
        for (i = 0; i < WAVE_SAMPLES_PER_BLOCK; i++) {
            xArg =  xArg+ N_q31*frequency;

            sineTable[!index][i] =(INT16U)( ((1<<31) + (amplitude*(arm_sin_q31(xArg)>>13)))>>18);
            }

    }
}


