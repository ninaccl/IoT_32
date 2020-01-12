#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h" 
#include "includes.h"

 
/************************************************
 ALIENTEKս��STM32������ʵ��52
 UCOSIIʵ��2-�ź��������� ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//����������
//�����������ȼ�
#define HEART_TASK_PRIO       			4 
//���������ջ��С
#define HEART_STK_SIZE  		    		64
//�����ջ	
OS_STK HEART_TASK_STK[HEART_STK_SIZE];
//������
void heart_task(void *pdata);

//������Ϣ��������
//�����������ȼ�
#define ANALYZE_TASK_PRIO       		 	3
//���������ջ��С
#define ANALYZE_STK_SIZE  				128
//�����ջ	
OS_STK ANALYZE_TASK_STK[ANALYZE_STK_SIZE];
//������
void analyze_task(void *pdata);

//LED����
//�����������ȼ�
#define LED_TASK_PRIO       			6 
//���������ջ��С
#define LED_STK_SIZE  					64
//�����ջ	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);

//KEY����
//�����������ȼ�
#define KEY_TASK_PRIO       			5 
//���������ջ��С
#define KEY_STK_SIZE  					64
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);
 
//////////////////////////////////////////////////////////////////////////////
//OS_EVENT * msg_instruction;			//ָ�������¼���ָ��
OS_EVENT * msg_light;	//ָ����¼���ָ��
OS_EVENT * sem_heart;		//�������ź���ָ��	 
enum WIFI_STATE{TRANS,WAITSERVER,WAITAP}wifistate;	//WIFI״̬

 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	usart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	 
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
  OSInit();  	 			//��ʼ��UCOSII
  OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}

////////////////////////////////////////////////////////////////////////////////
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	wifistate = TRANS;
	msg_instruction=OSMboxCreate((void*)0);	//������Ϣ����
	msg_light=OSMboxCreate((void*)0);	//������Ϣ����
	sem_heart=OSSemCreate(0);		//�����ź��� 			  
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(heart_task,(void *)0,(OS_STK*)&HEART_TASK_STK[HEART_STK_SIZE-1],HEART_TASK_PRIO);	 				   
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);			
	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
 	OSTaskCreate(analyze_task,(void *)0,(OS_STK*)&ANALYZE_TASK_STK[ANALYZE_STK_SIZE-1],ANALYZE_TASK_PRIO);	 	
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}	  
//LED����
void led_task(void *pdata)
{
	char* msg;
	static u8 RxBuffer[]="okay";
	u8 err;
	u8 i;
	while(1)
	{
	//TODO
	//������Ϣ
	msg = (char *)OSMboxPend(msg_light,0,&err);
	if(strcmp(msg,"light")==0)
	{
		//LIGHT???
		LED0=!LED0;
		LED1=!LED1;
		//���ڷ���okay
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
		key=KEY_Scan(0);	//�õ���ֵ
		if(key)			//������Ϣ
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

//����������
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
			//����������
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
			
			//�ź����յ���������
			OSSemPend(sem_heart,4000,&err);			
			
			//û�յ�������
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
			//�յ�������
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




//ָ���������
void analyze_task(void *pdata)
{							 
	//������Ϣ����
	char *msg;
	u8 err;	
	while(1)
	{
		//������Ϣ
		msg = (char *)OSMboxPend(msg_instruction,0,&err);
			//TODO
			//AP�����Ϣ��
		if(strcmp(msg, "WIFI CONNECTED")==0)
			wifistate = WAITAP;
		else if(strcmp(msg, "WIFI DISCONNECT")==0)
			wifistate = TRANS;
		//����������
		else if(strcmp(msg, "ping")==0)
			//�����ź���
			OSSemPost(sem_heart);
		else	
			//ת����ָ�������
			OSMboxPost(msg_light, msg);
		delay_ms(200);
	}
} 










