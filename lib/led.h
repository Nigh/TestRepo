#ifndef _LED_H_
#define _LED_H_
#include "r_cg_userdefine.h"
#include "typedef.h"
#include "r_cg_macrodriver.h"

#define OSC_FREQ (8000)
#define PWM50 (OSC_FREQ/2)

#define LED_M_OFF		(0xffff)
#define LED_M_HERATBEAT (0)
#define LED_M_BREATHE	(1)
#define LED_M_MQ		(2)
#define LED_M_FLASHALL	(3)
#define LED_M_SWING		(4)
#define LED_M_POWER		(5)
#define LED_M_RANDOM	(6)
#define LED_M_STATICPOWER	(7)

#define LED_M_MAX	(7)	//max mode num

#define PWM_RISE (1)
#define PWM_FALL (2)
#define PWM_HOLD (0)

#define BAT80 (75)
#define BAT60 (50)
#define BAT40 (25)
#define BAT20 (20)
#define BAT0 (0)

// #define led1Off() *(_PWM[0])=0,TOE0&=0xffff^_0002_TAU_CH1_OUTPUT_ENABLE,TO0 |= _0002_TAU_CH1_OUTPUT_VALUE_1
// #define led2Off() *(_PWM[1])=0,TOE0&=0xffff^_0004_TAU_CH2_OUTPUT_ENABLE,TO0 |= _0004_TAU_CH2_OUTPUT_VALUE_1
// #define led3Off() *(_PWM[2])=0,TOE0&=0xffff^_0008_TAU_CH3_OUTPUT_ENABLE,TO0 |= _0008_TAU_CH3_OUTPUT_VALUE_1
// #define led4Off() *(_PWM[3])=0,TOE0&=0xffff^_0010_TAU_CH4_OUTPUT_ENABLE,TO0 |= _0010_TAU_CH4_OUTPUT_VALUE_1

// #define led1On() TOE0|=_0002_TAU_CH1_OUTPUT_ENABLE
// #define led2On() TOE0|=_0004_TAU_CH2_OUTPUT_ENABLE
// #define led3On() TOE0|=_0008_TAU_CH3_OUTPUT_ENABLE
// #define led4On() TOE0|=_0010_TAU_CH4_OUTPUT_ENABLE

#define led1Off() *(_PWM[0])=0
#define led2Off() *(_PWM[1])=0
#define led3Off() *(_PWM[2])=0
#define led4Off() *(_PWM[3])=0

#define led1On() *(_PWM[0])=PWM50
#define led2On() *(_PWM[1])=PWM50
#define led3On() *(_PWM[2])=PWM50
#define led4On() *(_PWM[3])=PWM50

extern void ledAllOff(void);
extern void ledNext(void);
typedef struct
{
	uint pwmDC[4];		//duty cycle
	uint ledCount;	//1000hz count
	uint ledMode;
	uchar pwmStatu[4];
	uint times;	//execute times (1~16) (0xff for infinite loop)
} sLED;
extern sLED sLed;


void ledSetMode(uint,uint);

void led_heartBeat(void);
void led_breathe(void);
void led_mq(void);
void led_flashAll(void);
void led_swing(void);
void led_power(void);
void led_random(void);
void led_staticPower(void);

#endif
