#include <reg52.h>
#include <stdlib.h>

#define MAX 100 //随机数最大值
#define MIN 20   //随机数最小值

bit flag500ms=0;         //500ms定时标志
bit flag1s = 0;          //1s定时标志
unsigned int number=0;  //测速外部计数
unsigned int number2=0; //备份计数值


unsigned char RBUFF = 0; //从UART读出的数据
unsigned char T0RH = 0;  //T0重载值的高字节
unsigned char T0RL = 0;  //T0重载值的低字节

//产生随机数
unsigned char random(unsigned int seed);
//18B20声明
void ConfigTimer1();//1s中断
unsigned char IntToString(unsigned char *str, int dat);
extern bit Start18B20();
extern bit Get18B20Temp(int *temp);
//LCD声明
extern void InitLcd1602();
extern LcdWriteCmd(unsigned char dat);  //清屏
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);
//PWM声明
sbit PWMOUT = P2^1; //电机引脚
sbit PWMOUT2=P2^0;
//sbit CES=P1^7;
unsigned char HighRH=0;
unsigned char HighRL=0;
unsigned char LowRH=0;
unsigned char LowRL=0;

extern void ConfigPWM(unsigned int fr, unsigned char dc);
extern void ClosePWM();
//UART声明
extern void ConfigUART();
void delay(unsigned char n);

unsigned char PWM=MIN;     //PWM初始化
//测速
float numfloat =0;         //速度

void main()
{

    unsigned char numChar =0; //速度整数部分
    unsigned char numxiao =0;

    bit res;
    int temp;        //读取到的当前温度值
    int intT, decT;  //温度值的整数和小数部分
    int intT2=0;
    unsigned char len;
    unsigned char str[12];          
    unsigned char mode = 0;  //电机模式选择位
    PWMOUT=0;                //电机引脚初始化
    PWMOUT2=0;


    
    InitLcd1602();     //初始化液晶
    ConfigUART();  //配置波特率为9600
    ConfigTimer1();  //配置定时器1       1s 500ms
    Start18B20();      //启动DS18B20

    RBUFF=6;
//    CES=1;

    EX0=1;
    IT0=1;
    EA = 1;            //开总中断
    while(1)
    {

//测速部分--------------------------------------------------------
    ET1=1;
    numfloat=number2/20;
    numChar=(unsigned char)numfloat;
    numxiao=(int)(numfloat*10)%10;

    len = IntToString(str, (int)numChar); //整数部分转换为字符串
    str[len++] = '.';             //添加小数点
//    decT = (decT*10) / 16;        //二进制的小数部分转换为1位十进制位
    str[len++] =numxiao + '0';      //十进制小数位再转换为ASCII字符
    while (len < 5)               //用空格补齐到6个字符长度
    {
        str[len++] = ' ';
    }
    str[len] = '\0';              //添加字符串结束符
    LcdShowStr(0, 1, "V=");
    LcdShowStr(3, 1, str);        //显示到液晶屏上
//---------------------------------------------------------


//        RBUFF=5;
        mode=RBUFF;
        switch (mode)
        {
            case 0://加速
            LcdWriteCmd(0x01);
            LcdShowStr(0,0,"Mode: accelerate");//显示模式
            
            if(PWM>=MAX)
            {
                PWM=MAX;
                LcdWriteCmd(0x01);
                LcdShowStr(9,1,"Fastest!");
            }else PWM+=10;
            ConfigPWM(2000, PWM);
            RBUFF=5;
            break;
            case 1://减速
            LcdWriteCmd(0x01);
            LcdShowStr(0,0,"Mode: decelerate");//显示模式
            if((PWM<=MIN)||(PWM>MAX))
            {
                PWM=MIN;
                LcdWriteCmd(0x01);
                LcdShowStr(9,1,"Stop");
            }else PWM-=10;
            ConfigPWM(2000, PWM);
            RBUFF=5;
            break;
            case 2://自然风
            LcdWriteCmd(0x01);
            LcdShowStr(0,0,"Mode:NaturalWind");
            case 10:            //自然风
            ET1=1;   //  配置定时器1定时1s
            if(flag500ms)
            {
                PWM=random(((unsigned int)TH0<<8)|TL0);
                ConfigPWM(100,PWM);
                flag500ms=0;
            }
            RBUFF=10;
            break;
            case 3://温度自动调控速度
            ET1=1;
//            LcdShowStr(8,0,"Mode:T=");
            LcdShowStr(0,0,"Temperature:");
            LcdShowStr(9,1,"T=");


            if (flag1s)  //每秒更新一次温度
            {
                flag1s = 0;
                res = Get18B20Temp(&temp);  //读取当前温度
                if (res)                    //读取成功时，刷新当前温度显示
                {
                    intT = temp >> 4;             //分离出温度值整数部分                   
                    decT = temp & 0xF;            //分离出温度值小数部分
                    len = IntToString(str, intT); //整数部分转换为字符串
                    str[len++] = '.';             //添加小数点
                    decT = (decT*10) / 16;        //二进制的小数部分转换为1位十进制位
                    str[len++] = decT + '0';      //十进制小数位再转换为ASCII字符
                    while (len < 6)               //用空格补齐到6个字符长度
                    {
                        str[len++] = ' ';
                    }
                    str[len] = '\0';              //添加字符串结束符
                    LcdShowStr(11, 1, str);        //显示到液晶屏上
                    Start18B20();               //重新启动下一次转换
                }
                else                        //读取失败时，提示错误信息
                {
                    LcdWriteCmd(0x01);
                    LcdShowStr(0, 1, "error!");
                }
                if(intT2!=0)   //判断是否是第一次测温
                {
                    if(intT2>intT)
                    PWM-=20*(intT2-intT);
                    if(intT2<intT)
                    PWM+=20*(intT-intT2);
                }
                intT2=intT;                 //备份整数部分用来比较
                ConfigPWM(100,PWM);
                RBUFF=3;
            }
            break;
            case 4://关闭电动机
            LcdWriteCmd(0x01);
            LcdShowStr(0,0,"close!");
            ClosePWM();
            PWMOUT=0;
            PWM=MIN;
            RBUFF=5;
            break;
            case 5://hold
            break;
            case 6://测试用
            LcdShowStr(0,0,"ceshi!");
            ClosePWM();
            PWMOUT=1;
            PWM=MAX;
            RBUFF=5;
            break; 
            default:
            LcdShowStr(0,0,"Please Input:");                                       
        }

    }

}
void ConfigTimer1() //配置启动定时器1
{
    TMOD&=0X0F;
    TMOD|=0X10;
    TH1=0x48;
    TL1=0xFF;
    ET1=0;
    TR1=1;
}

