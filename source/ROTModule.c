/****************************************************************************************************************
 * ROTModule.c
 *
 *This module gets a reading from the ROT to then be sent to State.c to be used else where in the program.
 *Has two public functions, one for initialization and the other for sending the ROT levels. Also has a task
 *that reads the rotor levels.
 *
 *Created on: Mar 13, 2025
 *Authors: Todd Morton, Trey McCabe
 ****************************************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "UserSettings.h"
#include "ROTModule.h"

/*******************************************************************************
 *Prototypes
 *******************************************************************************/
static void  rmROTTask(void *p_arg);

/*******************************************************************************
* Allocate task control blocks
**********************************************************************************/
static OS_TCB rmROTTaskTCB;

/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK rmROTTaskTaskStk[APP_CFG_ROTOR_TASK_STK_SIZE];

/******************************************************************************************
* Project Defined Constants and types
******************************************************************************************/
#define SLICE_PER 50            /* 10ms time slice period                                */
#define EDGE_DIV 3              /* number of edges for one increment or decrement        */
#define pCNT_MIN 0              /* Minimum count value for pulse                         */
#define pCNT_MAX 100            /* Maximum count value for pulse                         */
#define sCNT_MIN 0              /* Minimum count value for sine                          */
#define sCNT_MAX 20             /* Maximum count value for sine                          */

/******************************************************************************************
* Private Global Variables
******************************************************************************************/
static INT16S qeCnt;
static INT16S qeXCntp = (pCNT_MAX*EDGE_DIV)/2;   //for pulse
static INT16S qeXCnts = (sCNT_MAX*EDGE_DIV)/2;   //for sine

/********************************************************************************************
 *Struct for the counter and flag
 *******************************************************************************************/
typedef struct{
    INT32U level;
    OS_SEM flag;

}ROTBuffer;

/**********************************************************************************************
 *struct instance, private
 **********************************************************************************************/
static ROTBuffer rotBuffer;

/**********************************************************************************************
* ROTInit() - Initialize QDC0 for simple rotational encoders.
*               For input on J3_1 (PHA) and J3_3 (PHB) on FRDM-N947 board.
*               Note the old acronym was ENC and the new acronym is QDC. The RM uses both.
*               Code below uses the old acronym
*
*Modifications: Creates a semaphore and a task
*Authors: Todd Morton (code from gitlab), modified by Trey McCabe
**********************************************************************************************/
void ROTInit(void){
    OS_ERR os_err;
    /* Enable clock for QDC0, PORT1, and PORT2 */
        SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_ENC0(1);
        SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1);
        SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT2(1);
        /* Configure Inputs - internal pull-ups and input buffer enabled
         *                    P1_22 to TRIG_IN3 and P2_0  to TRIG_IN5
         */
        PORT1->PCR[22] = PORT_PCR_MUX(1)|PORT_PCR_IBE(1)|PORT_PCR_PS(1)|PORT_PCR_PE(1);
        PORT2->PCR[0] = PORT_PCR_MUX(1)|PORT_PCR_IBE(1)|PORT_PCR_PS(1)|PORT_PCR_PE(1);
        /* Use INPUTMUX to tie TRIG_IN3 to PhaseA and TRIG_IN5 to PhaseB */
        SYSCON0->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_MUX(1);
        INPUTMUX0->ENCN[0].ENC_PHASEB = INPUTMUX_ENCN_ENC_PHASEB_INP(0x2f);
        INPUTMUX0->ENCN[0].ENC_PHASEA = INPUTMUX_ENCN_ENC_PHASEA_INP(0x2d);
        SYSCON0->AHBCLKCTRLCLR[0] = SYSCON_AHBCLKCTRL0_MUX(1);
        /* Configure input filter */
        ENC0->FILT = ENC_FILT_FILT_PRSC(7)|ENC_FILT_FILT_CNT(0)|ENC_FILT_FILT_PER(0xff);

        rotBuffer.level = 0;    //initializes ROT level to 0

        OSSemCreate(&rotBuffer.flag, "Flag Semaphore", 0, &os_err);       //creates a semaphore

        OSTaskCreate(&rmROTTaskTCB,                  /* Create rmROTPadTask                    */
                            "ROT Task ",
                            rmROTTask,
                            (void *) 0,
                            APP_CFG_ROTORTASK_PRIO,
                            &rmROTTaskTaskStk[0],
                            (APP_CFG_ROTOR_TASK_STK_SIZE / 10u),
                            APP_CFG_ROTOR_TASK_STK_SIZE,
                            0,
                            0,
                            (void *) 0,
                            (OS_OPT_TASK_NONE),
                            &os_err);
                assert(os_err == OS_ERR_NONE);

}

