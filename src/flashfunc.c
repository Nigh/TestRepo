#include "r_cg_macrodriver.h"
#include "flashfunc.h"
#include "bootmain.h"
#include "flashapp.h"

static unsigned char 		spirxflash[32]={0};
static unsigned char		spitxflash[32]={0};
static unsigned long		flash_mcuaddr;

_sFlash				sFlash;
_sFlashRang			stepFlash;
_sFlashRang			neckFlash;
_sFlashRang			programFlash;

const  unsigned char		flashaddradd[5] = {4, 8, 12, 16, 0};
//**********************************************************************************************************************
//Function Name: writeToFlashBytes
//Description  : write the bytes to flash
//Arguments    : datapoint: the data point
// 				 datalenth: the data length
// 				 flashAddr: the flash address the data writing
//return Value : sucess: 0x00;
// 			     error:  0xff;
//*********************************************************************************************************************/
void writeToFlashBytes(unsigned char *datapoint, unsigned char dataLength, unsigned long flashAddr)
{
	unsigned char i;

	for(i = 0; i < dataLength; i++) {
		sFlash.flashdata[i] = datapoint[i];
	}
	sFlash.datacnt = 0;
	sFlash.length = dataLength;
	sFlash.curAddr = flashAddr;
	sFlash.erasing = 0;
	flashWriteFunc();
}

//**********************************************************************************************************************
//Function Name: flashWriteFunc
//Description  : write the bytes to flash
//Arguments    : datapoint: the data point
// 				 datalenth: the data length
// 				 flashAddr: the flash address the data writing
//return Value : sucess: 0x00;
// 			     error:  0xff;
//*********************************************************************************************************************/
void flashWriteFunc(void)
{
	unsigned char	flashReg;
	unsigned char	delaycnt;
	unsigned short	curblocktemp;
	// read the flash register
	sFlash.rewrite = 0;

	if(isFlashIdle()){
		//process writring
		curblocktemp =(unsigned short)(sFlash.curAddr >> 12);
		if(sFlash.eraseblock != curblocktemp) {
erasetheblock:
			if(sFlash.erasing == 0) {
				sFlash.erasing = 1;
				enable_flash();
				FlashCommand(WREN_FLASH, 0);
				disable_flash();
				delay10Nop();
				enable_flash();
				FlashCommand(SE_FLASH, sFlash.curAddr);
				disable_flash();
				delay10Nop();
				enable_flash();
				FlashCommand(RDSR_FLASH, 0);
				SIO00=0xFF;
				while(CSIIF00==0);
				CSIIF00=0;
				flashReg = SIO00;
				if((flashReg != 0xFF) && ((flashReg & 0x01) == 0x01)) {
					sFlash.eraseblock = curblocktemp;
					switch(sFlash.programkind) {
					case 1: neckFlash.eraseblock = curblocktemp; break;
					case 2: stepFlash.eraseblock = curblocktemp; break;
					case 3: programFlash.eraseblock = curblocktemp; break;
					}
				}
				disable_flash();
			} else {
				sFlash.erasing = 0;
			}

			////delay 400ms then do this function(flashWriteFunc)
			sFlash.rewrite = 1;
		} else {
rewriteaddr:
			//write the data to flash
			if(sFlash.datacnt == sFlash.length) {
				goto	writeEnd;
			}
			enable_flash();
			FlashCommand(WREN_FLASH, 0);
			delay10Nop();
			disable_flash();
			delay10Nop();
			enable_flash();
			FlashCommand(PAGEP_FLASH, sFlash.curAddr);

			do {
				SIO00 = sFlash.flashdata[sFlash.datacnt];
				while(CSIIF00 ==0);
				CSIIF00=0;

				sFlash.curAddr++;
				sFlash.datacnt++;

				curblocktemp =(unsigned short)(sFlash.curAddr >> 12);
				if(sFlash.eraseblock != curblocktemp) {
					disable_flash();
					delay10Nop();
					goto	erasetheblock;
				}

				if(sFlash.curAddr % 256 == 0 && (sFlash.datacnt < sFlash.length)) {
					//rewrite addr
redelay:
					delay10Nop();
					disable_flash();
					delay10Nop();
					enable_flash();
					FlashCommand(RDSR_FLASH, 0);
					SIO00=0xFF;
					while(CSIIF00==0);
					CSIIF00=0;
					flashReg = SIO00;
					disable_flash();
					delay10Nop();
					if((flashReg & 0x01) == 0x01) {
						for(delaycnt = 0; delaycnt < 100; delaycnt++) {
							delay10Nop();
						}
						goto	redelay;
					}
					goto rewriteaddr;
				}
			} while(sFlash.datacnt < sFlash.length);
writeEnd:
			disable_flash();
		}
	}
}


