#include "led.h"
#include "r_cg_timer.h"

#define OSC_FREQ (8000)
#define PWM50 (OSC_FREQ/2)

static void pwmMClkOn(void);
static void pwmMClkOff(void);
void ledAllOff(void);

static uint* const _PWM[4]={&TDR01,&TDR02,&TDR03,&TDR04};


sLED sLed={{0,0,0,0},
		0,LED_M_OFF,
		{PWM_HOLD,PWM_HOLD,PWM_HOLD,PWM_HOLD}};

fFUNC ledMode[]={led_heartBeat,
				led_breathe,
				led_mq,
				led_flashAll,
				led_swing,
				led_power,
				led_random};

void ledSetMode(uint ledMode)
{
	uint* ptr=&sLed;

	*ptr++=0;*ptr++=0;*ptr++=0;*ptr++=0;
	*ptr++=0;*ptr++=ledMode;
	*ptr++=0;*ptr=0;

	// pwmMClkOn();
	// R_TAU0_Channel0_Start();
	// TOE0 &= 0xffff ^ ( _0002_TAU_CH1_OUTPUT_ENABLE
	// 	& _0004_TAU_CH2_OUTPUT_ENABLE
	// 	& _0008_TAU_CH3_OUTPUT_ENABLE
	// 	& _0010_TAU_CH4_OUTPUT_ENABLE );
	ledAllOff();
	switch(sLed.ledMode)
	{
		default:
		case LED_M_OFF:
			R_TAU0_Channel0_Stop();
			TO0 |= _0002_TAU_CH1_OUTPUT_VALUE_1
				| _0004_TAU_CH2_OUTPUT_VALUE_1
				| _0008_TAU_CH3_OUTPUT_VALUE_1
				| _0010_TAU_CH4_OUTPUT_VALUE_1;
		break;

		case LED_M_MQ:
		case LED_M_BREATHE:
			R_TAU0_Channel0_Start();
		break;

		case LED_M_FLASHALL:
		case LED_M_HERATBEAT:
		case LED_M_SWING:
		case LED_M_POWER:
		case LED_M_RANDOM:
			R_TAU0_Channel0_Start();
		break;
	}
}

int ledRise(uchar ledNo,uchar var)
{
	// TOE0|=(1<<(ledNo+1));
	if((*_PWM[ledNo])<PWM50) {
		(*_PWM[ledNo])+=var;
		return 0;
	} else {
		*(_PWM[ledNo])=PWM50;
		return 1;
	}
}

int ledFall(uchar ledNo,uchar var)
{
	if((*_PWM[ledNo])>var) {
		*(_PWM[ledNo])-=var;
		return 0;
	} else {
		*(_PWM[ledNo])=0;
		return 1;
	}
}



// extern void StopT16eCh0(void);
// extern void ResetT16eCh0(void);
#include "r_cg_timer.h"
void ledAllOff(void)
{
	*(_PWM[0])=0;
	*(_PWM[1])=0;
	*(_PWM[2])=0;
	*(_PWM[3])=0;

	// TOE0 &= 0xffff ^ ( _0002_TAU_CH1_OUTPUT_ENABLE
	// 	& _0004_TAU_CH2_OUTPUT_ENABLE
	// 	& _0008_TAU_CH3_OUTPUT_ENABLE
	// 	& _0010_TAU_CH4_OUTPUT_ENABLE );

	// TO0 |= _0002_TAU_CH1_OUTPUT_VALUE_1
	// 	| _0004_TAU_CH2_OUTPUT_VALUE_1
	// 	| _0008_TAU_CH3_OUTPUT_VALUE_1
	// 	| _0010_TAU_CH4_OUTPUT_VALUE_1;
}

void ledNext(void)
{
	if(sLed.ledMode<=6)
		ledMode[sLed.ledMode]();
	else
		ledSetMode(LED_M_OFF);
}


