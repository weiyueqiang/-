#include <reg52.h>

void ConfigUART();

void ConfigUART()
{
//    PCON &=0x7F;
    SCON=0x50;
    T2CON  = 0x34;  //配置串口为模式1
    RCAP2H=0xFF;
    RCAP2L =0xDC;     //初值等于重载值
    TH2=0xFF;
    TL2=0xDC;
//    TR2=1;

    ES  = 1;       //使能串口中断
}


