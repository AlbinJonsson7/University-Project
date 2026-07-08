/******************************************************************************************************
 * UserSettings.c
 *
 *Sets up a mutex and a struct that holds the system settings. Also creates public functions that access
 *the mutex to either get the values in the mutex, or to set the values in the mutex. Also has functions
 *for when the button is pressed for reset functionality of the ROT.
 *
 *Created on: Feb 23, 2025
 *Author: Trey McCabe
 ***************************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "UserSettings.h"

/*****************************************************************************************************
 *Mutex Key
 ****************************************************************************************************/
static OS_MUTEX sysStateKey;

/*****************************************************************************************************
 *SystemState struct for system settings
 ****************************************************************************************************/
typedef struct{

    INT32U interface; //0 = sine, 1 = pulse
    INT32U sineFreq;
    INT32U sineAmp;
    INT32U pulseFreq;
    INT32U pulseDC;
    INT32U buttonTwoPress;


}SystemState;

/*****************************************************************************************************
 *Struct instance, private
 ****************************************************************************************************/
static SystemState systemState;

/*****************************************************************************************************
 *void mutexInit(void)
 *Mutex creation and SystemState data initialization
 *****************************************************************************************************/
void mutexInit(void) {
    OS_ERR err;

    OSMutexCreate(&sysStateKey, "System State Mutex", &err);

    systemState.interface = 0;
    systemState.sineFreq = 1000;
    systemState.sineAmp = 10;
    systemState.pulseFreq = 1000;
    systemState.pulseDC = 50;
    systemState.buttonTwoPress =0;

}

/********************************************************************************
 *INT32U getTouchState(void)
 *Gets the state value from the SystemState struct and returns the value
 ********************************************************************************/
INT32U getTouchState(void){
    OS_ERR os_err;
    INT32U interfaceHolder;

    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key

    interfaceHolder = systemState.interface;                                   //gets interface value

    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    return interfaceHolder;                                                    //returns current value of interface

}

/********************************************************************************
 *void setTouchState(INT32U state)
 *Takes a state value as an input, and sets the interface value to that state value
 ********************************************************************************/
void setTouchState(INT32U state){
    OS_ERR os_err;

    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key

    systemState.interface = state;                                             //sets new interface value

    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key
}

/*****************************************************************************************************
 *INT32U getRotorState(void)
 *Depending on if in sine or pulse mode, returns the value of either the sineAmp or pulseDC for rotor
 *****************************************************************************************************/
INT32U getRotorState(void){
    OS_ERR os_err;
    INT32U sineAmpHolder;
    INT32U pulseDCHolder;

    //returns either sineAmp or pulseDC depending on interface state. 0 for sine, 1 for pulse
    if(systemState.interface == 0){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        sineAmpHolder = systemState.sineAmp;                                       //gets sine amp value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

        return sineAmpHolder;                                                      //returns sine amp value

    }
    else if (systemState.interface == 1){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        pulseDCHolder = systemState.pulseDC;                                       //gets pulse duty cycle value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

        return pulseDCHolder;                                                      //returns pulse DC value
    }
    else{
        return 2;                                                                  //return value of 2 is an error
    }

}

/*****************************************************************************************************
 *void setRotorState(INT32U state)
 *Depending on if in sine or pulse mode, sets the value of either sineAmp or pulseDC to the state value
 *****************************************************************************************************/
