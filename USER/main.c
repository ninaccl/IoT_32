#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h" 
#include "includes.h"

 
/************************************************
 ALIENTEK战舰STM32开发板实验52
 UCOSII实验2-信号量和邮箱 实验 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//心跳包任务
//设置任务优先级
#define HEART_TASK_PRIO       			4 
//设置任务堆栈大小
#define HEART_STK_SIZE  		    		64
//任务堆栈	
OS_STK HEART_TASK_STK[HEART_STK_SIZE];
//任务函数
void heart_task(void *pdata);

//接收信息分析任务
//设置任务优先级
#define ANALYZE_TASK_PRIO       		 	3
//设置任务堆栈大小
#define ANALYZE_STK_SIZE  				128
//任务堆栈	
OS_STK ANALYZE_TASK_STK[ANALYZE_STK_SIZE];
//任务函数
void analyze_task(void *pdata);

//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			6 
//设置任务堆栈大小
#define LED_STK_SIZE  					64
//任务堆栈	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//KEY任务
//设置任务优先级
#define KEY_TASK_PRIO       			5 
//设置任务堆栈大小
#define KEY_STK_SIZE  					64
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);
 
//////////////////////////////////////////////////////////////////////////////
//OS_EVENT * msg_instruction;			//指令邮箱事件块指针
OS_EVENT * msg_light;	//指令灯事件块指针
OS_EVENT * sem_heart;		//心跳包信号量指针	 
enum WIFI_STATE{TRANS,WAITSERVER,WAITAP}wifistate;	//WIFI状态

 int main(void)
 {	 		    
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	usart_init(115200);	 	//串口初始化为115200	 
	LED_Init();		  		//初始化与LED连接的硬件接口
	KEY_Init();          //初始化与按键连接的硬件接口
  OSInit();  	 			//初始化UCOSII
  OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}

////////////////////////////////////////////////////////////////////////////////
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	wifistate = TRANS;
	msg_instruction=OSMboxCreate((void*)0);	//创建消息邮箱
	msg_light=OSMboxCreate((void*)0);	//创建消息邮箱
	sem_heart=OSSemCreate(0);		//创建信号量 			  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(heart_task,(void *)0,(OS_STK*)&HEART_TASK_STK[HEART_STK_SIZE-1],HEART_TASK_PRIO);	 				   
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);			
	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
 	OSTaskCreate(analyze_task,(void *)0,(OS_STK*)&ANALYZE_TASK_STK[ANALYZE_STK_SIZE-1],ANALYZE_TASK_PRIO);	 	
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}	  
//LED任务
void led_task(void *pdata)
{
	char* msg;
	static u8 RxBuffer[]="okay";
	u8 err;
	u8 i;
	while(1)
	{
	//TODO
	//接收信息
	msg = (char *)OSMboxPend(msg_light,0,&err);
	if(strcmp(msg,"light")==0)
	{
		//LIGHT???
		LED0=!LED0;
		LED1=!LED1;
		//串口发送okay
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

void key_task(void *pdata)
{
	u8 key = 0;
	static u8 RxBuffer[]="press key";
	int i;
	while(1)
	{
		key=KEY_Scan(0);	//得到键值
		if(key)			//发送消息
		{
			for(i= 0;i < 9; i++)
			{
				USART_SendData(USART3, RxBuffer[i]);
				while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
			}
			delay_ms(200);
		}

		delay_ms(20);
	}
}

//心跳包任务
void heart_task(void *pdata)
{
	u8 RxBuffer[4];
	u8 lossmsg=0;
	u8 err;
	u8 i;
	while(1)
	{
		if(wifistate != WAITAP)
		{
			//发送心跳包
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
			
			//信号量收到心跳包？
			OSSemPend(sem_heart,4000,&err);			
			
			//没收到心跳包
			if(err==OS_ERR_TIMEOUT)
			{
				if(wifistate == TRANS)
				{
					lossmsg++;
					if(lossmsg==3)					
					{
						wifistate = WAITSERVER;
						lossmsg = 0;
					}		
				}
			}	
			//收到心跳包
			else
			{
				if(wifistate == WAITSERVER)
					wifistate = TRANS;
				delay_ms(10000);
			}
		}
		delay_ms(2000);
	}
}




//指令分析任务
void analyze_task(void *pdata)
{							 
	//定义消息数组
	char *msg;
	u8 err;	
	while(1)
	{
		//接收信息
		msg = (char *)OSMboxPend(msg_instruction,0,&err);
			//TODO
			//AP相关信息？
		if(strcmp(msg, "WIFI CONNECTED")==0)
			wifistate = WAITAP;
		else if(strcmp(msg, "WIFI DISCONNECT")==0)
			wifistate = TRANS;
		//是心跳包？
		else if(strcmp(msg, "ping")==0)
			//发送信号量
			OSSemPost(sem_heart);
		else	
			//转发给指令处理任务
			OSMboxPost(msg_light, msg);
		delay_ms(200);
	}
} 










