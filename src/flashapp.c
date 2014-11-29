

#include "flashapp.h"
#include "flashfunc.h"
#include "typedef.h"

sFLASHOP flashQueue[FLASHQUEUE_SIZE] = {0};
sFLASHQUEUE sFlashQueue;


static const sMSG sFlashFinishMsg={M_TYPE_TRANS,M_C_FLASHFINISH};
// fFUNC fFlashOp[]={_nop_Ex,fStepSave,fNeckSave,fBlockErase,fReadStepLog,fReadNeckLog,fIdleWait};

fFUNC fFlashOp[]={_nop_Ex,fIdleWait,fFlashWrite,fBlockErase,fFlashRead,_nop_Ex};

extern sFLASHOP gOP;


void fFlashWrite(void)
{
	if(gOP.detail==FLASH_S_STEP){
		fStepSave();
	}else if(gOP.detail==FLASH_S_NECK){
		fNeckSave();
	}
}

void fFlashRead(void)
{
	if(gOP.detail==FLASH_S_STEP){
		fReadStepLog();
	}else if(gOP.detail==FLASH_S_NECK){
		fReadNeckLog();
	}
}

void flashOpFin(void)
{
	DI();
	fifoPut4ISR(sFlashFinishMsg);
	EI();
}

// extern _sFlash sFlash;
extern void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr);
void neckDataSave(uchar* ptr,uchar len)
{
	flashWrite(ptr, len, neckFlash.startAddr);
}

void stepDataSave(uchar* ptr,uchar len)
{
	flashWrite(ptr, len, stepFlash.startAddr);
}

void progDataSave(uchar* ptr,uchar len)
{
	flashWrite(ptr, len, programFlash.startAddr);
}

extern sSTEPLOG stepLog;
void fStepSave(void){
	stepDataSave(&stepLog,sizeof(sSTEPLOG));
	flashSeek(sizeof(sSTEPLOG),stepFlash.endAddr);
	flashOpFin();
}

extern void memsetUser(uchar* ptr,const uchar ch,const size_t length);
extern sNECKLOG neckLog[16];
void fNeckSave(void){
	uchar i=0;
	while(neckLog[i++].UTC!=0);
	i-=1;	//i 为记录条数
	if(!i) return;
	neckDataSave(&neckLog,i*sizeof(sNECKLOG));
	memsetUser(&neckLog,0,16*sizeof(sNECKLOG));
	flashSeek(i*sizeof(sNECKLOG),neckFlash.endAddr);
	flashOpFin();
}
// extern flashErase(unsigned short dataLength, unsigned long flashAddr)
void fBlockErase(void){
	if(gOP.detail==FLASH_S_STEP){
		flashErase(2*sizeof(sNECKLOG),neckFlash.startAddr);
	}else if(gOP.detail==FLASH_S_NECK){
		flashErase(sizeof(sSTEPLOG),stepFlash.startAddr);
	}
	flashOpFin();
}

uchar logCache[20]={0};
extern const uchar data_neckLog[3];
extern const uchar data_stepLog[3];
void fReadStepLog(void){
	uchar i;
	readFromFlashBytes(logCache,sizeof(sSTEPLOG),stepFlash.startAddr);
	uartBufWrite(data_stepLog,3);
	for(i=0;i<sizeof(sSTEPLOG);i++)
		uartSendBuf[i+3]=logCache[i];
	calcSendBufSum();
	uartSend(sizeof(sSTEPLOG)+4);
	flashOpFin();
}

void fReadNeckLog(void){
	uchar i;
	readFromFlashBytes(logCache,2*sizeof(sNECKLOG),neckFlash.startAddr);
	uartBufWrite(data_neckLog,3);
	for(i=0;i<2*sizeof(sNECKLOG);i++)
		uartSendBuf[i+3]=logCache[i];
	calcSendBufSum();
	uartSend(2*sizeof(sNECKLOG)+4);
	flashOpFin();
}

int flashIdleWaitTask(void)
{
	static uchar count=0;
	if(!(count&0x4) and isFlashIdle()){
		flashOpFin();
		return 0;
	}
	count++;
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

	if(sFlashQueue.remain_size>0){
		flashQueue[sFlashQueue.write_pos]=op;
		sFlashQueue.write_pos++;
		sFlashQueue.write_pos&=FLASHQUEUE_SIZEMASK;
		sFlashQueue.remain_size--;
		return 0;
	}
	return 1;

	// if(*ptr++>0){
	// 	pmsgQueue[*ptr]=op;
	// 	*ptr=((*ptr)+1)&FLASHQUEUE_SIZEMASK;
	// 	*ptr--;
	// 	(*ptr)--;
	// 	return 0;
	// }
	// return 1;
}

sFLASHOP flashOpGet(void)
{
	register uchar *ptr=&sFlashQueue.remain_size;
	static sFLASHOP *const pmsgQueue=flashQueue;
	sFLASHOP op;

	if(sFlashQueue.remain_size<FLASHQUEUE_SIZE){
		op=flashQueue[sFlashQueue.read_pos];
		sFlashQueue.read_pos++;
		sFlashQueue.read_pos&=FLASHQUEUE_SIZEMASK;
		sFlashQueue.remain_size++;
	}else{
		op.opType=0;
	}
	return op;

	// if(*ptr++<FLASHQUEUE_SIZE) {
	// 	ptr++;
	// 	op = pmsgQueue[*ptr];
	// 	*ptr =((*ptr)+1)&FLASHQUEUE_SIZEMASK;
	// 	*ptr--;
	// 	(*--ptr)++;
	// } else {
	// 	op.opType=0;
	// }
	return op;
}
