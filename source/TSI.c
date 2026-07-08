/*****************************************************************************************
* TSI.c - will initalize the touchpad on the MCXN947FRDM board and read a button press. The
*         button press will be indicated as a 1 when pressed, and 0 when not pressed. When
*         pressed the value of 1 will be returned to the State.c fileusing the TSIState
*         function.
*
* 03/14/2025, David Friday
*****************************************************************************************/

#include "MCUType.h"
#include "State.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "TSI.h"
#include "app_cfg.h"


/********************************************************************
* Private Resources
********************************************************************/
#define TOUCH_OFFSET  0x6000U    // Touch offset from baseline


/***********************************************
 * Prototypes
 ************************************************/
static void tsiTask(void *p_arg);
static void tsiChCalibration(void);
static void TSIsemInit(void);
static void TSI_Init(void);


/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB tsiTaskTCB;


/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK tsiTaskStk[APP_CFG_TASK_TSI_STK_SIZE];


/*************************************************************************
* Enumerated type for Frequency buffer
*************************************************************************/
typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
}TOUCH_LEVEL_T;
static TOUCH_LEVEL_T tsiLevels;


/*************************************************************************
* Enumerated type for TSI buffer
*************************************************************************/
typedef struct{
    INT8U press;
    OS_SEM flag;
}TSIStruct;
static TSIStruct TSIBuffer;


/***********************************************************************
* TSIsemInit() - intializing semaphore
***********************************************************************/
static void TSIsemInit(void){
    OS_ERR os_err;
    //Creates a flag semaphore
    OSSemCreate(&TSIBuffer.flag,"Flag Semaphore", 0, &os_err);
}


/************************************************************************
* FreqScan_Init() - Initialization routine for the FreqScanTask
************************************************************************/
void TSITask_Init(void){

    OS_ERR os_err;
    TSIsemInit();
    TSI_Init();

    OSTaskCreate(&tsiTaskTCB,
                "TSI Task",
                tsiTask,
                (void *) 0,
                APP_CFG_TASK_TSI_PRIO,
                &tsiTaskStk[0],
                (APP_CFG_TASK_TSI_STK_SIZE / 10u),
                APP_CFG_TASK_TSI_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

}


/*****************************************************************************************
 * TSI_Init: Initializes TSI0 module
 *           Author: Todd Morton, GIT lab
 *           Modified: David Friday 3/14/2025
 *****************************************************************************************/
static void TSI_Init(void){

    /* set constant offset count */
    tsiLevels.offset = TOUCH_OFFSET;

    SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_TSI(1);
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1);

    /* Select clk_in as clock, 24MHz*/
    SYSCON0->TSICLKSEL = SYSCON_TSICLKSEL_SEL(2);

    /* Configure TSI for the touch pad on the FRDM board */
    TSI0->CONFIG = TSI_CONFIG_S_XCH(0)|TSI_CONFIG_TSICH(3)|TSI_CONFIG_S_CTRIM(0)|TSI_CONFIG_S_SEN(1)|
            TSI_CONFIG_S_XDN(1);
    TSI0->SINC = TSI_SINC_ORDER(1)|TSI_SINC_DECIMATION(28); /* ORDER = 2, DEC = 29 */
    TSI0->SSC0 = TSI_SSC0_PRBS_OUTSEL(2)|TSI_SSC0_CHARGE_NUM(4)|TSI_SSC0_PRBS_OUTSEL(2)|
            TSI_SSC0_BASE_NOCHARGE_NUM(2)|TSI_SSC0_SSC_PRESCALE_NUM(1);

    TSI0->GENCS |= TSI_GENCS_SETCLK(0)|TSI_GENCS_DVOLT(3);
    /* Enable TSI and calibrate */
    TSI0->GENCS |= TSI_GENCS_TSIEN(1);
    tsiChCalibration();
}


/********************************************************************************
*   tsiCalibration: Calibration to find non-touch baseline
*                   Note - the sensor must not be pressed when this is executed.
*                   Author: Todd Morton, GIT lab
*                   Modified: David Friday 3/14/2025
********************************************************************************/
static void tsiChCalibration(void){
    TSI0->GENCS |= TSI_GENCS_SWTS(1);             //start a scan sequence
    while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){} //wait for scan to finish
    TSI0->DATA |= TSI_DATA_EOSF(1);    //Clear flag
    tsiLevels.baseline = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    tsiLevels.threshold = tsiLevels.baseline + tsiLevels.offset;
}


/********************************************************************************
*   tsiTask: Cooperative task for timeslice scheduler
*            Blocks for ~6ms
*            In order to not block the task period should be > 6ms.
*            To not miss a press, the task period should be < ~25ms.
*            Author: Todd Morton, GIT lab
*            Modified: David Friday 3/14/2025
********************************************************************************/
static void tsiTask(void *p_arg){

    OS_ERR os_err;
    (void)p_arg;

    while(1){
    //start a scan sequence
        OSTimeDly(100, OS_OPT_TIME_PERIODIC, &os_err);
        assert(os_err == OS_ERR_NONE);

       TSI0->GENCS |= TSI_GENCS_SWTS(1);
    /* wait for scan to finish */
        while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){}
        TSI0->DATA |= TSI_DATA_EOSF(1);    //Clear flag

        // Process channel
        if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > tsiLevels.threshold){
        TSIBuffer.press = 1; //Updates value in TSIbuffer
        OSSemPost(&TSIBuffer.flag,OS_OPT_POST_1,&os_err); //Post semaphore flag

        }else{}

    }
}


/*******************************************************************************************
* TSIState() - Gets called from State.c and returns a 1 if button is pressed
*******************************************************************************************/
INT32U TSIState(OS_ERR *os_err_ptr){
    INT8U press = 0;
    //Check if semaphore is pending, blocking until detects a signal -> moved to waiting state
    OSSemPend(&TSIBuffer.flag,0,OS_OPT_PEND_BLOCKING,(CPU_TS *)0,os_err_ptr);
    press = TSIBuffer.press; //Assigns value of the TSIbuffer to local variable
    TSIBuffer.press = 0; //Resets value of  TSIbuffer
    return press; //Returns the frequency value
}

