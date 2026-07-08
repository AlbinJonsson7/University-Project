/*****************************************************************************************
* TSI.c - will initalize the touchpad on the MCXN947FRDM board and read a button press. The
*         button press will be indicated as a 1 when pressed, and 0 when not pressed. When
*         pressed the value of 1 will be returned to the State.c fileusing the TSIState
*         function.
*
* 03/14/2025, David Friday
*****************************************************************************************/

#ifndef TSI_H_
#define TSI_H_


/********************************************************************
* Function Prototypes
********************************************************************/
INT32U TSIState(OS_ERR *os_err_ptr);
void TSITask_Init(void);


#endif /* TSI_H_ */
