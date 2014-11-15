#ifndef   __LIS3DH_H_
#define   __LIS3DH_H_

#define StepUp1                     125//125            
#define StepUp2                     55//55
#define StepDown1                   110//115
#define StepDown2                   45//45

extern unsigned char       Stream_cnt;
extern unsigned char       Calorio_SecCount;
extern unsigned char       Calorio_StepCount;

typedef struct
{
    unsigned char     Enable:1;
    unsigned char     Run:1;
    unsigned char     up:1;
    unsigned char     down:1;
    unsigned char     start:1;
    unsigned char     ninestep:1;
    unsigned char     _47ms:1;
    unsigned char     sporttime:1;
    unsigned char     faststep:1;
    unsigned char     finishstep:1;
} __Lis3DStepFlag;

extern __Lis3DStepFlag  Lis3DStepFlag;

extern unsigned char       Stream_cnt;

extern unsigned int X_axis[4];
extern unsigned int Y_axis[4];
extern unsigned int Z_axis[4];

extern unsigned char StepInternalTimer[12];
extern unsigned char DifferenceTime[12];

extern unsigned int StepRange[12];

extern unsigned int x2y2z2[5];

extern unsigned char ReadDataCount;
extern unsigned char CompareCount;
extern unsigned char ReplaceCount;
extern unsigned char NineStepCount;
extern unsigned char Count_46_7ms_Lis3dh;

extern unsigned char BiggerCount;
extern unsigned char Lis3dh_15msCount;

extern unsigned int CompareData;
extern unsigned int MaxAcceleration, MinAcceleration;

extern unsigned char InvalidTime;

extern unsigned int Lis3D_Step;

unsigned char CalculateStep(unsigned char xyzData[3]);

#endif
