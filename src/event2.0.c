
#include "event2.0.h"

/************* !!!WARNING!!! ************
 * *********** *IMPORTANT* ************ *
The buf_size must be power of 2 !!!
***************************************/

sMSG msgQueue[BUF_SIZE] = {0};
sFIFO sMsgFifo;

void fifoInit(sFIFO *p_fifo, sMSG *p_buf_head)
{
	p_fifo->p_buf = p_buf_head;
	p_fifo->buf_size_mask = BUF_SIZE - 1;
	p_fifo->read_pos = 0;
	p_fifo->write_pos = 0;
	p_fifo->remain_size = BUF_SIZE;
};

int fifoPut4ISR(sMSG msg)
{
	register uchar *ptr=&sMsgFifo.remain_size;
	static sMSG *const pmsgQueue=msgQueue;

	if(*ptr++>0){
		pmsgQueue[*ptr]=msg;
		*ptr--=((*ptr)+1)&BUF_SIZE_MASK;
		(*ptr)--;
		return 0;
	}
	return 1;
}

sMSG fifoGet(void)
{
	register uchar *ptr=&sMsgFifo.remain_size;
	static sMSG *const pmsgQueue=msgQueue;
	sMSG msg;

	if(*ptr++<BUF_SIZE) {
		ptr++;
		msg = pmsgQueue[*ptr];
		*ptr-- =((*ptr)+1)&BUF_SIZE_MASK;
		(*--ptr)++;
	} else {
		msg.type=0;
	}
	return msg;
}

void fifoFlush(void)
{
	sMsgFifo.read_pos=sMsgFifo.write_pos;
	sMsgFifo.remain_size=sMsgFifo.buf_size_mask+1;
}

