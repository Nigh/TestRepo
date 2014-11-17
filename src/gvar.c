#include "r_cg_userdefine.h"

sSELF sSelf={0,0,0,0};
sUTCS sUtcs,sAlarmTime;
uint steps=0;
uchar directGEn=0;


time_t time2(void)
{
	return (time_t)sUtcs.lTime;
}

void startHClk(void)
{
	TDR05=0xC34F;
	if(HIOSTOP)
	{
		HIOSTOP = 0;
		NOP();
		NOP();
		NOP();
		CSS = 0;
		NOP();
		NOP();
		NOP();
		// P2.3=0;
	}
}

void stopHClk(void)
{
	uchar i=0;
	if(CSS == 0)
	{
		CSS = 1;
		HIOSTOP = 1;
		// P2.3=1;
		// P2.3=0;
		while(i++<7){
			NOP();
		}
		TDR05=200;
		TS0=0x0020;
		// P2.3=1;
	}
}

void _nop_Ex(void)
{
	NOP();
}

void _halt_Ex(void)
{
	if(sUart.statu==UART_IDLE && sLed.ledMode==LED_M_OFF)
		stopHClk();
	HALT();
	NOP();
}
