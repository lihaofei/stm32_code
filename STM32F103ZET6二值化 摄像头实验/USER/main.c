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
 ALIENTEK战舰STM32开发板实验35
 摄像头OV7670 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/
/*彩色图象的灰度其实在转化为黑白图像后的像素值（是一种广义的提法），转化的方法看应用的领域而定，一般按加权的方法转换，R， G，B 的比一般为3：6：1。
任何颜色都由红、绿、蓝三基色组成，假如原来某点的颜色为RGB(R，G，B)，那么，我们可以通过下面几种方法，将其转换为灰度：
1.浮点算法：Gray=R*0.3+G*0.59+B*0.11
2.整数方法：Gray=(R*30+G*59+B*11)/100
3.移位方法：Gray =(R*77+G*151+B*28)>>8;
4.平均值法：Gray=（R+G+B）/3;
5.仅取绿色：Gray=G；*/
const u8*LMODE_TBL[5]={"Auto","Sunny","Cloudy","Office","Home"};							//5种光照模式	    
const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7种特效 
extern u8 ov_sta;	//在exit.c里 面定义
extern u8 ov_frame;	//在timer.c里面定义		 
//更新LCD显示
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
	if(gray<=100) gray=0;   //黑色
	else          gray=255; //白色
	
	//坐标
	if(y_j>=320) 
	{
		y_j=0;
		x_i++;
	}
	x_i++;  //点数加一
	if(x_i>=240)
	{
		x_i=0;
	}
	corlor[y_j][x_i]=gray;
	if(	corlor[y_j][x_i]==0) //第一次扫描到黑点
	{
	
		if(	corlor[y_j+10][x_i]==0) //类似于消抖
		{
			count=1;
			x_tem1=x_i;
			y_tem1=y_j;
		}
	}
	
		if(	(corlor[y_j][x_i]==0)&&count==1) //第二次扫描到黑点
	 {
	
		if(	corlor[y_j+5][x_i]==255) //类似于消抖
		{
			count=2;
			x_tem2=x_i;
			y_tem2=y_j;
		}
	 }
	 //求x y坐标值
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
			 x_max=x_max; //最大值
		 }
		 
		 
		 if(y_max<=y_t)
		 {	
	     y_max=y_t;			 
		 }
		 else
		 {
			 y_max=y_max; //最大值
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
	if(ov_sta)//有帧中断更新？
	{
		LCD_Scan_Dir(U2D_L2R);		//从上到下,从左到右  
		if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//将显示区域设置到屏幕中央
		else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//将显示区域设置到屏幕中央
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
		OV7670_RRST=0;				//开始复位读指针 
		OV7670_RCK_L;
		OV7670_RCK_H;
		OV7670_RCK_L;
		OV7670_RRST=1;				//复位读指针结束 
		OV7670_RCK_H;
		i=0;
		for(j=0;j<76800;j++)
		{
//    i=0;
		OV7670_RCK_L;
		color=GPIOC->IDR&0XFF;	//读数据
		OV7670_RCK_H; 
		color<<=8;  
		OV7670_RCK_L;
		color|=GPIOC->IDR&0XFF;	//读数据
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
//				color=GPIOC->IDR&0XFF;	//读数据
//				OV7670_RCK_H; 
//				color<<=8;  
//				OV7670_RCK_L;
//				color|=GPIOC->IDR&0XFF;	//读数据
//				OV7670_RCK_H; 
//			  STAMP_COLOR[j][i]=color;
////			color=ili9320_BGR2RGB(color);
//				  
//			}
//					LCD->LCD_RAM=color;  
//		}  

 		ov_sta=0;					//清零帧中断标记
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向 
	} 
}	   


 int main(void)
 {	 
	u8 key;
	u8 lightmode=0,saturation=2,brightness=2,contrast=0;
	u8 effect=0;	 
 	u8 i=0;	    
	u8 msgbuf[15];				//消息缓存区
	u8 tm=0; 

	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为 115200
 	usmart_dev.init(72);		//初始化USMART		
// 	LED_Init();		  			//初始化与LED连接的硬件接口
//	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD  
//	TPAD_Init(6);				//触摸按键初始化 
 	POINT_COLOR=RED;			//设置字体为红色 
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
	while(OV7670_Init())//初始化OV7670
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
	TIM6_Int_Init(10000,7199);			//10Khz计数频率,1秒钟中断									  
	EXTI8_Init();						//使能定时器捕获
	OV7670_Window_Set(12,176,240,320);	//设置窗口	  
  	OV7670_CS=0;					
	LCD_Clear(BLACK);
 	while(1)
	{	
//		key=KEY_Scan(0);//不支持连按
//		if(key)
//		{
//			tm=20;
//			switch(key)
//			{				    
//				case KEY0_PRES:	//灯光模式Light Mode
//					lightmode++;
//					if(lightmode>4)lightmode=0;
//					OV7670_Light_Mode(lightmode);
//					sprintf((char*)msgbuf,"%s",LMODE_TBL[lightmode]);
//					break;
//				case KEY1_PRES:	//饱和度Saturation
//					saturation++;
//					if(saturation>4)saturation=0;
//					OV7670_Color_Saturation(saturation);
//					sprintf((char*)msgbuf,"Saturation:%d",(signed char)saturation-2);
//					break;
//				case KEY2_PRES:	//亮度Brightness				 
//					brightness++;
//					if(brightness>4)brightness=0;
//					OV7670_Brightness(brightness);
//					sprintf((char*)msgbuf,"Brightness:%d",(signed char)brightness-2);
//					break;
//				case WKUP_PRES:	//对比度Contrast			    
//					contrast++;
//					if(contrast>4)contrast=0;
//					OV7670_Contrast(contrast);
//					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-2);
//					break;
//			}
//		}	 
//		if(TPAD_Scan(0))//检测到触摸按键 
//		{
//			effect++;
//			if(effect>6)effect=0;
//			OV7670_Special_Effects(effect);//设置特效
//	 		sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
//			tm=20;
//		} 
		camera_refresh();//更新显示
 		if(tm)
		{
			LCD_ShowString((lcddev.width-240)/2+30,(lcddev.height-320)/2+60,200,16,16,msgbuf);
			tm--;
		}
		i++;
		if(i==15)//DS0闪烁.
		{
			i=0;
			LED0=!LED0;
 		}
	}	   
}













