#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "string.h"
#include "ov7670.h"
#include "tpad.h"
#include "timer.h"
#include "exti.h"
#include "usmart.h"

 
/************************************************
 ALIENTEKս��STM32������ʵ��35
 ����ͷOV7670 ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
/*��ɫͼ��ĻҶ���ʵ��ת��Ϊ�ڰ�ͼ��������ֵ����һ�ֹ�����ᷨ����ת���ķ�����Ӧ�õ����������һ�㰴��Ȩ�ķ���ת����R�� G��B �ı�һ��Ϊ3��6��1��
�κ���ɫ���ɺ졢�̡�������ɫ��ɣ�����ԭ��ĳ�����ɫΪRGB(R��G��B)����ô�����ǿ���ͨ�����漸�ַ���������ת��Ϊ�Ҷȣ�
1.�����㷨��Gray=R*0.3+G*0.59+B*0.11
2.����������Gray=(R*30+G*59+B*11)/100
3.��λ������Gray =(R*77+G*151+B*28)>>8;
4.ƽ��ֵ����Gray=��R+G+B��/3;
5.��ȡ��ɫ��Gray=G��*/
const u8*LMODE_TBL[5]={"Auto","Sunny","Cloudy","Office","Home"};							//5�ֹ���ģʽ	    
const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7����Ч 
extern u8 ov_sta;	//��exit.c�� �涨��
extern u8 ov_frame;	//��timer.c���涨��		 
//����LCD��ʾ
#define RGB565_MASK_RED        0xF800   
#define RGB565_MASK_GREEN      0x07E0   
#define RGB565_MASK_BLUE       0x001F  
void BackCoordinate()
{
	u8 x,y;
	
}


 	u16 ili9320_BGR2RGB(u16 c)
{
  u16  r, g, b, rgb ,gray;
	u16  corlor[320][240];
	u16 x_i,y_j;
	u16 x,y;
	u8 count ;
	u8 x_tem1,y_tem1;
	u8 x_tem2,y_tem2;
 
  b = (c & 0x001F)<< 3; //0001 1111
  g = (c & 0x07E0)>> 3; //0011 1111   0000 0111 1110  
  r = (c & 0xF800)>>8; //0001 1111
	
	gray =(r*77+g*150+b*29+128)/256;
	if(gray<=100) gray=0;   //��ɫ
	else          gray=255; //��ɫ
	
	//����
	if(y_j>=320) 
	{
		y_j=0;
		x_i++;
	}
	x_i++;  //������һ
	if(x_i>=240)
	{
		x_i=0;
	}
	corlor[y_j][x_i]=gray;
	if(	corlor[y_j][x_i]==0) //��һ��ɨ�赽�ڵ�
	{
	
		if(	corlor[y_j+10][x_i]==0) //����������
		{
			count=1;
			x_tem1=x_i;
			y_tem1=y_j;
		}
	}
	
		if(	(corlor[y_j][x_i]==0)&&count==1) //�ڶ���ɨ�赽�ڵ�
	 {
	
		if(	corlor[y_j+5][x_i]==255) //����������
		{
			count=2;
			x_tem2=x_i;
			y_tem2=y_j;
		}
	 }
	 //��x y����ֵ
	 if(count==2)
	 {
		 u16 x_t,y_t,y_max,x_max;
		 count=0;
		 x_t=x_tem2-x_tem1;
		 y_t=y_tem2-y_tem1;
		 if(x_max<=x_t)
		 {	
	     x_max=x_t;			 
		 }
		 else
		 {
			 x_max=x_max; //���ֵ
		 }
		 
		 
		 if(y_max<=y_t)
		 {	
	     y_max=y_t;			 
		 }
		 else
		 {
			 y_max=y_max; //���ֵ
		 }
		
	 }
//	printf("gray1=%d\r\n",gray);
	gray=gray/8;
//		printf("gray2=%d\r\n",gray);
	c=(0x001f&gray)<<11;
	c=c|(((0x003f)&(gray*2))<<5);
	c=c|(0x001f&gray);
	rgb=c;
//	b = 
//  
//	b = (c)  & 0x1f; //0001 1111
//  g = (c)  & 0x07e0; //0011 1111
//  r = (c)  & 0xf800; //0001 1111
//  r=255;
//	g=255;
//	b=255;
//  rgb =((r<<11)+(g<<5)+(b));//g<<5;  //(b<<11);// + (g<<5) + (r<<0);
//	printf("b=%d\r\n",b);
//	printf("g=%d\r\n",g);
//	printf("r=%d\r\n",r);
  return( rgb );
}
void camera_refresh(void)
{
	u32 j,i;
	u16 COLOR[76800];
 	u16 color;	 
	if(ov_sta)//��֡�жϸ��£�
	{
		LCD_Scan_Dir(U2D_L2R);		//���ϵ���,������  
		if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//����ʾ�������õ���Ļ����
		else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//����ʾ�������õ���Ļ����
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
		OV7670_RRST=0;				//��ʼ��λ��ָ�� 
		OV7670_RCK_L;
		OV7670_RCK_H;
		OV7670_RCK_L;
		OV7670_RRST=1;				//��λ��ָ����� 
		OV7670_RCK_H;
		i=0;
		for(j=0;j<76800;j++)
		{
//    i=0;
		OV7670_RCK_L;
		color=GPIOC->IDR&0XFF;	//������
		OV7670_RCK_H; 
		color<<=8;  
		OV7670_RCK_L;
		color|=GPIOC->IDR&0XFF;	//������
		OV7670_RCK_H; 
		color=ili9320_BGR2RGB(color);
//    COLOR[i]=color;
//			i++;
//						printf("color=%d,\r\n",color);
		LCD->LCD_RAM= color;  

//    if(i==76799) i=0;			
		//			color=ili9320_BGR2RGB(color);
	 
		}   		

//		for(j=0;j<320;j++)
//		{
//			for(i=0;i<240;i++)
//			{
//				OV7670_RCK_L;
//				color=GPIOC->IDR&0XFF;	//������
//				OV7670_RCK_H; 
//				color<<=8;  
//				OV7670_RCK_L;
//				color|=GPIOC->IDR&0XFF;	//������
//				OV7670_RCK_H; 
//			  STAMP_COLOR[j][i]=color;
////			color=ili9320_BGR2RGB(color);
//				  
//			}
//					LCD->LCD_RAM=color;  
//		}  

 		ov_sta=0;					//����֡�жϱ��
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽�� 
	} 
}	   


 int main(void)
 {	 
	u8 key;
	u8 lightmode=0,saturation=2,brightness=2,contrast=0;
	u8 effect=0;	 
 	u8 i=0;	    
	u8 msgbuf[15];				//��Ϣ������
	u8 tm=0; 

	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ 115200
 	usmart_dev.init(72);		//��ʼ��USMART		
// 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
//	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD  
//	TPAD_Init(6);				//����������ʼ�� 
 	POINT_COLOR=RED;			//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"WarShip STM32");	
	LCD_ShowString(30,70,200,16,16,"OV7670 TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2015/1/18"); 
	LCD_ShowString(30,130,200,16,16,"KEY0:Light Mode");
	LCD_ShowString(30,150,200,16,16,"KEY1:Saturation");
	LCD_ShowString(30,170,200,16,16,"KEY2:Brightness");
	LCD_ShowString(30,190,200,16,16,"KEY_UP:Contrast");
	LCD_ShowString(30,210,200,16,16,"TPAD:Effects");	 
  	LCD_ShowString(30,230,200,16,16,"OV7670 Init...");	  
	while(OV7670_Init())//��ʼ��OV7670
	{
		LCD_ShowString(30,230,200,16,16,"OV7670 Error!!");
		delay_ms(200);
	    LCD_Fill(30,230,239,246,WHITE);
		delay_ms(200);
	}
 	LCD_ShowString(30,230,200,16,16,"OV7670 Init OK");
	delay_ms(1500);	 	   
	OV7670_Light_Mode(lightmode);
	OV7670_Color_Saturation(saturation);
	OV7670_Brightness(brightness);
	OV7670_Contrast(contrast);
 	OV7670_Special_Effects(effect);	 
	TIM6_Int_Init(10000,7199);			//10Khz����Ƶ��,1�����ж�									  
	EXTI8_Init();						//ʹ�ܶ�ʱ������
	OV7670_Window_Set(12,176,240,320);	//���ô���	  
  	OV7670_CS=0;					
	LCD_Clear(BLACK);
 	while(1)
	{	
//		key=KEY_Scan(0);//��֧������
//		if(key)
//		{
//			tm=20;
//			switch(key)
//			{				    
//				case KEY0_PRES:	//�ƹ�ģʽLight Mode
//					lightmode++;
//					if(lightmode>4)lightmode=0;
//					OV7670_Light_Mode(lightmode);
//					sprintf((char*)msgbuf,"%s",LMODE_TBL[lightmode]);
//					break;
//				case KEY1_PRES:	//���Ͷ�Saturation
//					saturation++;
//					if(saturation>4)saturation=0;
//					OV7670_Color_Saturation(saturation);
//					sprintf((char*)msgbuf,"Saturation:%d",(signed char)saturation-2);
//					break;
//				case KEY2_PRES:	//����Brightness				 
//					brightness++;
//					if(brightness>4)brightness=0;
//					OV7670_Brightness(brightness);
//					sprintf((char*)msgbuf,"Brightness:%d",(signed char)brightness-2);
//					break;
//				case WKUP_PRES:	//�Աȶ�Contrast			    
//					contrast++;
//					if(contrast>4)contrast=0;
//					OV7670_Contrast(contrast);
//					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-2);
//					break;
//			}
//		}	 
//		if(TPAD_Scan(0))//��⵽�������� 
//		{
//			effect++;
//			if(effect>6)effect=0;
//			OV7670_Special_Effects(effect);//������Ч
//	 		sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
//			tm=20;
//		} 
		camera_refresh();//������ʾ
 		if(tm)
		{
			LCD_ShowString((lcddev.width-240)/2+30,(lcddev.height-320)/2+60,200,16,16,msgbuf);
			tm--;
		}
		i++;
		if(i==15)//DS0��˸.
		{
			i=0;
			LED0=!LED0;
 		}
	}	   
}













