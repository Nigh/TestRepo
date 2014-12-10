

#define uchar unsigned char
#define uint unsigned int

#define STEP_N1 (50)
#define STEP_N2 (25*60)
#define STEP_N3 (300)

#define S1 (0x170)	//判断阈值，默认值适用于机测75rpm-180rpm
#define S2 (0xa0)
#define S3 (0x170)
#define S4 (0xa0)

static int vSTEP_N1=STEP_N1,vSTEP_N2=STEP_N2,vSTEP_N3=STEP_N3;
static int vcS1=S1,vcS2=S2,vcS3=S3,vcS4=S4;

int _setTimeout(int n1,int n2,int n3){
	vSTEP_N1=n1;
	vSTEP_N2=n2;
	vSTEP_N3=n3;
	return 0;
}

int _setThreshold(int s1,int s2,int s3,int s4){
	vcS1=s1;
	vcS2=s2;
	vcS3=s3;
	vcS4=s4;
	return 0;
}

#define STEP_TIMEOUT1 (7)	//超时时间，应当设置约为每秒采样数的1/3
#define STEP_TIMEOUT2 (6)	//应当设置为略小于TIMEOUT1
// #define STEP_TIMEOUT3

#define MAXACC (32767)
#define MINACC (-32767)

#define S5 (0x18)

