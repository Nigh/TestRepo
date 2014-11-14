
#pragma interrupt INTAD intADC
#include "r_cg_macrodriver.h"
#include "r_cg_adc.h"

#include "battery.h"
uint batteryLevel=0xffff;

static uchar adcCount=0;
static uint adcValue[4];

void startAD(void)
{
	adcCount=0;
	P2.1=0;
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

__interrupt static void intADC(void)
{
	static const sMSG sMsg={M_TYPE_SYS,M_C_ADCEND};
	if(adcCount!=0)
		adcValue[adcCount++-1]=ADCR;
	if(adcCount<5)
		R_ADC_Start();
	else{
		stopAD();
		batteryLevel=(adcValue[0]>>2)+(adcValue[1]>>2)+(adcValue[2]>>2)+(adcValue[3]>>2);
		fifoPut4ISR(sMsg);
	}
}
