#include <reg52.h>

sbit PWMOUT = P2^1; //电机引脚
sbit PWMOUT2= P2^0;

#define MAX 100 //随机数最大值
#define MIN 20   //随机数最小值
extern unsigned char HighRH;
extern unsigned char HighRL;
extern unsigned char LowRH;
extern unsigned char LowRL;
extern unsigned char PWM;

//sbit GG=P3^1; //??????????????????????????????????????????

void ConfigPWM(unsigned int fr, unsigned char dc);
void ClosePWM();

void ConfigPWM(unsigned int fr, unsigned char dc)
{
	unsigned long tmp;
	unsigned int high, low;

    PWMOUT2=0;

    if((dc<MAX)&&(dc>MIN))
    {
    	tmp = 11059200/12/fr;
    	high = (tmp*dc)/100;
    	low = tmp - high;
    	high = 65536 - high + 12;
    	low = 65536 - low + 12;
    	HighRH = (unsigned char)(high >>8);
    	HighRL = (unsigned char)high;
    	LowRH = (unsigned char)(low >> 8);
    	LowRL = (unsigned char)low;
    	TMOD &= 0xF0;
    	TMOD |= 0x01;
    	TH0 = HighRH;
    	TL0 = HighRL;
    	ET0 = 1;
    	TR0 = 1;
    	PWMOUT = 1;
    }
    else if(dc<=MIN)
    {
        ClosePWM();
        PWM=MIN;
        PWMOUT=0;
    }
    else
    {
        ClosePWM();
        PWM=MAX;
        PWMOUT=1;    
    } 
    
//    GG=0;//?????????????????????????????????????
	
}
void ClosePWM()
{
	TR0 = 0;
	ET0 = 0;
//	PWMOUT = 0;



//    GG=0;
}