/* 整型数转换为字符串，str-字符串指针，dat-待转换数，返回值-字符串长度 */
unsigned char IntToString(unsigned char *str, int dat)
{
    signed char i = 0;
    unsigned char len = 0;
    unsigned char buf[6];
    
    if (dat < 0)  //如果为负数，首先取绝对值，并在指针上添加负号
    {
        dat = -dat;
        *str++ = '-';
        len++;
    }
    do {          //先转换为低位在前的十进制数组
        buf[i++] = dat % 10;
        dat /= 10;
    } while (dat > 0);
    len += i;     //i最后的值就是有效字符的个数
    while (i-- > 0)   //将数组值转换为ASCII码反向拷贝到接收指针上
    {
        *str++ = buf[i] + '0';
    }
    *str = '\0';  //添加字符串结束符
    
    return len;   //返回字符串长度
}
//产生MIN到MAX的随机数
unsigned char random(unsigned int seed)
{
    unsigned char value;
    srand(seed);
    value=rand()%(MAX+1-MIN)+MIN;
    return value;
}

//定时器1    1s  500ms
void Interrupttime1() interrupt 3
{
    static unsigned char cnt=0;
    TH1=0x48;
    TL1=0xFF;
    cnt++;
    if(cnt==10)//500ms  
    {
        flag500ms=1;
    }
    if(cnt>=20) //500ms  1s
    {
        cnt=0;
        flag500ms=1;
        flag1s=1;
        number2=number;
        number=0;
    }


	
}
//PWM定时器0
void InterruptTimer0() interrupt 1
{
    if (PWMOUT == 1)  //当前输出为高电平时，装载低电平值并输出低电平
    {
        TH0 = LowRH;
        TL0 = LowRL;
        PWMOUT = 0;
    }
    else              //当前输出为低电平时，装载高电平值并输出高电平
    {
        TH0 = HighRH;
        TL0 = HighRL;
        PWMOUT = 1;
    }
}
//定时器2用做波特率发生器
void InterruptUART() interrupt 4
{
    EA=0;
//    if(EXF2==1)
//    {
//        number++;
//        EXF2=0;    
//    }
    if(RI)
	{
		RI = 0;
		RBUFF = SBUF;
//        SBUF=SBUF;
	}
//    if(TI)
//    {
//        TI=0;  
//    }
    EA=1;
}

void interruptwai0() interrupt 0
{
    number++;
}

void delay(unsigned char n)
{
    unsigned char i=0,j=0;
    for(j=0;j<n;j++)
    for(i=0;i<120;i++);
}


