#include "flashapp.h"

sFLASHOP flashQueue[FLASHQUEUE_SIZE] = {0};
sFLASHQUEUE sFlashQueue;


static const sMSG sFlashFinishMsg={M_TYPE_TRANS,M_C_FLASHFINISH};
fFUNC fFlashOp[]={_nop_Ex,fStepSave,fNeckSave,fBlockErase,fReadStepLog,fReadNeckLog,fIdleWait};

int isFlashIdle(void)
{
	return 1;
}

void fStepSave(void){
	
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

void fNeckSave(void){
	
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

void fBlockErase(void){
	
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

void fReadStepLog(void){
	
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

void fReadNeckLog(void){
	
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

int flashIdleWaitTask(void)
{
	static uchar count=0;
	if(!(count&0x4) and isFlashIdle()){
		DI();
		fifoPut4ISR(sFlashFinishMsg);
		EI();
		return 0;
	}
	return 1;
}

void fIdleWait(void){
	taskInsert(&flashIdleWaitTask);
}


void flashQueueInit(sFLASHQUEUE *p_fifo)
{
	p_fifo->p_buf = flashQueue;
	p_fifo->buf_size_mask = FLASHQUEUE_SIZE - 1;
	p_fifo->read_pos = 0;
	p_fifo->write_pos = 0;
	p_fifo->remain_size = FLASHQUEUE_SIZE;
}

int flashOpPut(sFLASHOP op)
{
	register uchar *ptr=&sFlashQueue.remain_size;
	static sFLASHOP *const pmsgQueue=flashQueue;

	if(*ptr++>0){
		pmsgQueue[*ptr]=op;
		*ptr--=((*ptr)+1)&BUF_SIZE_MASK;
		(*ptr)--;
		return 0;
	}
	return 1;
}

sFLASHOP flashOpGet(void)
{
	register uchar *ptr=&sFlashQueue.remain_size;
	static sFLASHOP *const pmsgQueue=msgQueue;
	sFLASHOP op;

	if(*ptr++<FLASHQUEUE_SIZE) {
		ptr++;
		op = pmsgQueue[*ptr];
		*ptr-- =((*ptr)+1)&FLASHQUEUE_SIZEMASK;
		(*--ptr)++;
	} else {
		op.opType=0;
	}
	return op;
}