void setRotorState(INT32U state){
    OS_ERR os_err;

    //depending on if in sine state (0) or pulse state (1)
    if(systemState.interface == 0){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        systemState.sineAmp = state;                                               //sets sineAmp to state value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key                                                      //returns sine amp value

    }
    else if (systemState.interface == 1){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        systemState.pulseDC = state;                                               //sets pulseDC to state value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    }
    else{}

}

/*****************************************************************************************************
 *INT32U getFreqState(void)
 *Depending on if in sine or pulse mode, returns either sineFreq value or pulseFreq value
 *****************************************************************************************************/
INT32U getFreqState(void){
    OS_ERR os_err;
    INT32U sineFreqHolder;
    INT32U pulseFreqHolder;

    //returns either sineFreq or pulseFreq depending on interface state. 0 for sine, 1 for pulse
    if(systemState.interface == 0){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        sineFreqHolder = systemState.sineFreq;                                     //gets sine freq value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

        return sineFreqHolder;                                                     //returns sinefreq value

    }
    else if (systemState.interface == 1){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        pulseFreqHolder = systemState.pulseFreq;                                   //gets pulse freq value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

        return pulseFreqHolder;                                                    //returns pulse freq value
    }
    else{
        return 2;                                                                  //return value of 2 is an error
    }

}

/*****************************************************************************************************
 *void setFreqState(INT32U state)
 *Depending on if in sine or pulse mode, sets either sineFreq or pulseFreq to the state value
 *****************************************************************************************************/
void setFreqState(INT32U state){
    OS_ERR os_err;

    //depending on if in sine state (0) or pulse state (1)
    if(systemState.interface == 0){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        systemState.sineFreq = state;                                              //sets sineFreq to state value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key                                                      //returns sine amp value

    }
    else if (systemState.interface == 1){

        OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
        systemState.pulseFreq = state;                                             //sets pulseFreq to state value
        OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    }
    else{}

}

/**************************************************************************************
 *void systemStateReset(void)
 *Resets systemState to default values
 ***************************************************************************************/
void systemStateReset(void){
    OS_ERR os_err;

    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
    systemState.interface = 0;
    systemState.sineFreq = 1000;
    systemState.sineAmp = 10;
    systemState.pulseFreq = 1000;
    systemState.pulseDC = 50;
    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key
}
/**************************************************************************************
 * INT32U getTrigFreq(void)
 * Returns the value of the pulse train freq
 ***************************************************************************************/
INT32U getTrigFreq(void){
    OS_ERR os_err;

    INT32U trigFreqHolder;
    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
    trigFreqHolder = systemState.pulseFreq;                                    //gets pulseFreq value
    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    return trigFreqHolder;
}
/**************************************************************************************
 *INT32U getTrigDC(void)
 *Returns the value of the pulse train duty cycle
 ***************************************************************************************/
INT32U getTrigDC(void){
    OS_ERR os_err;

    INT32U trigDCHolder;
    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
    trigDCHolder = systemState.pulseDC;                                        //gets pulseDC value
    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    return trigDCHolder;
}
/**************************************************************************************
 * INT32U getSineFreq(void)
 * Returns the value of the sine freq
 ***************************************************************************************/
INT32U getSineFreq(void){
    OS_ERR os_err;

    INT32U sineFreqHolder;
    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
    sineFreqHolder = systemState.sineFreq;                                     //gets sineFreq value
    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    return sineFreqHolder;
}
/**************************************************************************************
 * INT32U getSineAmp(void)
 * Returns the value of the sine amp
 ***************************************************************************************/
INT32U getSineAmp(void){
    OS_ERR os_err;

    INT32U sineAmpHolder;
    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
    sineAmpHolder = systemState.sineAmp;                                       //gets sine amp value
    OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

    return sineAmpHolder;
}

/**************************************************************************************
 * INT32U getButtonPress(void)
 * Returns the value of buttonTwoPress. (Used for ROT reset purposes)
 ***************************************************************************************/
INT32U getButtonPress(void){
    OS_ERR os_err;

      INT32U buttonPressHolder;
      OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
     buttonPressHolder= systemState.buttonTwoPress;                              //gets buttonTwoPress value
      OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                      //release a mutex key

      return buttonPressHolder;
}

/**************************************************************************************
 *void setButtonPress(INT32U buttonSet)
 *Takes buttonSet as an input and sets buttonTwoPress to that value
 ***************************************************************************************/
void setButtonPress(INT32U buttonSet){
    OS_ERR os_err;
    OSMutexPend(&sysStateKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);  //wait for mutex key
       systemState.buttonTwoPress = buttonSet;                                 //sets buttonTwoPress to buttonSet value
       OSMutexPost(&sysStateKey, OS_OPT_POST_NONE, &os_err);                   //release a mutex key
}



