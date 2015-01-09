//#include "Clickpro.h"
#include "Clickpro.h"

#define CLICKFLAG_CLEAR_ALL (0x07)
#define CLICKFLAG_FILL_X (0x01)
#define CLICKFLAG_FILL_Y (0x02)
#define CLICKFLAG_FILL_Z (0x04)
#define CLICKFLAG_CLEAR_X (0x10)
#define CLICKFLAG_CLEAR_Y (0x20)
#define CLICKFLAG_CLEAR_Z (0x40)

static unsigned short ClickInvalidCnt = 0;

unsigned char IsClick(unsigned char *clickdata)
{
	static unsigned char CollectNum = 0;
	static unsigned char CurDataNum = 0;
	static char Xdifdata[8] = {0};
	static char Ydifdata[8] = {0};
	static char Zdifdata[8] = {0};
	static unsigned char XAbsDif[8] = {0};
	static unsigned char YAbsDif[8] = {0};
	static unsigned char ZAbsDif[8] = {0};
	static char XLastdata = 0;
	static char YLastdata = 0;
	static char ZLastdata = 0;
	static unsigned char XAbsData = 0;
	static unsigned char YAbsData = 0;
	static unsigned char ZAbsData = 0;
	static unsigned char XAbsData_2 = 0;
	static unsigned char YAbsData_2 = 0;
	static unsigned char ZAbsData_2 = 0;
	static char ClickInvalidTime = 0;
	static unsigned char StableTimes = 0;
	static unsigned ClickFlag = 0;			//bit0 0: 1: X Click fit;
											//bit1 0: 1: Y Click fit;
											//bit2 0: 1: Z Click fit;

											//bit4 0: 1: X Click fit flag should be clear
											//bit5 0: 1: Y Click fit flag should be clear
											//bit6 0: 1: Z Click fit flag should be clear
	unsigned char returedata = 0;
	unsigned char temp1 = 0;
	unsigned char temp2 = 0;
	unsigned char tempA = 0;
	unsigned char comparedata = 0;
	unsigned char comparedata1 = 0;

	if(ClickInvalidCnt < 300) ClickInvalidCnt++;
	
	Xdifdata[CurDataNum] = (unsigned char)((signed int)(*clickdata) - XLastdata);
	Ydifdata[CurDataNum] = (unsigned char)((signed int)(*(clickdata + 1)) - YLastdata);
	Zdifdata[CurDataNum] = (unsigned char)((signed int)(*(clickdata + 2)) - ZLastdata);

	XAbsData_2 = XAbsData;
	YAbsData_2 = YAbsData;
	ZAbsData_2 = ZAbsData;

	if(*clickdata & 0x80) XAbsData = (unsigned char)(*(clickdata) ^ 0xff) + 1;
	else XAbsData = (unsigned char)*(clickdata);

	if(*(clickdata + 1) & 0x80) YAbsData = (unsigned char)(*(clickdata + 1) ^ 0xff) + 1;
	else YAbsData = (unsigned char)*(clickdata + 1);

	if(*(clickdata + 2) & 0x80) ZAbsData = (unsigned char)(*(clickdata + 2) ^ 0xff) + 1;
	else ZAbsData = (unsigned char)*(clickdata + 2);

	if(Xdifdata[CurDataNum] & 0x80) XAbsDif[CurDataNum] = (unsigned char)(Xdifdata[CurDataNum] ^ 0xff) + 1;
	else XAbsDif[CurDataNum] = Xdifdata[CurDataNum];

	if(Ydifdata[CurDataNum] & 0x80) YAbsDif[CurDataNum] = (unsigned char)(Ydifdata[CurDataNum] ^ 0xff) + 1;
	else YAbsDif[CurDataNum] = Ydifdata[CurDataNum];

	if(Zdifdata[CurDataNum] & 0x80) ZAbsDif[CurDataNum] = (unsigned char)(Zdifdata[CurDataNum] ^ 0xff) + 1;
	else ZAbsDif[CurDataNum] = Zdifdata[CurDataNum];

	XLastdata = (signed char)(*clickdata);
	YLastdata = (signed char)(*(clickdata + 1));
	ZLastdata = (signed char)(*(clickdata + 2));
	if(((XAbsData_2 + XAbsData) > 253) ||
	   ((YAbsData_2 + YAbsData) > 253) ||
	   ((ZAbsData_2 + ZAbsData) > 253))
		ClickInvalidTime = 10;

	if((XAbsDif[CurDataNum] < 5) &&
	   (YAbsDif[CurDataNum] < 5) &&
	   (ZAbsDif[CurDataNum] < 5))		//in the stable state
	{
		StableTimes++;
		if(StableTimes > 14) {
			StableTimes = 15;
			ClickFlag = 0;
		}
		if(ClickInvalidTime) ClickInvalidTime--;
	} else {
		if(StableTimes < 4) StableTimes = 0;
		else StableTimes --;

		if(ClickInvalidTime) {
			ClickInvalidTime--;
			returedata = 0;
			goto	ActivityEndPro;
		}

		if(CurDataNum == 0) temp1= 7;
		else temp1 = CurDataNum - 1;

		if(temp1 == 0) temp2 = 7;
		else temp2 = temp1 - 1;

		if(((Xdifdata[CurDataNum] ^ Xdifdata[temp1]) & 0x80) == 0x80){		//direction different
			if(XAbsDif[CurDataNum] > XAbsDif[temp1]) tempA = XAbsDif[CurDataNum] - XAbsDif[temp1];
			else tempA = XAbsDif[temp1] - XAbsDif[CurDataNum];
			if(tempA < (XAbsDif[temp1] >> 1)){		//two difdata different in 1/2
				if(StableTimes > 5) comparedata = CLICKSTABLELIMIT_X;
				else comparedata = CLICKACTIVELIMIT_X;
				if(ClickInvalidCnt > 290)
					comparedata += FIRSTADD;
				if(XAbsDif[temp1] > comparedata){	//the indemitary data change fit the condition
					if((Xdifdata[temp1] ^ Xdifdata[temp2]) & 0x80){		//before
						ClickFlag |= 0x01;
						comparedata1 = CLICKABSOLUTE;
						if(ClickInvalidCnt > 290) comparedata1 = CLICKABSOLUTE + FIRSTADD;
						if(XAbsDif[temp1] > comparedata1) ClickFlag = CLICKFLAG_CLEAR_ALL;
						if(ClickFlag & 0x06) ClickFlag = CLICKFLAG_CLEAR_ALL;
					} else {
						if((XAbsDif[temp1] >> 2) > XAbsDif[temp2]) {
							ClickFlag |= 0x01;
							comparedata1 = CLICKABSOLUTE;
							if(ClickInvalidCnt > 290) comparedata1 = CLICKABSOLUTE + FIRSTADD;
							if(XAbsDif[temp1] > comparedata1) ClickFlag = CLICKFLAG_CLEAR_ALL;
							if(ClickFlag & 0x06) ClickFlag = CLICKFLAG_CLEAR_ALL;
						}
					}
				}
			}
		}


		if(((Ydifdata[CurDataNum] ^ Ydifdata[temp1]) & 0x80) == 0x80){		//direction different
			if(YAbsDif[CurDataNum] > YAbsDif[temp1]) tempA = YAbsDif[CurDataNum] - YAbsDif[temp1];
			else tempA = YAbsDif[temp1] - YAbsDif[CurDataNum];
			if(tempA < (YAbsDif[temp1] >> 1)){		//two difdata different in 1/2
				if(StableTimes > 5) comparedata = CLICKSTABLELIMIT_Y;
				else comparedata = CLICKACTIVELIMIT_Y;
				if(ClickInvalidCnt > 290) comparedata += FIRSTADD;
				if(YAbsDif[temp1] > comparedata){	//the indemitary data change fit the condition
				
					if((Ydifdata[temp1] ^ Ydifdata[temp2]) & 0x80){		//before
						ClickFlag |= 0x02;
						if(ClickFlag & 0x05)
							ClickFlag = CLICKFLAG_CLEAR_ALL;
						comparedata1 = CLICKABSOLUTE;
						if(ClickInvalidCnt > 290)
							comparedata1 = CLICKABSOLUTE + FIRSTADD;
						if(YAbsDif[temp1] > comparedata1)
							ClickFlag = CLICKFLAG_CLEAR_ALL;
					} else {
						if((YAbsDif[temp1] >> 2) > YAbsDif[temp2]) {
							ClickFlag |= 0x02;
							comparedata1 = CLICKABSOLUTE;
							if(ClickInvalidCnt > 290) comparedata1 = CLICKABSOLUTE + FIRSTADD;
							if(YAbsDif[temp1] > comparedata1) ClickFlag = CLICKFLAG_CLEAR_ALL;
							if(ClickFlag & 0x05) ClickFlag = CLICKFLAG_CLEAR_ALL;
						}
					}
				}
			}
		}

		if(((Zdifdata[CurDataNum] ^ Zdifdata[temp1]) & 0x80) == 0x80){		//direction different
			if(ZAbsDif[CurDataNum] > ZAbsDif[temp1]) tempA = ZAbsDif[CurDataNum] - ZAbsDif[temp1];
			else tempA = ZAbsDif[temp1] - ZAbsDif[CurDataNum];
			if(tempA < (ZAbsDif[temp1] >> 1)){		//two difdata different in 1/2
				if(StableTimes > 5)
					comparedata = CLICKSTABLELIMIT_Z;
				else
					comparedata = CLICKACTIVELIMIT_Z;
				if(ClickInvalidCnt > 290)
					comparedata += FIRSTADD;
				if(ZAbsDif[temp1] > comparedata)	//the indemitary data change fit the condition
				{
					if((Zdifdata[temp1] ^ Zdifdata[temp2]) & 0x80){		//before
						ClickFlag |= 0x04;
						comparedata1 = CLICKABSOLUTE;
						if(ClickInvalidCnt > 290) comparedata1 = CLICKABSOLUTE + FIRSTADD;
						if(ZAbsDif[temp1] > comparedata1) ClickFlag = CLICKFLAG_CLEAR_ALL;
						if(ClickFlag & 0x03) ClickFlag = CLICKFLAG_CLEAR_ALL;
					} else {
						if((ZAbsDif[temp1] >> 2) > ZAbsDif[temp2]) {
							ClickFlag |= 0x04;
							comparedata1 = CLICKABSOLUTE;
							if(ClickInvalidCnt > 290) comparedata1 = CLICKABSOLUTE + FIRSTADD;
							if(ZAbsDif[temp1] > comparedata1) ClickFlag = CLICKFLAG_CLEAR_ALL;
							if(ClickFlag & 0x03) ClickFlag = CLICKFLAG_CLEAR_ALL;
						}
					}
				}
			}
		}

		if(StableTimes > 10) {
			if(ClickFlag) {
				ClickInvalidTime = 7;
				ClickFlag = 0;
				returedata = 1;
				ClickInvalidCnt = 0;
			}
		} else {
			if((ClickFlag & CLICKFLAG_CLEAR_ALL) == CLICKFLAG_CLEAR_ALL) {
				ClickInvalidTime = 5;
				ClickFlag = 0;
				returedata = 1;
				ClickInvalidCnt = 0;
			} else {
				if(ClickFlag & CLICKFLAG_CLEAR_X) ClickFlag &= (0xFF^CLICKFLAG_FILL_X);
				if(ClickFlag & CLICKFLAG_CLEAR_Y) ClickFlag &= (0xFF^CLICKFLAG_FILL_Y);
				if(ClickFlag & CLICKFLAG_CLEAR_Z) ClickFlag &= (0xFF^CLICKFLAG_FILL_Z);
				ClickFlag |= (ClickFlag << 4);
			}
		}
	}

ActivityEndPro:

	CurDataNum ++;
	if(CurDataNum == 8)
		CurDataNum = 0;

	return returedata;
}

