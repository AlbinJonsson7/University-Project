/*****************************************************************************************
* PulseTrain.c - is a module that controls the pulse train. It initializes CTimer0 to
* generate a pulse train. Then by calling the UserSettings module we get updated values
* for the pulse train.
*
* 02/27/2025, Albin Jonsson
*****************************************************************************************/

#ifndef PULSETRAIN_H_
#define PULSETRAIN_H_


/**************************************
 * Public Initialization  Prototypes
 ***************************************/
void PT_Init(void);
void CTimer0_Init(void);




#endif /* PULSETRAIN_H_ */