/*****************************************************************************************************
* qeCntOutTask() -Task that uses the position difference register to add or
*                subtract counts. Uses EDGE_DIV to make it more or less sensitive.
*                Output is 0-20.
*
*Modifications: Uses an if statement that changes the max ROT levels based on the state of the system.
*Also has an if statement that detects if the SW2 was pressed, and resets values for system reset purposes.
*Incorporates a sempahore to signal ROT level change.
*Authors: Todd Morton (code from gitlab), modified by Trey McCabe
******************************************************************************************************/
static void  rmROTTask(void *p_arg){
    OS_ERR os_err;
    (void)p_arg;

    INT32U state;
    INT32U buttonTwo;

    while(1){

        OSTimeDly(10, OS_OPT_TIME_PERIODIC, &os_err);   //10ms delay
        assert(os_err == OS_ERR_NONE);

        state = getTouchState();   //gets current state of the system

        //state = 0 for sine, 1 for pulse
        if(state == 0){

            qeXCnts = qeXCnts + (INT16S)(ENC0->POSD);
            /* control max and min values */
            if(qeXCnts > sCNT_MAX*EDGE_DIV){
                qeXCnts = sCNT_MAX*EDGE_DIV;
            }else if(qeXCnts < sCNT_MIN){
                qeXCnts = sCNT_MIN;
            }

            /* Output final count */
            qeCnt = qeXCnts/EDGE_DIV;

            buttonTwo = getButtonPress();    //gets value if buttonPressed

            //f button press detected
            if(buttonTwo == 1){

                //resets values
                qeXCnts = (sCNT_MAX*EDGE_DIV)/2;   //for sine
                qeXCntp = (pCNT_MAX*EDGE_DIV)/2;    //for pulse
                qeCnt=0;
                setButtonPress(0);  //sets buttonPress value back to 0

             }else{ }
            rotBuffer.level = (INT32U) qeCnt;
            OSSemPost(&rotBuffer.flag, OS_OPT_POST_1, &os_err);   //signals semaphore

        }
        else if(state == 1){

            /* get difference count and add */
            qeXCntp = qeXCntp + (INT16S)(ENC0->POSD);
            /* control max and min values */
            if(qeXCntp > pCNT_MAX*EDGE_DIV){
                qeXCntp = pCNT_MAX*EDGE_DIV;
            }else if(qeXCntp < pCNT_MIN){
                qeXCntp = pCNT_MIN;
            }

            /* Output final count */
            qeCnt = qeXCntp/EDGE_DIV;

            buttonTwo = getButtonPress();    //gets value if buttonPressed

            //f button press detected
            if(buttonTwo== 1){

                //resets values
                qeXCntp = (pCNT_MAX*EDGE_DIV)/2;    //for sine
                qeXCnts = (sCNT_MAX*EDGE_DIV)/2;    //for pulse
                qeCnt=0;
                setButtonPress(0);  //sets buttonPress value back to zero

            }else{ }

            rotBuffer.level = (INT32U) qeCnt;
            OSSemPost(&rotBuffer.flag, OS_OPT_POST_1, &os_err);   //signals semaphore

        } else{}


    }
}

/*******************************************************************************************
 *INT32U ROTState(OS_ERR *os_err_ptr)
 *Returns the level of the rotor whenever there is a change in value
 ********************************************************************************************/
INT32U ROTState(OS_ERR *os_err_ptr){

    OSSemPend(&rotBuffer.flag, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err_ptr); //checks to see if semaphore is pending
                                                                                  //if semaphore is not pending, moves task into waiting state
    return rotBuffer.level; //returns rotor level
}



