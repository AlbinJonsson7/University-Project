/*****************************************************************************************
* Frequency.c - will prompt the user to enter sine or pulse frequency in the terminal
*               if the enter key is pressed. When a frequency is entered in the terminal
*               to submit that frequency, the enter key needs to be pressed again.
*
* 03/13/2025, Albin Jonsson
*****************************************************************************************/

#include "MCUType.h"
#include "State.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "Frequency.h"
#include "UserSettings.h"

/********************************************************************
* Private Resources
********************************************************************/
static void fsFreqScanTask(void *p_arg);
static void currentState(void);
static void clearBuffer(INT8U index, INT8C *const strg);
static void semInit(void);
static INT32U strToInt(INT8C *str);


/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB fsFreqScanTaskTCB;


/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK fsFreqScanTaskStk[APP_CFG_FREQSCANTASK_STK_SIZE];


/*************************************************************************
* Enumerated type for Frequency buffer
*************************************************************************/
typedef struct{
    INT32U frequency;
    OS_SEM flag;
} FrequencyBuffer;
static FrequencyBuffer freqBuffer;    //Instance of FrequencyBuffer


/***********************************************************************
* semInit() - intializing semaphores and frequency
***********************************************************************/
static void semInit(void){
    OS_ERR os_err;

    OSSemCreate(&freqBuffer.flag,"Flag Semaphore", 0, &os_err);    //Creates a flag semaphore
}


/************************************************************************
* FreqScan_Init() - Initialization routine for the FreqScanTask
************************************************************************/
void FreqScan_Init(void){

    OS_ERR os_err;
    semInit();


    OSTaskCreate(&fsFreqScanTaskTCB,
                "FreqScanTask ",
                fsFreqScanTask,
                (void *) 0,
                APP_CFG_FREQSCANTASK_PRIO,
                &fsFreqScanTaskStk[0],
                (APP_CFG_FREQSCANTASK_STK_SIZE / 10u),
                APP_CFG_FREQSCANTASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

}


/********************************************************************************************
* TASK - FreqScanTask
* This task waits for the enter key to be pressed then promtps the user to enter a frequency
* Then for every other key pressed that character will be added to a buffer. When another
* enter key is detected the string of characters in the buffer will be set to a an integer
* number. The integer value will be stored to the frequency buffer, then post a semaphore flag
*********************************************************************************************/
static void fsFreqScanTask(void *p_arg){

    OS_ERR os_err;
    (void)p_arg;

    //Initialize local variables
    INT32U freqValue = 0;
    INT8U bufferIndex = 0;
    INT8C readInput = 0;
    INT8C buffer[10];

    while(1){

        //Time delay to not overload CPU
        OSTimeDly(50, OS_OPT_TIME_PERIODIC, &os_err);
        assert(os_err == OS_ERR_NONE);


        readInput = BIORead(); //Reads character entered in the terminal
        if(readInput == '\r'){ //If enter key is pressed enter this if statement
            if(buffer[0] == '\0'){ //If buffer is empty call state function
                currentState();
            }else{ //If buffer is not empty get number from buffer
                freqValue = strToInt(buffer); //Converts characters in buffer to integer value
                clearBuffer(bufferIndex,buffer);//clear buffer
                bufferIndex = 0; //Reset index
                if((freqValue <= 10000) && (freqValue >= 10)){ //If frequency value is in the range
                    freqBuffer.frequency = freqValue; //Set the frequency buffer value to this frequency
                }else if(freqValue > 10000){
                    freqBuffer.frequency = 10000; //If freqValue > 10000, set to max at 10000Hz
                }else if(freqValue < 10){
                    freqBuffer.frequency = 10;//If freqValue < 10, set to min at 10Hz
                }else{}

                OSSemPost(&freqBuffer.flag,OS_OPT_POST_1,&os_err); //Semaphore sends a signal to signal input is done
                BIOPutStrg("\n"); //New line
            }
        }else if(readInput != '\0'){ //If enter key is not pressed
            buffer[bufferIndex] = readInput; //Fill buffer with input character
            BIOWrite(buffer[bufferIndex]);//Print character
            bufferIndex++;

        }else{}
    }

}


/*****************************************************************************************
* currentState() - Prompts for new frequncy for the current state
*****************************************************************************************/
static void currentState(void){

    INT32U state = 0;

    state = getTouchState(); //Gets the current state
    if(state == 0){
        BIOPutStrg("\n\r Sine f: "); //Prompts this if in sine state
    }else if(state == 1){
        BIOPutStrg("\n\r Pulse f: "); //Prompts this if in oulse state
    }else{}
}

/*****************************************************************************************
* clearBuffer() - Empties all characters entered in the buffer
*****************************************************************************************/
static void clearBuffer(INT8U index, INT8C *const strg){
    //Empites the buffer of each index used
    for(INT8U i = 0 ; i <= index ; i++){
        strg[i] = '\0';
    }
}


/*****************************************************************************************
* strToInt() - Takes an input string and converts characters of numbers to integer value
*****************************************************************************************/
static INT32U strToInt(INT8C *str){
    INT32U result = 0;

    while(*str != '\0'){           // Iterate through the string/buffer
            if((*str >= '0') && (*str <= '9')){   // Check if character is a digit
                INT32U digit = (INT32U)(*str - '0'); //Make digit to integer
                result = (result * 10) + digit;  // Convert and accumulate
            }else{
                break;  // Stop conversion
            }
            str++;     // Move to the next character
        }

    return result;
}


/*******************************************************************************************
* FreqState() - A function that will return the input frequency entered in the terminal
*******************************************************************************************/
INT32U FreqState(OS_ERR *os_err_ptr){
    //Check if semaphore is pending, blocking until detects a signal -> moved to waiting state
    OSSemPend(&freqBuffer.flag,0,OS_OPT_PEND_BLOCKING,(CPU_TS *)0,os_err_ptr);
    return freqBuffer.frequency; //Returns the frequency value
}