void flashSeek(unsigned short dataLength, unsigned long flashAddr)
{
	unsigned long block;
	if(flashAddr>=PROGRAMERRANGSTART){
		programFlash.endAddr += dataLength;
	}else if(flashAddr>=STEPRANGSTART){

		stepFlash.endAddr += dataLength;
		if(stepFlash.endAddr >= STEPRANGEND) {
			stepFlash.endAddr = 0;
			stepFlash.overwrite = 1;
			stepFlash.startAddr = 4096 + 4 + STEPRANGSTART;
		}
		if(stepFlash.overwrite) {
			block = (stepFlash.endAddr / 4096);
			if(stepFlash.startAddr <= stepFlash.endAddr) {
				stepFlash.startAddr += (block + 1) * 4096 +
									   flashaddradd[(block - STEPSTARTBLOCK) % 5];
				if(stepFlash.startAddr >= STEPRANGEND) {
					stepFlash.startAddr = 0;
				}
			}
		}
	}else if(flashAddr>=NECKRANGSTART){

		neckFlash.endAddr += dataLength;

		if(neckFlash.endAddr >= NECKRANGEND) {
			neckFlash.endAddr = 0;
			neckFlash.overwrite = 1;
			neckFlash.startAddr = 4096 + 4 + NECKRANGSTART;
		}
		if(neckFlash.overwrite) {
			block = (neckFlash.endAddr / 4096);
			if(neckFlash.startAddr <= neckFlash.endAddr) {
				neckFlash.startAddr += (block + 1) * 4096 +
									   flashaddradd[(block - NECKSTARTBLOCK) % 5];
				if(neckFlash.startAddr >= NECKRANGEND) {
					neckFlash.startAddr = 0;
				}
			}
		}
	}
}

void flashErase(unsigned short dataLength, unsigned long flashAddr)
{
	unsigned long addr;
	enable_flash();
	FlashCommand(WREN_FLASH, 0);
	disable_flash();
	delay10Nop();
	enable_flash();
	FlashCommand(SE_FLASH, flashAddr + dataLength - 1);
	disable_flash();
}


void flashWrite(unsigned char* ptr, unsigned short dataLength, unsigned long flashAddr)
{
	unsigned short i = 0;
	unsigned char delaycnt;
	unsigned char flashReg;

rewriteaddr2:
	enable_flash();
	FlashCommand(WREN_FLASH, 0);
	delay10Nop();
	disable_flash();
	delay10Nop();
	enable_flash();
	FlashCommand(PAGEP_FLASH, flashAddr);
	do {
			SIO00 = ptr[i];
			while(CSIIF00 ==0);
			CSIIF00=0;
			flashAddr++;
			i++;
			if(flashAddr == NECKRANGEND)
			{
				flashAddr = NECKRANGSTART;
			}
			if(flashAddr == STEPRANGEND)
			{
				flashAddr = STEPRANGSTART;
			}
			if(flashAddr % 256 == 0 && (i < dataLength)) {
					//rewrite addr
redelay2:

				DI();
				SIO00=0xFF;
				NOP();NOP();
				P2.3=1;
				EI();
				while(CSIIF00==0);
				CSIIF00=0;

				disable_flash();
				delay10Nop();
				enable_flash();
				FlashCommand(RDSR_FLASH, 0);
				SIO00=0xFF;
				while(CSIIF00==0);
				CSIIF00=0;
				flashReg = SIO00;
				disable_flash();
				delay10Nop();
				if((flashReg & 0x01) == 0x01) {
					for(delaycnt = 0; delaycnt < 100; delaycnt++) {
							delay10Nop();
					}
						goto	redelay2;
				}
				goto rewriteaddr2;
			}
		} while(i < dataLength);

		DI();
		SIO00=0xFF;
		NOP();NOP();
		P2.3=1;
		EI();
		while(CSIIF00==0);
		CSIIF00=0;
		disable_flash();
}

