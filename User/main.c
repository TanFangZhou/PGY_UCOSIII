#include "stm32f4xx.h"
#include "led.h"
#include  <includes.h>



#define START_TASK_PRIO		3
#define START_STK_SIZE 		512
OS_TCB StartTaskTCB;
CPU_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *p_arg);





#define LED1_TASK_PRIO		5
#define LED1_STK_SIZE 		128
OS_TCB Led1TaskTCB;	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);





OS_TMR OsTmr;
void tim_callback(void *p_arg);


int main()
{
	OS_ERR  err;
	OSInit(&err);                                               /* Init uC/OS-III.                                      */

	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		
				 (CPU_CHAR	* )"start task", 		
                 (OS_TASK_PTR )start_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )START_TASK_PRIO,     
                 (CPU_STK   * )&START_TASK_STK[0],	
                 (CPU_STK_SIZE)START_STK_SIZE/10,	
                 (CPU_STK_SIZE)START_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);		

	OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */

	while(1);

	
}

static  void  start_task(void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;

	  CPU_SR_ALLOC();
   (void)p_arg;

    LED_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
                 / (CPU_INT32U)OSCfg_TickRate_Hz;

    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

    Mem_Init();                                                 /* Initialize memory managment module                   */
  

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif


#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
    App_SerialInit();                                           /* Initialize Serial communication for application ...  */
#endif

	OSTmrCreate ((OS_TMR  *)&OsTmr,
										 (CPU_CHAR *)"tmr",
										 500,
										 100,
										 (OS_OPT)OS_OPT_TMR_PERIODIC,
										 (OS_TMR_CALLBACK_PTR)tim_callback,
										 (void *)0,
										 (OS_ERR *)err);

	OS_CRITICAL_ENTER();	
    
	OSTaskCreate((OS_TCB 	* )&Led1TaskTCB,		
				 (CPU_CHAR	* )"led1 task", 		
                 (OS_TASK_PTR )led1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED1_TASK_PRIO,     
                 (CPU_STK   * )&LED1_TASK_STK[0],	
                 (CPU_STK_SIZE)LED1_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);		
				 								 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);			 
	OS_CRITICAL_EXIT();	
}




void led1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	OSTmrStart(&OsTmr,&err);
	while(1)
	{
		GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


void tim_callback(void *p_arg)
{
	GPIO_ToggleBits(GPIOF,GPIO_Pin_10);
}
