
#pragma interrupt INTAD intADC
#include "r_cg_macrodriver.h"
#include "r_cg_adc.h"

#include "battery.h"
int batteryLevel=0xffff;

static uchar adcCount=0;
uint adcValue[4];
uchar powerLevel=4;

void ADPro(void)
{
	if(sVibrate.en==0){
		if(P2.1!=0){
			P2.1=0;
			setADTimer(1);
		}else{
			startAD();
			setADTimer(600);
		}
	}else{
		setADTimer(10);
	}
}

void startAD(void)
{
	adcCount=0;
	ADCE = 1U;
	NOP();
	NOP();
	R_ADC_Start();
}

void stopAD(void)
{
	P2.1=1;
	R_ADC_Stop();
	ADCE = 0U;
}

sAPPTIMER adTimer={0,0,&ADPro};
void setADTimer(uint time)
{
	adTimer.en=1;
	adTimer.count=time;
}

__interrupt static void intADC(void)
{
	static const sMSG sMsg={M_TYPE_SYS,M_C_ADCEND};
	if(adcCount++!=0)
		adcValue[adcCount-2]=ADCR;
	if(adcCount<5)
		R_ADC_Start();
	else{
		stopAD();
		fifoPut4ISR(sMsg);
	}
}