unsigned char needErase(unsigned short dataLength, unsigned long flashAddr)
{
	unsigned int temp=flashAddr%FLASH_BLOCK_SIZE;
	if(temp+dataLength>FLASH_BLOCK_SIZE)	//(temp==0时，应返回1)
		return 1;
	if(temp==0)
		return 1;
	if(flashAddr==NECKRANGSTART || flashAddr==STEPRANGSTART || flashAddr==PROGRAMERRANGSTART)
		return 1;
	return 0;
}

//**********************************************************************************************************************
//Function Name: writeToFlashBytes
//Description  : write the bytes to flash
//Arguments    : datapoint: the target data point
// 				 datalenth: the data length
// 				 flashAddr: the flash address the data writing
//return Value : None
//*********************************************************************************************************************/
void readFromFlashBytes(unsigned char *rxbufPointer, unsigned short dataLength, unsigned long flashAddr)
{
	unsigned ret, flashReg;
	unsigned short i;

	enable_flash();
	FlashCommand(READ_FLASH, flashAddr);
	for(i = 0; i < dataLength; i++) {
		SIO00=0xFF;
		while(CSIIF00==0);
		CSIIF00=0;
		rxbufPointer[i] = SIO00;
		flashAddr++;
		if((flashAddr % 256) == 0 && (sFlash.datacnt < sFlash.length)) {
			delay10Nop();
			disable_flash();
			delay10Nop();
			enable_flash();
			FlashCommand(READ_FLASH, flashAddr);
		}
	}
	disable_flash();
}


void delay10Nop(void)
{
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
}



//**********************************************************************************************************************
//Description  : init the neck data start addr, end addr and the rang
//				 init the step data start addr, end addr and the rang
//Arguments    :
//return Value :
//*********************************************************************************************************************/
void initFlash(void)
{

	stepFlash.overwrite = 0;
	stepFlash.startAddr = STEPRANGSTART;
	stepFlash.endAddr = STEPRANGSTART;
	stepFlash.eraseblock = 0xffff;

	neckFlash.overwrite = 0;
	neckFlash.startAddr = NECKRANGSTART;
	neckFlash.endAddr = NECKRANGSTART;
	neckFlash.eraseblock = 0xffff;

	programFlash.overwrite = 0;
	programFlash.startAddr = PROGRAMERRANGSTART;
	programFlash.endAddr = PROGRAMERRANGSTART;
	programFlash.eraseblock = 0xffff;
}

//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
void saveNeckData(unsigned char *dataP, unsigned char dataLength)
{
	sFlash.programkind = 1;
	sFlash.eraseblock = neckFlash.eraseblock;
	writeToFlashBytes(dataP, dataLength, neckFlash.endAddr);

	neckFlash.endAddr += dataLength;

	if(neckFlash.endAddr >= NECKRANGEND) {
		neckFlash.endAddr = 0;
		neckFlash.overwrite = 1;
		neckFlash.startAddr = 4096 + 4 + NECKRANGSTART;
	}
	if(neckFlash.overwrite) {
		if(neckFlash.startAddr <= neckFlash.endAddr) {
			neckFlash.startAddr += (neckFlash.eraseblock + 1) * 4096 +
								   flashaddradd[(neckFlash.eraseblock - NECKSTARTBLOCK) % 5];
			if(neckFlash.startAddr >= NECKRANGEND) {
				neckFlash.startAddr = 0;
			}
		}
	}
	neckFlash.eraseblock = sFlash.eraseblock;
}



