/*****************************************************************************************
* A simple Function generator program for uCOS-III. In this case it uses Cesium3 (CsOS) RTOS - The
* most recent version of uCOS.
*
* It produces a sine and pulse train that can be adjusted by a quadrature encoder and
* through the terminal.
*
* This version is written for the MCXN947FRDM board.
*
* If CsOS is working you should see: sine,  1000Hz, 20, [pulse],  1000Hz,  50%
* displayed in the terminal.
* Version 2025.1
* 3/16/25 Josh Benner, David Friday, Trey McCabe, Albin Jonsson
*****************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "State.h"
#include "UserSettings.h"
#include "Frequency.h"
#include "app_cfg.h"
#include "SineWave.h"
#include "PulseTrain.h"
#include "TSI.h"
#include "DisplaySettings.h"
#include "ROTModule.h"


/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appTaskStartTCB;

/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

/*****************************************************************************************
* Task Function Prototypes. 
*   - Private if in the same module as startup task. Otherwise public.
*****************************************************************************************/
static void  appStartTask(void *p_arg);

/*****************************************************************************************
* main()
*****************************************************************************************/
void main(void) {

    OS_ERR  os_err;

    GpioDBugBitsInit();
    FRDM_MCXN947InitBootClock();
    BIOOpen(BIO_BIT_RATE_115200);	//Startup BasicIO for asserts
    CPU_IntDis();               /* Disable all interrupts, OS will enable them  */
    OSInit(&os_err);                    /* Initialize uC/OS-III                         */
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTaskStartTCB,                  /* Address of TCB assigned to task */
                 "Start Task",                      /* Name you want to give the task */
                 appStartTask,                      /* Address of the task itself */
                 (void *) 0,                        /* p_arg is not used so null ptr */
                 APP_CFG_TASK_START_PRIO,           /* Priority you assign to the task */
                 &appTaskStartStk[0],               /* Base address of task�s stack */
                 (APP_CFG_TASK_START_STK_SIZE/10u), /* Watermark limit for stack growth */
                 APP_CFG_TASK_START_STK_SIZE,       /* Stack size */
                 0,                                 /* Size of task message queue */
                 0,                                 /* Time quanta for round robin */
                 (void *) 0,                        /* Extension pointer is not used */
                 (OS_OPT_TASK_NONE),                /* Options */
                 &os_err);                          /* Ptr to error code destination */

    assert(os_err == OS_ERR_NONE);

    OSStart(&os_err);               /*Start multitasking(i.e. give control to uC/OS)    */
    assert(0);						/*Should never get here */
}

/*****************************************************************************************
* STARTUP TASK
* This should run once and be deleted. Could restart everything by creating.
*****************************************************************************************/
static void appStartTask(void *p_arg) {

    OS_ERR os_err;
    (void)p_arg;                        /* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SystemCoreClock);
    OSStatTaskCPUUsageInit(&os_err);
    GpioDBugBitsInit();
    SwInit();
    stateModuleInit();
    mutexInit();
    SwSineInit();
    FreqScan_Init();
    PT_Init();
    TSITask_Init();
    displaySettingsInit();
    ROTInit();

    OSTaskDel((OS_TCB *)0, &os_err); /* Delete start task */
    assert(os_err == OS_ERR_NONE);
}


/********************************************************************************/

