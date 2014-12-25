#ifndef __NECK_ALGORITHM_H__
#define __NECK_ALGORITHM_H__

#include "time.h"
//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
extern time_t time2(void);
/*********************************************************************
 * TYPEDEFS
 */
typedef enum 
{
    HEAD_DOWN   = 0x01,
    HEAD_UP     = 0x02,
    FACE_LEFT   = 0x04,
    FACE_RIGHT  = 0x08,
    HEAD_LEFT   = 0x10,
    HEAD_RIGHT  = 0x20,
    
    HEAD_DOWN_FACE_LEFT  = HEAD_DOWN | FACE_LEFT,
    HEAD_DOWN_FACE_RIGHT = HEAD_DOWN | FACE_RIGHT,
    HEAD_DOWN_HEAD_LEFT  = HEAD_DOWN | HEAD_LEFT,
    HEAD_DOWN_HEAD_RIGHT = HEAD_DOWN | HEAD_RIGHT,
    HEAD_DOWN_FACE_LEFT_HEAD_LEFT   = HEAD_DOWN | FACE_LEFT  | HEAD_LEFT,
    HEAD_DOWN_FACE_LEFT_HEAD_RIGHT  = HEAD_DOWN | FACE_LEFT  | HEAD_RIGHT,
    HEAD_DOWN_FACE_RIGHT_HEAD_LEFT  = HEAD_DOWN | FACE_RIGHT | HEAD_LEFT,
    HEAD_DOWN_FACE_RIGHT_HEAD_RIGHT = HEAD_DOWN | FACE_RIGHT | HEAD_RIGHT,
    
    HEAD_UP_FACE_LEFT  = HEAD_UP | FACE_LEFT,
    HEAD_UP_FACE_RIGHT = HEAD_UP | FACE_RIGHT,
    HEAD_UP_HEAD_LEFT  = HEAD_UP | HEAD_LEFT,
    HEAD_UP_HEAD_RIGHT = HEAD_UP | HEAD_RIGHT,
    HEAD_UP_FACE_LEFT_HEAD_LEFT   = HEAD_UP | FACE_LEFT  | HEAD_LEFT,
    HEAD_UP_FACE_LEFT_HEAD_RIGHT  = HEAD_UP | FACE_LEFT  | HEAD_RIGHT,
    HEAD_UP_FACE_RIGHT_HEAD_LEFT  = HEAD_UP | FACE_RIGHT | HEAD_LEFT,
    HEAD_UP_FACE_RIGHT_HEAD_RIGHT = HEAD_UP | FACE_RIGHT | HEAD_RIGHT,
    
    FACE_LEFT_HEAD_LEFT  = FACE_LEFT | HEAD_LEFT,
    FACE_LEFT_HEAD_RIGHT = FACE_LEFT | HEAD_RIGHT,
    
    FACE_RIGHT_HEAD_LEFT  = FACE_RIGHT | HEAD_LEFT,
    FACE_RIGHT_HEAD_RIGHT = FACE_RIGHT | HEAD_RIGHT,
}eNECK_ACTIVITY_ID;


typedef struct
{
    float Pitch;
    float Roll;
    float Yaw;
}tfMOVEMENT;

typedef struct
{
    unsigned short Pitch;
    unsigned short Roll;
    unsigned short Yaw;
}tiMOVEMENT;

typedef struct
{
    unsigned char Pitch;
    unsigned char Roll;
    unsigned char Yaw;
}tcMOVEMENT;

typedef struct
{
    char PitchPostive;
    char PitchNegative;
    char RollPostive;
    char RollNegative;
    char YawPostive;
    char YawNegative;
}tTIME_PERCENT;


typedef struct
{
    char          MoveID;      //1 byte
    char          PositionID;  //1 byte
    tTIME_PERCENT TimePercent; //6 byte
    tiMOVEMENT    Movement;    //6 byte
    time_t        StartTime;   //4 byte
}tNECK;

typedef struct
{
    unsigned short Positive;
    unsigned short Zero;
    unsigned short Negative;
}tPOS_TIMER;

typedef struct
{
    unsigned char Positive;
    unsigned char Zero;
    unsigned char Negative;
}tcPOS_TIMER;


typedef struct
{
    tcPOS_TIMER    HeadUpDownTmr;     //3
    tcPOS_TIMER    HeadLeftRightTmr;  //3
    tiMOVEMENT     Movement;          //6
    time_t         StartTime;         //4
}tNECK_LOG;

/*



// BLE Service Log
typedef struct
{
    tPOS_TIMER     HeadUpDownTmr;     //6
    tiMOVEMENT     Movement;          //6
    time_t         StartTime;         //4
}tNECK_LOG_PART1;

typedef struct
{
    tPOS_TIMER     HeadLeftRightTmr;  //6
    time_t         StartTime;         //4
}tNECK_LOG_PART2;

//BLE Service Log End
*/

typedef struct
{
    char           PositionID;     //1
    time_t         StartTime;      //4
}tAlarm;


typedef struct
{
    float Roll;
    float Pitch;
    float Yaw;
}tEULER;

int NeckActivityAlgorithm(tEULER* pNewEuler, tNECK* pNeckData);
const char* NeckID2String(char ID);

unsigned char GetNeckLogPeriod(void);
void SetNeckLogPeriod(unsigned char Period);

unsigned char GetBleNeckLogEnable(void);
void SetBleNeckLogEnable(unsigned char Enable);

typedef int (*CALLBACK_LOG)(tNECK_LOG*);  // Declare Function pointer for CALLBACK_LOG


#endif
