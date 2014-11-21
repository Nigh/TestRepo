
#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "r_cg_dmac.h"
#include "uartapp.h"


const uchar data_transSuccess[5]={'#',0x03,0x01,0x01,0x02};
const uchar data_transFail[5]={'#',0x03,0x01,0xFF,0x00};

const uchar data_neckLog[3]={'#',0x16,0x10};
const uchar data_stepLog[3]={'#',0x16,0x16};
const uchar data_logCount[3]={'#',0x06,0x12};
const uchar data_axisDirect[3]={'#',0x08,0x13};
const uchar data_batteryLevel[3]={'#',0x03,0x14};
const uchar data_bleCtrl[3]={'#',0x03,0x15};

uchar uartSendBuf[32]={1,2,3,4,5,6,7,8,9,1,2,3};
uchar uartRevBuf[32];

sUART sUart={UART_IDLE,0,0};

void uartInit(void)
{
	ST0 |= _0008_SAU_CH3_STOP_TRG_ON | _0004_SAU_CH2_STOP_TRG_ON;    /* disable UART1 receive and transmit */
	STMK1 = 1U;    /* disable INTST1 interrupt */
	STIF1 = 0U;    /* clear INTST1 interrupt flag */
	SRMK1 = 1U;    /* disable INTSR1 interrupt */
	SRIF1 = 0U;    /* clear INTSR1 interrupt flag */
	SREMK1 = 1U;   /* disable INTSRE1 interrupt */
	SREIF1 = 0U;   /* clear INTSRE1 interrupt flag */
	/* Set INTST1 low priority */
	STPR11 = 1U;
	STPR01 = 1U;
	/* Set INTSR1 low priority */
	SRPR11 = 1U;
	SRPR01 = 1U;
	SMR02 = _0020_SAU_SMRMN_INITIALVALUE | _0000_SAU_CLOCK_SELECT_CK00 | _0000_SAU_TRIGGER_SOFTWARE |
			_0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
	SCR02 = _8000_SAU_TRANSMISSION | _0000_SAU_INTSRE_MASK | _0000_SAU_PARITY_NONE | _0000_SAU_MSB | _0010_SAU_STOP_1 |
			_0007_SAU_LENGTH_8;
	SDR02 = _4400_UART1_TRANSMIT_DIVISOR;
	NFEN0 |= _04_SAU_RXD1_FILTER_ON;
	SIR03 = _0004_SAU_SIRMN_FECTMN | _0002_SAU_SIRMN_PECTMN | _0001_SAU_SIRMN_OVCTMN;    /* clear error flag */
	SMR03 = _0020_SAU_SMRMN_INITIALVALUE | _0000_SAU_CLOCK_SELECT_CK00 | _0100_SAU_TRIGGER_RXD | _0000_SAU_EDGE_FALL |
			_0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
	SCR03 = _4000_SAU_RECEPTION | _0000_SAU_INTSRE_MASK | _0000_SAU_PARITY_NONE | _0000_SAU_MSB | _0010_SAU_STOP_1 |
			_0007_SAU_LENGTH_8;
	SDR03 = _4400_UART1_RECEIVE_DIVISOR;
	SO0 |= _0004_SAU_CH2_DATA_OUTPUT_1;
	SOL0 |= _0000_SAU_CHANNEL2_NORMAL;    /* output level normal */
	SOE0 |= _0004_SAU_CH2_OUTPUT_ENABLE;    /* enable UART1 output */
	/* Set RxD1 pin */
	PM0 |= 0x02U;
	/* Set TxD1 pin */
	P0 |= 0x01U;
	PM0 &= 0xFEU;



	SO0 |= _0004_SAU_CH2_DATA_OUTPUT_1;    /* output level normal */
	SOE0 |= _0004_SAU_CH2_OUTPUT_ENABLE;    /* enable UART1 output */
	SS0 |= _0008_SAU_CH3_START_TRG_ON | _0004_SAU_CH2_START_TRG_ON;    /* enable UART1 receive and transmit */
	STIF1 = 0U;    /* clear INTST1 interrupt flag */
	SRIF1 = 0U;    /* clear INTSR1 interrupt flag */
	STMK1 = 0U;    /* enable INTST1 interrupt */
	SRMK1 = 0U;    /* enable INTSR1 interrupt */
}

static void uartBufInit(uchar len)
{
	DEN1 = 1;
	DSA1 = 0x44;
	DRA1 = (uint)uartSendBuf;
	DBC1 = len;
	// DEN1 = 0;
	// DSA0 = 0x46;
	// DRA0 = uartRevBuf;
}

void uartBufWrite(uchar *wPtr,uchar len)
{
	uchar i=0;
	uchar* bPtr=uartSendBuf;
	len&=0x1f;
	while(i++<len){
		*bPtr++=*wPtr++;
	}
}

void wait5msTimer(void)
{
	TMIF06 = 0U;    /* clear INTTM06 interrupt flag */
	TMMK06 = 0U;    /* enable INTTM06 interrupt */
	TS0 |= _0040_TAU_CH6_START_TRG_ON;
}

void uartSend(uchar len)
{
	if(sUart.statu!=UART_IDLE)
		return;
	sUart.statu=UART_WAIT;
	uartBufInit(len);
	P2.0=0;
	startHClk();
	// sUart.count=0;
	// sUart.time=0;
	wait5msTimer();
}

void calcSendBufSum(void)
{
	uchar len=uartSendBuf[1];
	uchar sum=0,i=2;
	while(i<=len){
		sum+=uartSendBuf[i++];
	}
	uartSendBuf[i]=sum;
}