#define DETECTRISE1 1
#define DETECTRISE2 2
#define DETECTFALL1 3
#define DETECTFALL2 4
int _calcStep(char *sAxis,uchar ctr)	//ctr=0 normal; ctr=1 9steps; ctr=2 sleepClear
{
	static char sSleepAxis[3]={0,0,0};
	static int minAcceleration=MAXACC,maxAcceleration=MINACC,compareAcc;
	static uchar stepCounter=0;
	static uint timeCounter=0,sleepCounter=0;
	// 频谱分析，离散处理
	static char stepInterval=0,avrInterval=0;
	static uint vS1=S1,vS2=S2,vS3=S3,vS4=S4;

	static int sAxisSqEx[4]={0,0,0,0},inA=0;
	static float K=0.8;

	static uchar stepFlag=DETECTRISE1;	//DETECTRISE1,DETECTRISE2,DETECTFALL1,DETECTFALL2
	static uchar count=0;

	#ifdef STEP_DEBUG
	static char t_stepPtr=0,t_stepInterval[9]={0};
	#endif

	register int temp0;
	int temp1,temp2;
	char *psAxis=&sAxis,*psSleepAxis=&sSleepAxis;
	uchar deltaA;	//与上次采样相比三轴的变化量之和
	char output=0;
	int sAxisSq=0;

	if(ctr==2){
		sleepCounter=0;
		timeCounter=0;
		stepFlag=DETECTRISE1;
	}


	// 不使用deltaA时屏蔽
	// temp0=*psSleepAxis++-*psAxis++;
	// if(temp0<0) temp0=~temp0;
	// temp1=*psSleepAxis++-*psAxis++;
	// if(temp1<0) temp1=~temp1;
	// temp2=*psSleepAxis-*psAxis;
	// if(temp2<0) temp2=~temp2;

	// deltaA=temp0+temp1+temp2;


	stepInterval++;
	psAxis=sAxis;
	temp0=(uint)(*psAxis)*(*psAxis++);
	temp0+=(uint)(*psAxis)*(*psAxis++);
	temp0+=(uint)(*psAxis)*(*psAxis);

	inA=inA*0.75+temp0*0.25;

	sAxisSqEx[count]=K*(2*sAxisSqEx[(count+3)&0x3]-sAxisSqEx[(count+2)&0x3])+(1-K)*(inA);

	sAxisSq = (sAxisSqEx[0]/4+sAxisSqEx[1]/4+sAxisSqEx[2]/4+sAxisSqEx[3]/4);

	count=(count+1)&0x3;

	switch(stepFlag){
		case DETECTRISE1:
			if(minAcceleration>sAxisSq){
				minAcceleration=sAxisSq;
			}else{
				timeCounter++;
				if(sAxisSq-minAcceleration>vS1){
					compareAcc=sAxisSq;
					stepFlag=DETECTRISE2;
				}
			}
		break;

		case DETECTRISE2:
			if(sAxisSq-compareAcc>vS2){
				stepFlag=DETECTFALL1;
				maxAcceleration=MINACC;
				timeCounter=0;
			}else{
				if(timeCounter++>STEP_TIMEOUT1){
					stepFlag=DETECTRISE1;
					minAcceleration=MAXACC;
				}
			}
		break;

		case DETECTFALL1:
			if(sAxisSq>maxAcceleration){
				maxAcceleration=sAxisSq;
			}else{
				if(sAxisSq<maxAcceleration-vS3){
					compareAcc=sAxisSq;
					stepFlag=DETECTFALL2;
					timeCounter=0;
				}else{
					if(timeCounter++>STEP_TIMEOUT1){
						stepFlag=DETECTRISE1;
						minAcceleration=MAXACC;
					}
				}
			}
		break;

		case DETECTFALL2:
			if(sAxisSq<compareAcc-vS4){	//step count 1 !!
				stepFlag=DETECTRISE1;
				minAcceleration=MAXACC;
				timeCounter=0;
				sleepCounter=0;
				if(ctr==1){
					stepCounter++;

					#ifdef STEP_DEBUG
					t_stepPtr=stepCounter-3;
					#endif

					if(stepCounter==3){
						avrInterval=stepInterval;
						#ifdef STEP_DEBUG
						t_stepInterval[stepCounter-3]=avrInterval;
						#endif
					}else if(stepCounter>3){
						temp0=stepInterval-avrInterval;
						#ifdef STEP_DEBUG
						t_stepInterval[stepCounter-3]=stepInterval;
						#endif
						switch(avrInterval){
							case 6:if(temp0<-1 || temp0>2)stepCounter=0; break;
							case 7:if(temp0<-1 || temp0>2)stepCounter=0; break;
							case 8:if(temp0<-1 || temp0>3)stepCounter=0; break;
							case 9:if(temp0<-1 || temp0>4)stepCounter=0; break;
							case 10:if(temp0<-2 || temp0>4)stepCounter=0; break;
							case 11:if(temp0<-2 || temp0>4)stepCounter=0; break;
							case 12:if(temp0<-3 || temp0>4)stepCounter=0; break;
							case 13:if(temp0<-3 || temp0>5)stepCounter=0; break;
							case 14:if(temp0<-3 || temp0>6)stepCounter=0; break;
							case 15:if(temp0<-4 || temp0>5)stepCounter=0; break;
							case 16:if(temp0<-4 || temp0>5)stepCounter=0; break;
							case 17:if(temp0<-4 || temp0>5)stepCounter=0; break;

							case 18:if(temp0<-4 || temp0>5)stepCounter=0; break;
							case 19:if(temp0<-5 || temp0>4)stepCounter=0; break;
							case 20:if(temp0<-6 || temp0>3)stepCounter=0; break;
							case 21:if(temp0<-7 || temp0>3)stepCounter=0; break;
							case 22:if(temp0<-8 || temp0>2)stepCounter=0; break;
							case 23:if(temp0<-9 || temp0>1)stepCounter=0; break;
							case 24:if(temp0<-10 || temp0>0)stepCounter=0; break;

							default:
							if(stepCounter>0)
								stepCounter-=1;
							break;
						}

						if(stepInterval<6 || stepInterval>24)
							stepCounter=0;	//band pass filter

					}

					#ifdef STEP_DEBUG
					if(stepCounter==0){
						output=0;
					}
					#endif

					if(stepCounter>0){
						if(stepInterval<=8)
							K=0.6;
						else if(stepInterval<=10)
							K=0.7;
						else if(stepInterval<=12)
							K=0.75;
						else
							K=0.8;
					}

					if(stepCounter>=9){
						stepCounter=0;
						output=9;
					}
				}else if(ctr==0){
					if(stepInterval>=6){
						output=1;
						if(stepInterval<=8)
							K=0.6;
						else if(stepInterval<=10)
							K=0.7;
						else if(stepInterval<=12)
							K=0.75;
						else
							K=0.8;
					}
				}
				// avrInterval=avrInterval*0.75+stepInterval*0.25;
				avrInterval=stepInterval;
				if(avrInterval>=15){vS1=vcS1,vS2=vcS2,vS3=vcS3,vS4=vcS4;}	//<80/min
				else if(avrInterval>=12){vS1=vcS1+0x30,vS2=vcS2+0x30,vS3=vcS3+0x30,vS4=vcS4+0x30;}	//<100/min
				else if(avrInterval>=10){vS1=vcS1+0x80,vS2=vcS2+0x60,vS3=vcS3+0x80,vS4=vcS4+0x60;}	//<120/min
				else if(avrInterval>=8){vS1=vcS1+0x110,vS2=vcS2+0xE0,vS3=vcS3+0x110,vS4=vcS4+0xE0;}	//<140/min
				else {vS1=vcS1+0x220,vS2=vcS2+0x140,vS3=vcS3+0x220,vS4=vcS4+0x140;}

				stepInterval=0;
			}else{
				if(timeCounter++>STEP_TIMEOUT2){
					stepFlag=DETECTRISE1;
					minAcceleration=MAXACC;
				}
			}
		break;

		default:break;
	}

	sleepCounter++;
	if(sleepCounter==vSTEP_N1){
		vS1=vcS1,vS2=vcS2,vS3=vcS3,vS4=vcS4;
		stepFlag=DETECTRISE1;
		minAcceleration=0xff;
		stepCounter=0;
		output=-1;
		K=0.8;
	}
	if(timeCounter>vSTEP_N3 || sleepCounter>vSTEP_N2){
		output=-2;
		K=0.8;
	}

	return output;
}
