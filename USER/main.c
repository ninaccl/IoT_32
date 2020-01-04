#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h" 
#include "includes.h"

 
/************************************************
 ALIENTEKÕ½½¢STM32¿ª·¢°åÊµÑé52
 UCOSIIÊµÑé2-ĞÅºÅÁ¿ºÍÓÊÏä ÊµÑé 
 ¼¼ÊõÖ§³Ö£ºwww.openedv.com
 ÌÔ±¦µêÆÌ£ºhttp://eboard.taobao.com 
 ¹Ø×¢Î¢ĞÅ¹«ÖÚÆ½Ì¨Î¢ĞÅºÅ£º"ÕıµãÔ­×Ó"£¬Ãâ·Ñ»ñÈ¡STM32×ÊÁÏ¡£
 ¹ãÖİÊĞĞÇÒíµç×Ó¿Æ¼¼ÓĞÏŞ¹«Ë¾  
 ×÷Õß£ºÕıµãÔ­×Ó @ALIENTEK
************************************************/


/////////////////////////UCOSIIÈÎÎñÉèÖÃ///////////////////////////////////
//START ÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define START_TASK_PRIO      			10 //¿ªÊ¼ÈÎÎñµÄÓÅÏÈ¼¶ÉèÖÃÎª×îµÍ
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define START_STK_SIZE  				64
//ÈÎÎñ¶ÑÕ»	
OS_STK START_TASK_STK[START_STK_SIZE];
//ÈÎÎñº¯Êı
void start_task(void *pdata);	
 			   
//ĞÄÌø°üÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define HEART_TASK_PRIO       			4 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define HEART_STK_SIZE  		    		64
//ÈÎÎñ¶ÑÕ»	
OS_STK HEART_TASK_STK[HEART_STK_SIZE];
//ÈÎÎñº¯Êı
void heart_task(void *pdata);

//½ÓÊÕĞÅÏ¢·ÖÎöÈÎÎñ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define ANALYZE_TASK_PRIO       		 	3
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define ANALYZE_STK_SIZE  				128
//ÈÎÎñ¶ÑÕ»	
OS_STK ANALYZE_TASK_STK[ANALYZE_STK_SIZE];
//ÈÎÎñº¯Êı
void analyze_task(void *pdata);

//Ö¸ÁîÅĞ¶ÏºÍÖ´ĞĞ
//ÉèÖÃÈÎÎñÓÅÏÈ¼¶
#define LED_TASK_PRIO       			6 
//ÉèÖÃÈÎÎñ¶ÑÕ»´óĞ¡
#define LED_STK_SIZE  					64
//ÈÎÎñ¶ÑÕ»	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//ÈÎÎñº¯Êı
void led_task(void *pdata);
 
//////////////////////////////////////////////////////////////////////////////
//OS_EVENT * msg_instruction;			//Ö¸ÁîÓÊÏäÊÂ¼ş¿éÖ¸Õë
OS_EVENT * msg_light;	//Ö¸ÁîµÆÊÂ¼ş¿éÖ¸Õë
OS_EVENT * sem_heart;		//ĞÄÌø°üĞÅºÅÁ¿Ö¸Õë	 
enum USART_STATE{TRANS,WAITSERVER,WAITAP}usartstate;	//WIFI×´Ì¬

 int main(void)
 {	 		    
	delay_init();	    	 //ÑÓÊ±º¯Êı³õÊ¼»¯	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//ÉèÖÃÖĞ¶ÏÓÅÏÈ¼¶·Ö×éÎª×é2£º2Î»ÇÀÕ¼ÓÅÏÈ¼¶£¬2Î»ÏìÓ¦ÓÅÏÈ¼¶
	usart_init(115200);	 	//´®¿Ú³õÊ¼»¯Îª115200	 
	LED_Init();		  		//³õÊ¼»¯ÓëLEDÁ¬½ÓµÄÓ²¼ş½Ó¿Ú
  OSInit();  	 			//³õÊ¼»¯UCOSII
  OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//´´½¨ÆğÊ¼ÈÎÎñ
	OSStart();
}

