/*******************************************************************************
 * StateModule.c
 *
 *This module is used to connect the user inputs to the mutex that holds the system values
 *(state, frequencies, amplitudes, etc.). It uses four tasks, 3 of the tasks pend on an input
 *from the user to update the system settings, while the last task pends on a SW2 press which
 *from resets all values to default values. Everything is private besides stateModuleInit(),
 *which is called in main to initialize the tasks.
 *Created on: Feb 23, 2025
 *Author: Trey McCabe
 ********************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "UserSettings.h"
#include "State.h"
#include "Frequency.h"
#include "ROTModule.h"
#include "TSI.h"

/*******************************************************************************
 * Prototypes
 *******************************************************************************/
static void  stTouchPadTask(void *p_arg);
static void  stRotorTask(void *p_arg);
static void  stFreqTask(void *p_arg);
static void stResetTask(void *p_arg);

/*******************************************************************************
* Allocate task control blocks
**********************************************************************************/
static OS_TCB stTouchPadTaskTCB;
static OS_TCB stRotorTaskTCB;
static OS_TCB stFreqTaskTCB;
static OS_TCB stResetTaskTCB;

/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK stTouchPadTaskStk[APP_CFG_TOUCHPAD_TASK_STK_SIZE];
static CPU_STK stRotorTaskStk[APP_CFG_ROTOR_TASK_STK_SIZE];
static CPU_STK stFreqTaskStk[APP_CFG_FREQ_TASK_STK_SIZE];
static CPU_STK stResetTaskStk[APP_CFG_RESET_TASK_STK_SIZE];

/*************************************************************************
* void stateModuleInit(void)
* Initialization function for StateModule. Calls OSTaskCreate for the three
* tasks that are used in this module.
*************************************************************************/
void stateModuleInit(void){
    OS_ERR  os_err;

    OSTaskCreate(&stTouchPadTaskTCB,                  /* Create stTouchPadTask                    */
                    "TouchPad Task ",
                    stTouchPadTask,
                    (void *) 0,
                    APP_CFG_TOUCHPADTASK_PRIO,
                    &stTouchPadTaskStk[0],
                    (APP_CFG_TOUCHPAD_TASK_STK_SIZE / 10u),
                    APP_CFG_TOUCHPAD_TASK_STK_SIZE,
                    0,
                    0,
                    (void *) 0,
                    (OS_OPT_TASK_NONE),
                    &os_err);
        assert(os_err == OS_ERR_NONE);


        OSTaskCreate(&stRotorTaskTCB,                  /* Create stRotorTask                    */
                        "Rotor Task ",
                        stRotorTask,
                        (void *) 0,
                        APP_CFG_ROTORTASK_PRIO,
                        &stRotorTaskStk[0],
                        (APP_CFG_ROTOR_TASK_STK_SIZE / 10u),
                        APP_CFG_ROTOR_TASK_STK_SIZE,
                        0,
                        0,
                        (void *) 0,
                        (OS_OPT_TASK_NONE),
                        &os_err);
            assert(os_err == OS_ERR_NONE);

       OSTaskCreate(&stFreqTaskTCB,                  /* Create stFreqTask                    */
                        "Freq Task ",
                        stFreqTask,
                        (void *) 0,
                        APP_CFG_FREQTASK_PRIO,
                        &stFreqTaskStk[0],
                        (APP_CFG_FREQ_TASK_STK_SIZE / 10u),
                        APP_CFG_FREQ_TASK_STK_SIZE,
                        0,
                        0,
                        (void *) 0,
                        (OS_OPT_TASK_NONE),
                        &os_err);
            assert(os_err == OS_ERR_NONE);

            OSTaskCreate(&stResetTaskTCB,           /* create stResetTask                   */
                           "State Reset Task ",
                           stResetTask,
                           (void *) 0,
                           APP_CFG_RESET_TASK_PRIO,
                           &stResetTaskStk[0],
                           (APP_CFG_RESET_TASK_STK_SIZE / 10u),
                           APP_CFG_RESET_TASK_STK_SIZE,
                           0,
                           0,
                           (void *) 0,
                           (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                           &os_err);
               assert(os_err == OS_ERR_NONE);


}

/*************************************************************************
* static void stTouchPadTask(void *p_arg)
* Task that pends on TSIState(). Whenever there's a value change for TSIState()
* the tasks uses that value to determine what the new state will be. Either sine
* or pulse state.
*************************************************************************/
static void  stTouchPadTask(void *p_arg){
    OS_ERR os_err;
    (void)p_arg;

    //initialization of variables
    INT32U currentStateTouch = 0;;
    INT32U newStateTouch;

    while(1){

        newStateTouch = TSIState(&os_err);  //sets newStateTouch value whenever TSIState() detects new value
        assert(os_err == OS_ERR_NONE);
        currentStateTouch = currentStateTouch + newStateTouch;  //currentStateTouch is incremented with value of newStateTouch

        //if currentStateTouch is 1
        if(currentStateTouch == 1){
            setTouchState(1);    //set new state to pulse train
        }
        else if(currentStateTouch == 2){
            setTouchState(0);   //sets new state to sine state
            currentStateTouch = 0;  //resets currentStateTouch
        }else{}
    }

}

/*************************************************************************
* static void  stRotorTask(void *p_arg)
* This task pends on ROTState() and whenever ROTState() returns a value,
* it sets the rotor state in the mutex. Checks to see if values are different
* before updating mutex.
*************************************************************************/
static void  stRotorTask(void *p_arg){
    OS_ERR os_err;
    (void)p_arg;

    //initialization of variables
    INT32U currentStateRotor;
    INT32U newStateRotor;

    while(1){
        newStateRotor = ROTState(&os_err);  //sets newStateRotor to ROTState() value
        assert(os_err == OS_ERR_NONE);
        currentStateRotor = getRotorState();   //gets the current rotor value from mutex

        //if the new rotor value, and current rotor values don't match
        if(newStateRotor != currentStateRotor){
            setRotorState(newStateRotor);    //update systemState to new rotor value
        }
        else{}
    }

}


/*************************************************************************
*static void  sFreqTask(void *p_arg)
*Task that pends on FreqState() and updates the frequency values in the mutex
*whenever theres a change in freqeucy input.
*************************************************************************/
static void  stFreqTask(void *p_arg){
    OS_ERR os_err;
   (void)p_arg;

   //variable initilization
    INT32U currentStateFreq;
    INT32U newStateFreq;

    while(1){
        newStateFreq = FreqState(&os_err);  //sets newStateFreq to FreqState() value
        assert(os_err == OS_ERR_NONE);
        currentStateFreq = getFreqState();   //gets the current state. 0 is sine, 1 is pulse

        //if the touchpad input doesn't match system state
        if(newStateFreq != currentStateFreq){
            setFreqState(newStateFreq);    //update systemState to new Freq value
        }
        else{}
    }

}
/**************************************************************************
 * static void stResetTask(void *p_arg)
 * stResetTask pends on swPend.
 * if switch 2 is pressed reset the system back to default
 * else do nothing
 * Author: Josh Benner
 **************************************************************************/
static void stResetTask(void *p_arg){
    OS_ERR os_err;
    SW_T switchState=0;
    (void)p_arg;

    while(1){
       switchState= SwPend(0, &os_err); //pends on a SW2 press
       //if SW2 is pressed
       if(switchState == SW2){
           systemStateReset();  //resets values to default
           setButtonPress(1);   //sets buttonCounter value for ROT reset purposes
       }else{
           continue;
       }

    }
}