//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
void saveStepData(unsigned char *dataP, unsigned char dataLength)
{
	sFlash.programkind = 2;
	sFlash.eraseblock = stepFlash.eraseblock;
	writeToFlashBytes(dataP, dataLength, stepFlash.endAddr);

	stepFlash.endAddr += dataLength;
	if(stepFlash.endAddr >= STEPRANGEND) {
		stepFlash.endAddr = 0;
		stepFlash.overwrite = 1;
		stepFlash.startAddr = 4096 + 4 + STEPRANGSTART;
	}
	if(stepFlash.overwrite) {
		if(stepFlash.startAddr <= stepFlash.endAddr) {
			stepFlash.startAddr += (stepFlash.eraseblock + 1) * 4096 +
								   flashaddradd[(stepFlash.eraseblock - STEPSTARTBLOCK) % 5];
			if(stepFlash.startAddr >= STEPRANGEND) {
				stepFlash.startAddr = 0;
			}
		}
	}
	stepFlash.eraseblock = sFlash.eraseblock;
}


//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
void saveProgramData(unsigned char *dataP, unsigned char dataLength)
{
	sFlash.programkind = 3;
	sFlash.eraseblock = programFlash.eraseblock;
	writeToFlashBytes(dataP, dataLength, programFlash.endAddr);

	programFlash.endAddr += dataLength;
	programFlash.eraseblock = sFlash.eraseblock;
}
//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
// unsigned char readProgramData(unsigned char *datap, unsigned char length)
// {
// 	unsigned char ret;
// 	if(programFlash.endAddr != programFlash.startAddr) {
// 		ret = readFromFlashBytes(datap, length, programFlash.startAddr);
// 		if(ret == 0x00) {
// 			programFlash.startAddr += length;
// 		}
// 	} else {
// 		ret = 0xff;
// 	}
// 	return ret;
// }

//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
// unsigned char readNeckData(unsigned char *datap, unsigned char length)
// {
// 	unsigned char ret;
// 	if(neckFlash.endAddr != neckFlash.startAddr) {
// 		ret = readFromFlashBytes(datap, length, neckFlash.startAddr);
// 		if(ret == 0x00) {
// 			neckFlash.startAddr += length;
// 			if(neckFlash.startAddr >= NECKRANGEND) {
// 				neckFlash.startAddr = 0;
// 			}
// 		}
// 	} else {
// 		ret = 0xff;
// 	}
// 	return ret;
// }


//**********************************************************************************************************************
//Arguments    : data point and the length
//return Value : 0x00: success
//				 0xff: fail
//*********************************************************************************************************************/
// unsigned char readStepData(unsigned char *datap, unsigned char length)
// {
// 	unsigned char ret;
// 	if(stepFlash.endAddr != stepFlash.startAddr) {
// 		ret = readFromFlashBytes(datap, length, stepFlash.startAddr);
// 		if(ret == 0x00) {
// 			stepFlash.startAddr += length;
// 			if(stepFlash.startAddr >= STEPRANGEND) {
// 				stepFlash.startAddr = 0;
// 			}
// 		}
// 	} else {
// 		ret = 0xff;
// 	}
// 	return ret;
// }


//**********************************************************************************************************************
//return Value : the start addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readNeckstartAddr(void)
{
	return neckFlash.startAddr;
}

//**********************************************************************************************************************
//return Value : the end addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readNeckendAddr(void)
{
	return neckFlash.endAddr;
}


//**********************************************************************************************************************
//return Value : the start addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readStepstartAddr(void)
{
	return stepFlash.startAddr;
}

//**********************************************************************************************************************
//return Value : the end addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readStependAddr(void)
{
	return stepFlash.endAddr;
}

//**********************************************************************************************************************
//return Value : the start addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readProgramstartAddr(void)
{
	return programFlash.startAddr;
}

//**********************************************************************************************************************
//return Value : the end addr of Neckdata in extern flash
//*********************************************************************************************************************/
unsigned long readProgramendAddr(void)
{
	return programFlash.endAddr;
}


//**********************************************************************************************************************
//return Value : 0x00: standby
// 				 0x01: busy
//*********************************************************************************************************************/
int isFlashIdle(void)
{
	uchar flashReg;
	enable_flash();
	FlashCommand(RDSR_FLASH, 0);
	SIO00=0xFF;
	while(CSIIF00==0);CSIIF00=0;
	flashReg = SIO00;
	disable_flash();
	if((flashReg & 0x01) == 0x01) {
		return 0;
	} else {
		return 1;
	}
}
