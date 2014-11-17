#include "apptimer.h"

// sAPPTIMER stopBuzzerTimer={0,0,&stopBuzzer};
sAPPTIMER stopVibrateTimer={0,0,&stopVibrate};
// sAPPTIMER i2cWaitTimer={0,0,&_nop_Ex};
// sAPPTIMER handWaitTimer={0,0,&_nop_Ex};
// sAPPTIMER BLEResetTimer={0,0,&_nop_Ex};

sTIMERTASK sTimerTask={0};

void stopVibrate(void)
{
	stopVibrateTimer.en=0;
	// sVibrate.en=0;
	VibrateOff();
}



// possible improvement:task至少应包含自身的函数指针以及一个状态标记
// 以便在insert task时作为判断，则可选择相同task是否能在一次调用中
// 被多次执行
// 目前采用各task中各资源的状态标记来间接实现此功能
void taskInsert(iFUNC* task)	// 代码量12
{
	sTimerTask.tArray[sTimerTask.maxIndex]=task;
	sTimerTask.maxIndex++;
}

void taskDelete(uint ptr)	// 代码量23
{
	register iFUNC** _array=sTimerTask.tArray;
	register uint _temp=ptr,*_max=&sTimerTask.maxIndex;
	_array+=_temp;
	while(_temp++<*_max)
		*_array++=*(_array+1);
	(*_max)--;
}

uchar apptimerTaskEn=0;
int apptimerTask(void)
{
	uchar refCount=0;
	refCount+=timer(&stopVibrateTimer);
	apptimerTaskEn=refCount;
	return refCount;
}

// int vibrateTask(void)
// {
// 	static uchar vibrate32HzCount=0;
// 	if(sVibrate.en==1){
// 		if(vibrate32HzCount>=sVibrate.array[sVibrate.count]) {
// 			if(sVibrate.count%2==0 && sVibrate.count!=sVibrate.length-1){
// 				VibrateOn();
// 			} else {
// 				VibrateOff();
// 			}
// 			sVibrate.count=(sVibrate.count+1)%sVibrate.length;
// 			vibrate32HzCount=0;
// 		}
// 		vibrate32HzCount++;
// 		return 1;
// 	}
// 	vibrate32HzCount=0;
// 	return 0;
// }

void setTimer64Hz(sAPPTIMER* apptimer,uint period)
{
	if(apptimerTaskEn==0){
		taskInsert(&apptimerTask);
		apptimerTaskEn=1;
	}
	apptimer->count=period;
	apptimer->en=1;	
	R_TAU0_Channel6_Start();
}

void functionX(void)
{
	return;
}

uchar timer(sAPPTIMER* apptimer)
{
	if(apptimer->en==1)	//en
	{
		apptimer->count--;
		if(apptimer->count==0){	//count
			apptimer->en=0;	//en
			return 1;
		}
	}
	return 0;
}
