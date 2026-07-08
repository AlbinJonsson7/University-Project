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

#ifndef STATE_H
#define STATE_H

/****************************************************************************************************
 *Public Functino Prototypes
 ****************************************************************************************************/

/*************************************************************************
* void stateModuleInit(void)
* Initialization function for StateModule. Calls OSTaskCreate for the three
* tasks that are used in this module.
*************************************************************************/
void stateModuleInit(void);

#endif /* STATEMODULE_H */
