
#ifndef STEP_H_
#define STEP_H_


/***********
*input:array[3]={X,Y,Z}
*		ctr=0 普通计步，ctr=1 9步采样 ctr=2 sleepCounterClear
*return:
	0:无计步
	1:记1步
	9:记9步
	-1: N1 次采样内无计步发生
	-2: N2 次采样内无计步发生 或 N3 次采样内无运动
*
************/


/**************************
函数说明:
	函数原型:int calcStep(char *ptr,uchar flag);
	第一个参数为包含三轴的值的结构指针，无论是用结构体还是数组，应按照x,y,z顺序紧密排列。
	读取数据时会读取ptr,ptr+1,ptr+2三个地址的值分别作为x,y,z三轴的采样值。
	三轴的采样值必须为char类型。

	*** 适用于2G，采样率为20Hz左右时(为读取速率，非传感器采样速率)。 ***

	当flag为0时，为普通计步，每计到一步，返回1，否则返回0
	当flag为1时，为9步计步，计满合格9步，返回9，否则返回0
	当flag为2时，清除内置超时计数器

	当 N1 次采样内无计步发生时，函数返回-1
	一般用作 2s 无计步空闲识别
	(只在 N1 次时返回一次，区别于下面)

	当大于 N2 次采样内无计步发生，或大于 N3 次采样内无运动时，返回-2
	一般用作睡眠识别
	(不清除将一直返回-2)

**************************/
int _calcStep(char*,unsigned char);

/**************************
函数说明:
	以下函数用于配置计步参数，当未调用时，保持默认值；

	参数设置:
		默认值:
			s1=0x1a0
			s2=0xa0
			s3=0x1a0
			s4=0xa0
			N1=(50)
			N2=(25*60)
			N3=(300)

	其中s1-s4为计步阈值，越大，则计步判定越严格，即越不容易计步。反之则越宽松。
	N1-N3见前述。
**************************/
int _setThreshold(int s1,int s2,int s3,int s4);
int _setTimeout(int N1,int N2,int N3);

#endif /* STEP_H_ */
