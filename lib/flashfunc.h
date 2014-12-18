#ifndef		__FLASHFUNC_H
#define		__FLASHFUNC_H


#define		NECKRANGSTART		0		//BLOCK0 -- BLOCK21
#define		NECKRANGEND			86400
#define		NECKSTARTBLOCK		0
#define		NECKENDBLOCK		21

#define		ADDR_SAVE_START		90112
#define		ADDR_SAVE_BLOCK		22

#define		STEPRANGSTART		131072	//BLOCK32 -- BLOCK37
#define		STEPRANGEND			152672
#define		STEPSTARTBLOCK		32
#define		STEPENDBLOCK		37

#define		PROGRAMFLAGADDR		159744

#define		PROGRAMERRANGSTART	196608	//BLOCK48 -- BLOCKXX (0x30000)
#define		PROGRAMERRANGEND	655360

#define FLASH_BLOCK_SIZE (0x1000)

typedef	struct 
{
	unsigned char	flashdata[64];
	unsigned char	datacnt;
	unsigned char	length;
	unsigned char	programkind;
	unsigned char	erasing;
	unsigned char	rewrite;
	unsigned long	curAddr;
	unsigned long	eraseblock;
}_sFlash;


typedef	struct 
{
	unsigned char	overwrite;
	unsigned short	eraseblock;
	unsigned long	startAddr;
	unsigned long	endAddr;
}_sFlashRang;


extern _sFlashRang stepFlash;
extern _sFlashRang neckFlash;
extern _sFlashRang programFlash;


void writeToFlashBytes(unsigned char *datapoint, unsigned char dataLength, unsigned long flashAddr);
void readFromFlashBytes(unsigned char *rxbufPointer, unsigned short dataLength, unsigned long flashAddr);
void delayMS(unsigned short millisec);
void flashWriteFunc(void);
void delay10Nop(void);

void initFlash(void);
void saveNeckData(unsigned char *dataP, unsigned char dataLength);
void saveStepData(unsigned char *dataP, unsigned char dataLength);
void saveProgramData(unsigned char *dataP, unsigned char dataLength);

unsigned char readNeckData(unsigned char *datap, unsigned char length);
unsigned char readStepData(unsigned char *datap, unsigned char length);
unsigned char readProgramData(unsigned char *datap, unsigned char length);

unsigned long readNeckstartAddr(void);
unsigned long readNeckendAddr(void);
unsigned long readStepstartAddr(void);
unsigned long readStependAddr(void);
unsigned long readProgramstartAddr(void);
unsigned long readProgramendAddr(void);

int isFlashIdle(void);
unsigned char needErase(unsigned short dataLength, unsigned long flashAddr);
void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr);
void flashErase(unsigned short dataLength, unsigned long flashAddr);
void flashSeek(unsigned short dataLength, unsigned long flashAddr);

#endif
