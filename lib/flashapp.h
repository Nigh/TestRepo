
#ifndef _FLASHAPP_H_
#define _FLASHAPP_H_
#include "r_cg_userdefine.h"
#include "typedef.h"
#include "apptimer.h"

#define FLASHQUEUE_SIZE 0x8
#define FLASHQUEUE_SIZEMASK 0x7



typedef struct
{
	uchar opType;
	uint detail;
} sFLASHOP;

typedef struct
{
	uchar remain_size;
	uchar write_pos;
	uchar read_pos;
	uchar buf_size_mask;
	sFLASHOP *p_buf;
} sFLASHQUEUE;


#define FLASH_F_STEPSAVE 1
#define FLASH_F_NECKSAVE 2
#define FLASH_F_BLOCKERASE 3
#define FLASH_F_READSTEPLOG 4
#define FLASH_F_READNECKLOG 5
#define FLASH_F_IDLEWAIT 6

extern fFUNC fFlashOp[];
extern sFLASHQUEUE sFlashQueue;

void fStepSave(void);
void fNeckSave(void);
void fBlockErase(void);
void fReadStepLog(void);
void fReadNeckLog(void);
void fIdleWait(void);


extern void flashQueueInit(sFLASHQUEUE *p_fifo);
extern int flashOpPut(sFLASHOP op);
extern sFLASHOP flashOpGet(void);
extern int flashIdleWaitTask(void);
extern int isFlashIdle(void);


#endif

