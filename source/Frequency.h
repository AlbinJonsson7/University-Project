/*****************************************************************************************
* Frequency.c - will prompt the user to enter sine or pulse frequency in the terminal
*               if the enter key is pressed. When a frequency is entered in the terminal
*               to submit that frequency, the enter key needs to be pressed again.
*
* 03/13/2025, Albin Jonsson
*****************************************************************************************/

#ifndef FREQUENCY_H_
#define FREQUENCY_H_

/*****************************************************************
 * Public Function Prototypes
 ******************************************************************/
INT32U FreqState(OS_ERR *os_err_ptr);
void FreqScan_Init(void);


#endif /* FREQUENCY_H_ */
