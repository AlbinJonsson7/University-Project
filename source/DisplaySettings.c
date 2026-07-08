/**************************************************************************************
 *DisplaySettings.c
 *
 *This module controls the output of the program. Has one public function for initialization,
 *one private function for settings up the display, and a task that updates the display
 *every 250ms if an input has been changed.
 *
 *Created on: Mar 12, 2025
 *Author: Trey McCabe
 ***************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "DisplaySettings.h"
#include "UserSettings.h"

/**********************************************************************************
 *Prototypes
 **********************************************************************************/
static void  dsDisplaySettingsTask(void *p_arg);

/**********************************************************************************
* Allocate task control blocks
**********************************************************************************/
static OS_TCB dsDisplaySettingsTaskTCB;

/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK dsDisplaySettingsTaskStk[APP_CFG_DISPLAYSETTINGS_TASK_STK_SIZE];

/*********************************************************************************
 *static void displaySettings(INT32U interfaceHolder, INT32U sineFreqHolder, INT32U sineAmpHolder, INT32U pulseFreqHolder, INT32U pulseDCHolder)
 *Takes multiple different INT32U parameters, and displays those parameters.
 *Depending on the state of the system (sine or pulse) the display will vary
 *********************************************************************************/
static void displaySettings(INT32U interfaceHolder, INT32U sineFreqHolder, INT32U sineAmpHolder, INT32U pulseFreqHolder, INT32U pulseDCHolder){

    //interfaceHolder = 0 means sine, 1 means pulse train
    if(interfaceHolder == 0){
        //for sine
        BIOPutStrg("\r"); //carriage return, to restart at beginning of terminal line
        BIOPutStrg("[sine], "); //displays: [sine],
        BIOOutDecWord(sineFreqHolder, 5, BIO_OD_MODE_AR); //assumming sineFeq = 10,000... displays: [sine], 10000
        BIOPutStrg("Hz, "); //displays [sine], 10000Hz,
        BIOOutDecWord(sineAmpHolder, 2, BIO_OD_MODE_AR); //assumming sineAmp = 10... displays: [sine], 10000Hz, 10
        BIOPutStrg(", pulse, "); //displays: [sine], 10000Hz, 10, pulse,
        BIOOutDecWord(pulseFreqHolder, 5, BIO_OD_MODE_AR);//assumming pulseFreq = 10,000...displays: [sine], 10000Hz, 10, pulse, 10000
        BIOPutStrg("Hz, "); //displays: [sine], 10000Hz, 10, pulse, 10000Hz,
        BIOOutDecWord(pulseDCHolder, 3, BIO_OD_MODE_AR); //assumming pulseDC = 50...displays: [sine], 10000Hz, 10, pulse, 10000Hz, 50
        BIOPutStrg("%"); //displays: [sine], 10000Hz, 10, pulse, 10000Hz, 50%

    }
    else if(interfaceHolder == 1){
        //for pulse
        BIOPutStrg("\r"); //carriage return, to restart at beginning of terminal line
        BIOPutStrg("sine, "); //displays: sine,
        BIOOutDecWord(sineFreqHolder, 5, BIO_OD_MODE_AR); //assumming sineFeq = 10,000... displays: sine, 10000
        BIOPutStrg("Hz, "); //displays sine, 10000Hz,
        BIOOutDecWord(sineAmpHolder, 2, BIO_OD_MODE_AR); //assumming sineAmp = 10... displays: sine, 10000Hz, 10
        BIOPutStrg(", [pulse], "); //displays: sine, 10000Hz, 10, [pulse],
        BIOOutDecWord(pulseFreqHolder, 5, BIO_OD_MODE_AR);//assumming pulseFreq = 10,000...displays: sine, 10000Hz, 10, [pulse], 10000
        BIOPutStrg("Hz, "); //displays: sine, 10000Hz, 10, [pulse], 10000Hz,
        BIOOutDecWord(pulseDCHolder, 3, BIO_OD_MODE_AR); //assumming pulseDC = 50...displays: sine, 10000Hz, 10, [pulse], 10000Hz, 50
        BIOPutStrg("%"); //displays: sine, 10000Hz, 10, [pulse], 10000Hz, 50%

    }else{}

}

/**************************************************************************
 *void displaySettingsInit(void)
 *Initialization function that create the task, and displays the default
 *system settings.
 **************************************************************************/
void displaySettingsInit(void){
    OS_ERR  os_err;

    OSTaskCreate(&dsDisplaySettingsTaskTCB,                  /* Create dsDisplaySettingsTask              */
                    "DisplaySettings Task ",
                    dsDisplaySettingsTask,
                    (void *) 0,
                    APP_CFG_DISPLAYSETTINGS_PRIO,
                    &dsDisplaySettingsTaskStk[0],
                    (APP_CFG_DISPLAYSETTINGS_TASK_STK_SIZE / 10u),
                    APP_CFG_DISPLAYSETTINGS_TASK_STK_SIZE,
                    0,
                    0,
                    (void *) 0,
                    (OS_OPT_TASK_NONE),
                    &os_err);
        assert(os_err == OS_ERR_NONE);

        //interface:sine, sinefreq: 1000, sineAmp: 10, pulseFreq: 1000, pulseDC: 50
        displaySettings(0, 1000, 10, 1000, 50); //displays default state on initialization

}


/*********************************************************************************
 *static void  dsDisplaySettingsTask(void *p_arg)
 *Task that has a 250ms period. It reads the values in the mutex, and if anything has changed
 *it displays the new data.
 *********************************************************************************/
static void  dsDisplaySettingsTask(void *p_arg){
   OS_ERR os_err;
   (void)p_arg;

   //initialized to default values
   INT32U currentInterface = 0; //0 = sine, 1 = pulse
   INT32U currentSineFreq = 1000;
   INT32U currentSineAmp = 10;
   INT32U currentPulseFreq = 1000;
   INT32U currentPulseDC = 50;

   INT32U newInterface; //0 = sine, 1 = pulse
   INT32U newSineFreq;
   INT32U newSineAmp;
   INT32U newPulseFreq;
   INT32U newPulseDC;

    while(1){
        DB1_TURN_OFF();
        //sets task period to 250ms
        OSTimeDly(250, OS_OPT_TIME_PERIODIC, &os_err);
        assert(os_err == OS_ERR_NONE);
        DB1_TURN_ON();
        //assigns current mutex values to newVal variables
        newInterface = getTouchState(); //gets the system state
        newSineFreq = getSineFreq();    //gets sine freq/amp
        newSineAmp = getSineAmp();
        newPulseFreq = getTrigFreq();   //gets pulse train freq/DC
        newPulseDC = getTrigDC();

        //compares current values (old) to new values of mutex to see if theres been any changes
        //if so, displays the new values
        if(currentInterface != newInterface || currentSineFreq != newSineFreq || currentSineAmp != newSineAmp
                || currentPulseFreq != newPulseFreq || currentPulseDC != newPulseDC){

            displaySettings(newInterface, newSineFreq, newSineAmp, newPulseFreq, newPulseDC); //displays new values
            //updates old values to new values
            currentInterface = newInterface; //gets the system state
            currentSineFreq = newSineFreq;    //gets sine freq/amp
            currentSineAmp = newSineAmp;
            currentPulseFreq = newPulseFreq;   //gets pulse train freq/DC
            currentPulseDC = newPulseDC;

        }else{}

    }
}

