#ifndef _ANGLE_H_
#define _ANGLE_H_

#include "NeckAlgorithm.h"

#define PI (3.141592653589793)
typedef struct
{
	int x;
	int y;
	int z;
} sGACC;

// typedef struct
// {
// 	float Roll;
// 	float Pitch;
// 	float Yaw;
// } tEULER;

tEULER* calcRulerA(sGACC* sGAcc);

#endif
