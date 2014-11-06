#include "gvar.h"


void _nop_Ex(void)
{
	NOP();
}

void _halt_Ex(void)
{
	HALT();
	NOP();
}
