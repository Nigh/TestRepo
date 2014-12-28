#include "r_cg_userdefine.h"

sSELF sSelf={SYS_ACTIVE,0,0,0};
sUTCS sUtcs,sAlarmTime;

uchar version[16]=STR_VERSION;

uchar isTimeSync=DATA_BEFORE_TIMESYNC;

uint stepTarget=TARGET_STEP;
uint steps=0;
sSTEPLONGLOG currentStepLog={0};
uchar directGEn=0;
uchar batteryStatu=BAT_NORMAL;
sFLASHOP gOP;
sUPLOAD sUpload={0};
sCLKLEVEL sClkLevel={0};
uchar BLE_Connect_Timeout=0;

uchar neckUnhealthCount=0;

time_t time2(void)
{
	return (time_t)sUtcs.lTime;
}

void startHClk(void)
{
	SETCLK_REQ_CLR();
	SETCLK_H_OP();
	TDR05=0xC34F;
	if(HIOSTOP)
	{
		HIOSTOP = 0; NOP(); NOP(); NOP();
		CSS = 0; NOP(); NOP(); NOP();
	}
	SETCLK_HCLK();
	SETCLK_OP_CLR();
}

void stopHClk(void)
{
	uchar i=0;
	if(CSS == 0)
	{
		CSS = 1;
		HIOSTOP = 1;
		while(i++<7){
			NOP();
		}
		TDR05=200;
		TS0=0x0020;
	}
	SETCLK_LCLK();
	SETCLK_OP_CLR();
}

void _nop_Ex(void)
{
	NOP();
}

extern uchar BLEResetCount;
void _halt_Ex(void)
{
	SETCLK_L_OP();
	if(sUart.statu==UART_IDLE 
		&& BLEResetCount==0
		&& sLed.ledMode==LED_M_OFF 
		&& sVibrate.en==0 
		&& sUpload.statu==UPLOAD_IDLE 
		&& sSelf.mode!=SYS_OAD 
		&& sSelf.mode!=SYS_TEST)
		stopHClk();
	else
		startHClk();
	if(sClkLevel.level&CL_HREQ)
		startHClk();
	HALT();
	NOP();
}