// void strncpy(char *to,char *from,int count)
// {
// 	int n=(count+7)/8;
// 	switch (count%8){
// 	case 0: do{*to=*from++;
// 	case 7: *to = *from++;
// 	case 6: *to = *from++;
// 	case 5: *to = *from++;
// 	case 4: *to = *from++;
// 	case 3: *to = *from++;
// 	case 2: *to = *from++;
// 	case 1: *to = *from++;
// 			}while(--n>0);
// 	}
// }

#define X_WEIGHT (16)
#define Y_WEIGHT (16)
#define Z_WEIGHT (16)

#define isDiffSign(X,Y) ((X^Y)<0)
#define iDiff(X,Y) (X>Y?(X-Y):(Y-X))
#define iAbs(X) (X>0?(X):(-X))

#define CLICK_IDLE (0)
#define CLICK_IMPULSE_START (1)
// #define CLICK_WAIT_IMPULSE_END (2)
#define CLICK_WAIT_STATIC (3)
#define CLICK_SLEEP (4)
unsigned char isClickEx(char *array)
{
	static int base[3]={0};
	static int sArray[3]={0},amplitudeForward[3]={0},amplitudeBackward[3]={0};
	static int amplitudeForwardArray[3]={0};
	static int amplitudeForwardSum=0,amplitudeBackwardSum=0;
	static char sleep=50,staticCount=0;
	static char statu=CLICK_SLEEP;
	int i,isClick=0;
	unsigned int impulseSum,impulseCount=0;
	for(i=0;i<3;i++){
		base[i]=base[i]-(base[i]/8);	//base[i]=7/8*(base[i])+1/8*(sArray[i])
		base[i]+=array[i];
		sArray[i]=(int)array[i]*8;
	}
	// sArray -512~511
	if(statu==CLICK_SLEEP){
		if(sleep-->0)
			return isClick;
		else
			statu=CLICK_IDLE;
	}

	if(statu==CLICK_IMPULSE_START){
		statu=CLICK_IDLE;
		amplitudeBackwardSum=0;
		for(i=0;i<3;i++){
			amplitudeBackward[i]=sArray[i]-amplitudeForwardArray[i];
			amplitudeBackwardSum+=iAbs(amplitudeBackward[i]);
		}
		if(amplitudeBackwardSum>200){
			impulseSum=0;
			for(i=0;i<3;i++){
				if(i==1) continue;	//y-axis ignore
				if(isDiffSign(amplitudeForward[i],amplitudeBackward[i])){
					impulseCount++;
					impulseSum+=iDiff(amplitudeForward[i],amplitudeBackward[i]);
				}
			}
			if(staticCount>6 && impulseCount>=2 && impulseSum>460){
				isClick=1;
				staticCount=0;
				return isClick;
			}else{
				sleep=3;
				staticCount=0;
				statu=CLICK_SLEEP;
			}
		}
	}
	//calculate the amplitude of 3-axis
	if(staticCount<50) staticCount++;
	amplitudeForwardSum=0;
	for(i=0;i<3;i++){
		amplitudeForwardArray[i]=sArray[i];
		amplitudeForward[i]=amplitudeForwardArray[i]-base[i];
		amplitudeForwardSum+=iAbs(amplitudeForward[i]);
	}
	if(statu==CLICK_IDLE && amplitudeForwardSum>240){
		statu=CLICK_IMPULSE_START;
	}

	return isClick;
}
