/*****************************************************************************************
* PulseTrain.c - is a module that controls the pulse train. It initializes CTimer0 to
* generate a pulse train. Then by calling the UserSettings module we get updated values
* for the pulse train.
*
* 02/27/2025, Albin Jonsson
*****************************************************************************************/

#include "MCUType.h"
#include "PulseTrain.h"
#include "UserSettings.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"


/********************************************************************
* Private Resources
********************************************************************/
static void ptPulseTrainTask(void *p_arg);


/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB ptPulseTrainTaskTCB;


/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK ptPulseTrainTaskStk[APP_CFG_PULSETRAINTASK_STK_SIZE];


/*******************************************************************************************
* CTimer0_Init() - Initializes Ctimer0 for the pulse train
* Author Todd Morton, GITLab
* Modified by Albin Jonsson 02/27/2025
*******************************************************************************************/
void CTimer0_Init(void){

    SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_TIMER0(1); //Enable clock for CTimer0
    SYSCON0->CTIMERCLKSEL[0] = SYSCON_CTIMERCLKSEL_SEL(1); // Select pll0_clk as CTIMER0's clock, 150MHz

    SYSCON0->CTIMERCLKDIV[0] = 0; // Setup CTIMER0's clock divider and wait for it to stabilize
    while (SYSCON0->CTIMERCLKDIV[0] & SYSCON_CTIMERXCLKDIV_CTIMERCLKDIV_UNSTAB(1)){}

    SYSCON0->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1);// Enable clock gate for PORT0
    PORT0->PCR[11] = PORT_PCR_MUX(4); // Configure P0_11 to ALT4, CT0_MAT1

    // Timer setup
    CTIMER0->TCR = CTIMER_TCR_CEN(0); // Disable timer
    CTIMER0->MCR = CTIMER_MCR_MR0R(1);// Set CTIMER0 to reset counter on match 0
    CTIMER0->PR = CTIMER_PR_PRVAL(0); // Set PR to 0 (dividing clock by 1)
    CTIMER0->MR[0] = CTIMER_MR_MATCH(150000 - 1);   //Set Pulse Train to frequency of 1kHz (period of 1ms)
    CTIMER0->MR[1] = CTIMER_MR_MATCH(75000);    //Set duty cycle to 50%, 75000 is half of 150000
    CTIMER0->PWMC |= CTIMER_PWMC_PWMEN1(1); // Enable PWM output on CTIMER0_MAT1
    CTIMER0->TCR |= CTIMER_TCR_CEN(1);  // Enable CTIMER0

}


/********************************************************************
* PT_Init() - Initialization routine for the Pulste Train task
********************************************************************/
void PT_Init(void){

    OS_ERR os_err;
    CTimer0_Init(); //CTimer0 Init

    OSTaskCreate(&ptPulseTrainTaskTCB,
                "PulseTrainTask ",
                ptPulseTrainTask,
                (void *) 0,
                APP_CFG_PULSETRAINTASK_PRIO,
                &ptPulseTrainTaskStk[0],
                (APP_CFG_PULSETRAINTASK_STK_SIZE / 10u),
                APP_CFG_PULSETRAINTASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

}


/*****************************************************************************************
* TASK - PulseTrainTask
* Pends the semaphore flag and waiting until its posted. Then the getTrigWave function is
* called and will get the updated values for the pulse train from the usersettings. If the
* values are updated then the pulse train will be updated.
*
*****************************************************************************************/
static void ptPulseTrainTask(void *p_arg){
    OS_ERR os_err;
    INT32U ptFreq = 0;   //Variable holding the frequency of the Pule Train
    INT32U ptDutyCycle = 0; // Variable holding the dutycycle of the Pulse Train
    INT32U MR0Value = 0;    //Value placed in the MR0 register to change the freq of PT
    INT32U newPtFreq = 0;   //new freq from user settings
    INT32U newPtDutyCycle = 0;  //New duty cycle from user settings
    INT32U MR1Value = 0;

    (void)p_arg;

    while(1){
        DB3_TURN_OFF();
        OSTimeDly(10, OS_OPT_TIME_PERIODIC, &os_err);
        assert(os_err == OS_ERR_NONE);
        DB3_TURN_ON();
        newPtFreq = getTrigFreq(); //Get new frequency
        newPtDutyCycle = getTrigDC(); //Get new duty cycle

        //if statment that checks if frequency or duty cycle is changed
        if((newPtFreq != ptFreq)|| (newPtDutyCycle != ptDutyCycle)){
            //Updating values
            ptFreq = newPtFreq;
            ptDutyCycle = newPtDutyCycle;

            //Math for frequency for pulsetrain
            MR0Value = ((150000000)/ptFreq);  //MR0 value = (freq of clk) / f

            //Math for dutycycle
            MR1Value = ((MR0Value)*((100-ptDutyCycle))/100);

            // Set MR0 to register to change freq of pulsetrain
            CTIMER0->MR[0] = CTIMER_MR_MATCH(MR0Value - 1); //Set frquency of pulse train

            // Set MR1 to register the dutycycle of pulsetrain
            CTIMER0->MR[1] = CTIMER_MR_MATCH(MR1Value); //set duty cycle
            CTIMER0->TCR |= CTIMER_TCR_CRST(1); // Assert reset bit to reset timer to 0
            CTIMER0->TCR &= ~CTIMER_TCR_CRST(1); // Clear reset bit to start counting again

        }else{}

    }
}