////////////////////////////////////////////////////////////////////////////////
//¿ªÊ¼ÈÎÎñ
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	usartstate = TRANS;
	msg_instruction=OSMboxCreate((void*)0);	//´´½¨ÏûÏ¢ÓÊÏä
	msg_light=OSMboxCreate((void*)0);	//´´½¨ÏûÏ¢ÓÊÏä
	sem_heart=OSSemCreate(0);		//´´½¨ĞÅº=	 			  
	OSStatInit();					//³õÊ¼»¯Í³¼ÆÈÎÎñ.ÕâÀï»áÑÓÊ±1ÃëÖÓ×óÓÒ	
 	OS_ENTER_CRITICAL();			//½øÈëÁÙ½çÇø(ÎŞ·¨±»ÖĞ¶Ï´ò¶Ï)    
 	//OSTaskCreate(heart_task,(void *)0,(OS_STK*)&HEART_TASK_STK[HEART_STK_SIZE-1],HEART_TASK_PRIO);	 				   
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						   
 	OSTaskCreate(analyze_task,(void *)0,(OS_STK*)&ANALYZE_TASK_STK[ANALYZE_STK_SIZE-1],ANALYZE_TASK_PRIO);	 				   		   				   
 	OSTaskSuspend(START_TASK_PRIO);	//¹ÒÆğÆğÊ¼ÈÎÎñ.
	OS_EXIT_CRITICAL();				//ÍË³öÁÙ½çÇø(¿ÉÒÔ±»ÖĞ¶Ï´ò¶Ï)
}	  
//LEDÈÎÎñ
void led_task(void *pdata)
{
	char* msg;
	static u8 RxBuffer[]="okay";
	u8 err;
	u8 i;
	while(1)
	{
	//TODO
	//½ÓÊÕĞÅÏ¢
	msg = (char *)OSMboxPend(msg_light,0,&err);
	if(strcmp(msg,"light")==0)
	{
		//LIGHT???
		LED0=!LED0;
		LED1=!LED1;
		//´®¿Ú·¢ËÍokay
		//RxBuffer[0]='o';
		//RxBuffer[1]='k';
		//RxBuffer[2]='a';
		//RxBuffer[3]='y';
		for(i= 0;i < 4; i++)
		{
			USART_SendData(USART3, RxBuffer[i]);
			while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		}
	}
	delay_ms(200);
	}									 
}	   

//ĞÄÌø°üÈÎÎñ
void heart_task(void *pdata)
{
	u8 RxBuffer[4];
	u8 lossmsg=0;
	u8 err;
	u8 i;
	while(1)
	{
		if(usartstate != WAITAP)
		{
			//·¢ËÍĞÄÌø°ü
		RxBuffer[0]='p';
		RxBuffer[1]='i';
		RxBuffer[2]='n';
		RxBuffer[3]='g';
		for(i= 0;i < 4; i++)
		{
			USART_SendData(USART3, RxBuffer[i]);
			while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		}
			delay_ms(1000);
			
			//ĞÅºÅÁ¿ÊÕµ½ĞÄÌø°ü£¿
			OSSemPend(sem_heart,4000,&err);			
			
			//Ã»ÊÕµ½ĞÄÌø°ü
			if(err==OS_ERR_TIMEOUT)
			{
				if(usartstate == TRANS)
				{
					lossmsg++;
					if(lossmsg==3)					
					{
						usartstate = WAITSERVER;
						lossmsg = 0;
					}		
				}
			}	
			//ÊÕµ½ĞÄÌø°ü
			else
			{
				if(usartstate == WAITSERVER)
					usartstate = TRANS;
				delay_ms(10000);
			}
		}
		delay_ms(2000);
	}
}


//Ö¸Áî·ÖÎöÈÎÎñ
void analyze_task(void *pdata)
{							 
	//¶¨ÒåÏûÏ¢Êı×é
	char *msg;
	u8 err;	
	while(1)
	{
		//½ÓÊÕĞÅÏ¢
		msg = (char *)OSMboxPend(msg_instruction,0,&err);
			//TODO
			//APÏà¹ØĞÅÏ¢£¿
		if(strcmp(msg, "WIFI CONNECTED")==0)
			usartstate = WAITAP;
		else if(strcmp(msg, "WIFI DISCONNECT")==0)
			usartstate = TRANS;
		//ÊÇĞÄÌø°ü£¿
		else if(strcmp(msg, "ping")==0)
			//·¢ËÍĞÅºÅÁ¿
			OSSemPost(sem_heart);
		else	
			//×ª·¢¸øÖ¸Áî´¦ÀíÈÎÎñ
			OSMboxPost(msg_light, msg);
		delay_ms(200);
	}
} 










