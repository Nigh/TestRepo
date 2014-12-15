
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


#define FLASH_S_STEP 0xF1
#define FLASH_S_NECK 0xF2
#define FLASH_S_OAD 0xF3

#define FLASH_F_IDLEWAIT 1
#define FLASH_F_WRITE 2
#define FLASH_F_BLOCKERASE 3

#define FLASH_F_READ 4

extern fFUNC fFlashOp[];
extern sFLASHQUEUE sFlashQueue;

void fFlashWrite(void);
void fFlashRead(void);

void fStepSave(void);
void fNeckSave(void);
void fOADSave(void);

void fBlockErase(void);
void fReadStepLog(void);
void fReadNeckLog(void);
void fIdleWait(void);

extern void flashQueueInit(sFLASHQUEUE *p_fifo);
extern int flashOpPut(sFLASHOP op);
extern sFLASHOP flashOpGet(void);
extern int flashIdleWaitTask(void);
extern void flashOpFin(void);


#endif

