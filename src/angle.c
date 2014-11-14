#include "angle.h"

float absf(float f)
{
	return f>0?f:-f;
}

int absi(int f)
{
	return f>0?f:-f;
}

float userATan(float eta)	//0~pi/4	ERR<=2%
{
	eta=absf(eta);
	return eta-(eta*eta*eta)*0.225;
}

float axisTan(int k,int m)
{
	float kAbs=absi(k),mAbs=absi(m),temp;
	if(k==0 && k==m)
        return 0;
	if(kAbs>mAbs){
		temp=(float)(mAbs/kAbs);
		temp=userATan(temp);
		if(m<0){
			if(k<0){
				temp=PI/2-temp;
			}else{
				temp=temp-PI/2;
			}
		}else{
			if(k<0){
				temp=PI/2+temp;
			}else{
				temp=-temp-PI/2;
			}
		}
	}else{
		temp=(float)kAbs/mAbs;
		temp=userATan(temp);
		if(m<0){
			if(k>0){
				temp=-temp;
			} // else{temp=-temp;}
		}else{
			if(k<0){
				temp=PI-temp;
			}else{
				temp=temp-PI;
			}
		}
	}
	return temp;
}

tEULER* calcRulerA(sGACC* sGAcc)
{
	static tEULER tEular;
	//float xAbs=absf(sGAcc->x),yAbs=absf(sGAcc->y),zAbs=absf(sGAcc->z);
	float temp;
	tEular.Pitch=axisTan(sGAcc->x,sGAcc->z);
	tEular.Roll=axisTan(sGAcc->y,sGAcc->z);
	tEular.Yaw=axisTan(sGAcc->y,sGAcc->x);
	return &tEular;
}

