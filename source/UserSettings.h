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

#ifndef USERSETTINGS_H
#define USERSETTINGS_H

/****************************************************************************************************
 *Public Functino Prototypes
 ****************************************************************************************************/

/*****************************************************************************************************
 *void mutexInit(void)
 *Mutex creation and SystemState data initialization
 *****************************************************************************************************/
void mutexInit(void);

/**************************************************************************************
 * INT32U getTrigFreq(void)
 * Returns the value of the pulse train freq
 ***************************************************************************************/
INT32U getTrigFreq(void);

/**************************************************************************************
 *INT32U getTrigDC(void)
 *Returns the value of the pulse train duty cycle
 ***************************************************************************************/
INT32U getTrigDC(void);

/**************************************************************************************
 * INT32U getSineFreq(void)
 * Returns the value of the sine freq
 ***************************************************************************************/
INT32U getSineFreq(void);

/**************************************************************************************
 * INT32U getSineAmp(void)
 * Returns the value of the sine amp
 ***************************************************************************************/
INT32U getSineAmp(void);

/********************************************************************************
 *INT32U getTouchState(void)
 *Gets the state value from the SystemState struct and returns the value
 ********************************************************************************/
INT32U getTouchState(void);

/********************************************************************************
 *void setTouchState(INT32U state)
 *Takes a state value as an input, and sets the interface value to that state value
 ********************************************************************************/
void setTouchState(INT32U);

/*****************************************************************************************************
 *INT32U getRotorState(void)
 *Depending on if in sine or pulse mode, returns the value of either the sineAmp or pulseDC for rotor
 *****************************************************************************************************/
INT32U getRotorState(void);

/*****************************************************************************************************
 *void setRotorState(INT32U state)
 *Depending on if in sine or pulse mode, sets the value of either sineAmp or pulseDC to the state value
 *****************************************************************************************************/
void setRotorState(INT32U);

/*****************************************************************************************************
 *INT32U getFreqState(void)
 *Depending on if in sine or pulse mode, returns either sineFreq value or pulseFreq value
 *****************************************************************************************************/
INT32U getFreqState(void);

/*****************************************************************************************************
 *void setFreqState(INT32U state)
 *Depending on if in sine or pulse mode, sets either sineFreq or pulseFreq to the state value
 *****************************************************************************************************/
void setFreqState(INT32U);

/**************************************************************************************
 *void systemStateReset(void)
 *Resets systemState to default values
 ***************************************************************************************/
void systemStateReset(void);

/**************************************************************************************
 * INT32U getButtonPress(void)
 * Returns the value of buttonTwoPress. (Used for ROT reset purposes)
 ***************************************************************************************/
INT32U getButtonPress(void);

/**************************************************************************************
 *void setButtonPress(INT32U buttonSet)
 *Takes buttonSet as an input and sets buttonTwoPress to that value
 ***************************************************************************************/
void setButtonPress(INT32U buttonSet);


#endif /* USERSETTINGS_H */
