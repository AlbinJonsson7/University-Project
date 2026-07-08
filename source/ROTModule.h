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

#ifndef ROTMODULE_H
#define ROTMODULE_H

/*******************************************************************************************************************
 *Public Function Prototypes
 *******************************************************************************************************************/

/**********************************************************************************************
* ROTInit() - Initialize QDC0 for simple rotational encoders.
*               For input on J3_1 (PHA) and J3_3 (PHB) on FRDM-N947 board.
*               Note the old acronym was ENC and the new acronym is QDC. The RM uses both.
*               Code below uses the old acronym
*
*Modifications: Creates a semaphore and a task
*Authors: Todd Morton (code from gitlab), modified by Trey McCabe
**********************************************************************************************/
void ROTInit(void);

/*******************************************************************************************
 *INT32U ROTState(OS_ERR *os_err_ptr)
 *Returns the level of the rotor whenever there is a change in value
 ********************************************************************************************/
INT32U ROTState(OS_ERR *os_err_ptr);

#endif /* ROTMODULE_H*/
