#include "main.h"
#include "gvar.h"

#define VECTOR(vec) vec

fFUNC const msgHandler[]={		// No
	VECTOR(_halt_Ex),				// 0
	VECTOR(fRtcPro),			// 1
	VECTOR(fKeyPro),			// 2
	VECTOR(fTimerPro),			// 3
	VECTOR(fBLEPro),			// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
	VECTOR(_nop_Ex),				// 9
	VECTOR(_nop_Ex),				// 10
	VECTOR(_nop_Ex),				// 11
	VECTOR(_nop_Ex),				// 12
	VECTOR(_nop_Ex),				// 13
	VECTOR(_nop_Ex),				// 14
	VECTOR(fReset),				// 15
};

sMSG gMsg={0,0};	//global message

void afterBoot(void)
{

}

void iMain(void)
{
	afterBoot();
	fifoInit(sMsgFifo,msgQueue);
	fifoFlush();
	// R_TAU0_Channel0_Start();
	ledSetMode(LED_M_OFF);
	EI();
	while(1){
		DI();
		gMsg=fifoGet();
		EI();
		NOP();
		msgHandler[gMsg.type]();
	}
}

void fReset(void)
{
	
}

void fRtc2Hz(void)
{
	static uint count=0;
	count++;

	switch(count)
	{
		case 4:
		ledSetMode(LED_M_SWING);
		break;

		case 16:
		ledSetMode(LED_M_BREATHE);
		break;

		case 32:
		ledSetMode(LED_M_MQ);
		break;

		case 42:
		ledSetMode(LED_M_POWER);
		break;

		case 52:
		ledSetMode(LED_M_HERATBEAT);
		break;

		case 62:
		ledSetMode(LED_M_FLASHALL);
		break;

		case 72:
		ledSetMode(LED_M_RANDOM);
		break;

		case 82:
		ledSetMode(LED_M_OFF);
		break;

		case 84:
		count=0;
		break;
	}
}

fFUNC const rtcHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(fRtc2Hz),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fRtcPro(void)
{
	rtcHandler[gMsg.content]();
}


fFUNC const keyHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fKeyPro(void)
{
	keyHandler[gMsg.content]();
}


fFUNC const timerHandler[]={	// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fTimerPro(void)
{
	timerHandler[gMsg.content]();
}


fFUNC const bleHandler[]={		// No
	VECTOR(_nop_Ex),				// 0
	VECTOR(_nop_Ex),				// 1
	VECTOR(_nop_Ex),				// 2
	VECTOR(_nop_Ex),				// 3
	VECTOR(_nop_Ex),				// 4
	VECTOR(_nop_Ex),				// 5
	VECTOR(_nop_Ex),				// 6
	VECTOR(_nop_Ex),				// 7
	VECTOR(_nop_Ex),				// 8
};

void fBLEPro(void)
{
	bleHandler[gMsg.content]();
}