void led_heartBeat(void)
{
	if(sLed.ledCount<1200){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	if(sLed.ledCount==1){
		led2On();
		led3On();
	}else if(sLed.ledCount==250){
		led4On();
	}else if(sLed.ledCount==310){
		led1On();
		sLed.pwmStatu[1]=PWM_FALL;
		sLed.pwmStatu[2]=PWM_FALL;
	}else if(sLed.ledCount==400){
		sLed.pwmStatu[0]=PWM_FALL;
		sLed.pwmStatu[3]=PWM_FALL;
	}

	if(sLed.pwmStatu[0]==PWM_FALL){
		if(ledFall(0,PWM50/800))
			sLed.pwmStatu[0]=PWM_HOLD;
	}
	if(sLed.pwmStatu[1]==PWM_FALL){
		if(ledFall(1,PWM50/800))
			sLed.pwmStatu[1]=PWM_HOLD;
	}
	if(sLed.pwmStatu[2]==PWM_FALL){
		if(ledFall(2,PWM50/800))
			sLed.pwmStatu[2]=PWM_HOLD;
	}
	if(sLed.pwmStatu[3]==PWM_FALL){
		if(ledFall(3,PWM50/800))
			sLed.pwmStatu[3]=PWM_HOLD;
	}
}

void led_breathe(void)
{
	if(sLed.ledCount<3500){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	switch(sLed.ledCount){
		case 1:
			sLed.pwmStatu[0]=PWM_RISE;
			sLed.pwmStatu[1]=PWM_RISE;
			sLed.pwmStatu[2]=PWM_RISE;
			sLed.pwmStatu[3]=PWM_RISE;
		break;

		case 1500:
			sLed.pwmStatu[0]=PWM_FALL;
			sLed.pwmStatu[1]=PWM_FALL;
			sLed.pwmStatu[2]=PWM_FALL;
			sLed.pwmStatu[3]=PWM_FALL;
		break;

		case 3000:
			ledAllOff();
		break;
	}

	if(sLed.pwmStatu[0]==PWM_RISE){
		if(ledRise(0,PWM50/1500))
			sLed.pwmStatu[0]=PWM_HOLD;
		ledRise(1,PWM50/1500);
		ledRise(2,PWM50/1500);
		ledRise(3,PWM50/1500);
	}else if(sLed.pwmStatu[0]==PWM_FALL){
		if(ledFall(0,PWM50/1500))
			sLed.pwmStatu[0]=PWM_HOLD;
		ledFall(1,PWM50/1500);
		ledFall(2,PWM50/1500);
		ledFall(3,PWM50/1500);
	}
}

void led_mq(void)
{
	if(sLed.ledCount<1300){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	switch(sLed.ledCount){
		case 1:
			sLed.pwmStatu[0]=PWM_RISE;
		break;
		case 166:
			sLed.pwmStatu[1]=PWM_RISE;
		break;
		case 333:
			sLed.pwmStatu[2]=PWM_RISE;
		break;
		case 499:
			sLed.pwmStatu[3]=PWM_RISE;
		break;
		case 100:
			sLed.pwmStatu[0]=PWM_FALL;
		break;
		case 266:
			sLed.pwmStatu[1]=PWM_FALL;
		break;
		case 433:
			sLed.pwmStatu[2]=PWM_FALL;
		break;
		case 599:
			sLed.pwmStatu[3]=PWM_FALL;
		break;
		case 1000:
			ledAllOff();
		break;
	}

	if(sLed.pwmStatu[0]==PWM_RISE){
		if(ledRise(0,PWM50/90))
			sLed.pwmStatu[0]=PWM_HOLD;
	}else if(sLed.pwmStatu[0]==PWM_FALL){
		if(ledFall(0,PWM50/400))
			sLed.pwmStatu[0]=PWM_HOLD;
	}

	if(sLed.pwmStatu[1]==PWM_RISE){
		if(ledRise(1,PWM50/90))
			sLed.pwmStatu[1]=PWM_HOLD;
	}else if(sLed.pwmStatu[1]==PWM_FALL){
		if(ledFall(1,PWM50/400))
			sLed.pwmStatu[1]=PWM_HOLD;
	}

	if(sLed.pwmStatu[2]==PWM_RISE){
		if(ledRise(2,PWM50/90))
			sLed.pwmStatu[2]=PWM_HOLD;
	}else if(sLed.pwmStatu[2]==PWM_FALL){
		if(ledFall(2,PWM50/400))
			sLed.pwmStatu[2]=PWM_HOLD;
	}

	if(sLed.pwmStatu[3]==PWM_RISE){
		if(ledRise(3,PWM50/90))
			sLed.pwmStatu[3]=PWM_HOLD;
	}else if(sLed.pwmStatu[3]==PWM_FALL){
		if(ledFall(3,PWM50/400))
			sLed.pwmStatu[3]=PWM_HOLD;
	}
}

void led_flashAll(void)
{
	if(sLed.ledCount<250){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	if(sLed.ledCount==1){
		led1On();
		led2On();
		led3On();
		led4On();
	}else if(sLed.ledCount==125){
		ledAllOff();
	}
}

void led_swing(void)
{
	if(sLed.ledCount<1000){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	if(sLed.ledCount==1){
		led1On();
		led3On();
		led2Off();
		led4Off();
	}else if(sLed.ledCount==500){
		led2On();
		led4On();
		led1Off();
		led3Off();
	}
}

void led_power(void)
{
	if(sLed.ledCount<1000){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}

	if(sLed.ledCount==1){
		led1On();
		led2On();
		led3On();
	}else if(sLed.ledCount==500){
		led3Off();
	}
}

void led_random(void)
{
	if(sLed.ledCount<1100){
		sLed.ledCount++;
	}else{
		sLed.ledCount=0;
	}	

	switch(sLed.ledCount)
	{
		case 1:
			led1Off();
			led4On();
		break;
		case 125:
			led4Off();
			led2On();
		break;
		case 250:
			led2Off();
			led1On();
		break;
		case 375:
			led1Off();
			led3On();
		break;
		case 500:
			led3Off();
			led1On();
		break;
		case 625:
			led1Off();
			led2On();
		break;
		case 750:
			led2Off();
			led4On();
		break;
		case 875:
			led4Off();
			led3On();
		break;
		case 1000:
			led3Off();
			led1On();
		break;
	}
}


void pwmMClkOn(void)
{
	TMMK00 = 0U;
	TS0 |= _0001_TAU_CH0_START_TRG_ON | _0002_TAU_CH1_START_TRG_ON | _0004_TAU_CH2_START_TRG_ON |
           _0008_TAU_CH3_START_TRG_ON | _0010_TAU_CH4_START_TRG_ON;
}
void pwmMClkOff(void)
{
	TT0 |= _0001_TAU_CH0_STOP_TRG_ON | _0002_TAU_CH1_STOP_TRG_ON | _0004_TAU_CH2_STOP_TRG_ON |
           _0008_TAU_CH3_STOP_TRG_ON | _0010_TAU_CH4_STOP_TRG_ON;
}

