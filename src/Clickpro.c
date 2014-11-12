//#include "Clickpro.h"
#include "Clickpro.h"

static unsigned char CollectNum = 0;
static unsigned char CurDataNum = 0;
static char Xdifdata[8] = {0};
static char Ydifdata[8] = {0};
static char Zdifdata[8] = {0};
static unsigned char XAbsDif[8] = {0};
static unsigned char YAbsDif[8] = {0};
static unsigned char ZAbsDif[8] = {0};
static char XLastdata = 0;
static char YLastdata = 0;
static char ZLastdata = 0;
static unsigned char XAbsData = 0;
static unsigned char YAbsData = 0;
static unsigned char ZAbsData = 0;
static unsigned char XAbsData_2 = 0;
static unsigned char YAbsData_2 = 0;
static unsigned char ZAbsData_2 = 0;
//static char XValidTime = 0;
//static char YValidTime = 0;
//static char ZValidTime = 0;
static char ClickInvalidTime = 0;
static unsigned char StableTimes = 0;
static unsigned ClickFlag = 0;			//bit0 0: 1: X Click fit;
										//bit1 0: 1: Y Click fit;
										//bit2 0: 1: Z Click fit;

										//bit4 0: 1: X Click fit flag should be clear
										//bit5 0: 1: Y Click fit flag should be clear
										//bit6 0: 1: Z Click fit flag should be clear
unsigned char IsClick(unsigned char *clickdata)
{
	unsigned char returedata = 0;
	unsigned char temp1 = 0;
	unsigned char temp2 = 0;
	unsigned char tempA = 0;
	unsigned char comparedata = 0;


	//unsigned char k = 0;

	if(CollectNum == 8)
	{
		Xdifdata[CurDataNum] = (unsigned char)((signed int)(*clickdata) - XLastdata);
		Ydifdata[CurDataNum] = (unsigned char)((signed int)(*(clickdata + 1)) - YLastdata);
		Zdifdata[CurDataNum] = (unsigned char)((signed int)(*(clickdata + 2)) - ZLastdata);


        XAbsData_2 = XAbsData;
        YAbsData_2 = YAbsData;
        ZAbsData_2 = ZAbsData;

        if(*clickdata & 0x80)
        {
            XAbsData = (unsigned char)(*(clickdata) ^ 0xff) + 1;
        }
        else
        {
            XAbsData = (unsigned char)*(clickdata);
        }

        if(*(clickdata + 1) & 0x80)
        {
            YAbsData = (unsigned char)(*(clickdata + 1) ^ 0xff) + 1;
        }
        else
        {
            YAbsData = (unsigned char)*(clickdata + 1);
        }

        if(*(clickdata + 2) & 0x80)
        {
            ZAbsData = (unsigned char)(*(clickdata + 2) ^ 0xff) + 1;
        }
        else
        {
            ZAbsData = (unsigned char)*(clickdata + 2);
        }





		if(Xdifdata[CurDataNum] & 0x80)
		{
			XAbsDif[CurDataNum] = (unsigned char)(Xdifdata[CurDataNum] ^ 0xff) + 1;
		}
		else
		{
			XAbsDif[CurDataNum] = Xdifdata[CurDataNum];
		}

		if(Ydifdata[CurDataNum] & 0x80)
		{
			YAbsDif[CurDataNum] = (unsigned char)(Ydifdata[CurDataNum] ^ 0xff) + 1;
		}
		else
		{
			YAbsDif[CurDataNum] = Ydifdata[CurDataNum];
		}

		if(Zdifdata[CurDataNum] & 0x80)
		{
			ZAbsDif[CurDataNum] = (unsigned char)(Zdifdata[CurDataNum] ^ 0xff) + 1;
		}
		else
		{
			ZAbsDif[CurDataNum] = Zdifdata[CurDataNum];
		}

		XLastdata = (signed char)(*clickdata);
		YLastdata = (signed char)(*(clickdata + 1));
		ZLastdata = (signed char)(*(clickdata + 2));
        if(((XAbsData_2 + XAbsData) > 253) ||
           ((YAbsData_2 + YAbsData) > 253) ||
           ((ZAbsData_2 + ZAbsData) > 253))
        {
            ClickInvalidTime = 10;
        }


		if((XAbsDif[CurDataNum] < 5) &&
		   (YAbsDif[CurDataNum] < 5) &&
		   (ZAbsDif[CurDataNum] < 5))		//in the stable state
		{
			StableTimes++;
			if(StableTimes > 14)
			{
				StableTimes = 15;
				ClickFlag = 0;
			}
			if(ClickInvalidTime)
            {
                ClickInvalidTime--;
            }
		}
		else
		{
			if(StableTimes < 4)
			{
				StableTimes = 0;
			}
			else
			{
				StableTimes --;
			}

			if(ClickInvalidTime)
			{
				ClickInvalidTime--;
				returedata = 0;
				goto	ActivityEndPro;
			}




			if(CurDataNum == 0)
			{
				temp1= 7;
			}
			else
			{
				temp1 = CurDataNum - 1;
			}

			if(temp1 == 0)
			{
				temp2 = 7;
			}
			else
			{
				temp2 = temp1 - 1;
			}




			if(((Xdifdata[CurDataNum] ^ Xdifdata[temp1]) & 0x80) == 0x80)		//direction different
			{
				if(XAbsDif[CurDataNum] > XAbsDif[temp1])
				{
					tempA = XAbsDif[CurDataNum] - XAbsDif[temp1];
				}
				else
				{
					tempA = XAbsDif[temp1] - XAbsDif[CurDataNum];
				}
				if(tempA < (XAbsDif[temp1] >> 1))		//two difdata different in 1/2
				{
					if(StableTimes > 10)
					{
						comparedata = CLICKSTABLELIMIT;
					}
					else
					{
						comparedata = CLICKACTIVELIMIT;

					}
					if(XAbsDif[temp1] > comparedata)	//the indemitary data change fit the condition
					{
						if((Xdifdata[temp1] ^ Xdifdata[temp2]) & 0x80)		//before
						{
							ClickFlag |= 0x01;
							if(XAbsDif[temp1] > 50)
							{
								ClickFlag = 0x07;
							}
							if(ClickFlag & 0x06)
							{
								ClickFlag = 0x07;
							}
						}
						else
						{
							if((XAbsDif[temp1] >> 2) > XAbsDif[temp2])
							{
								ClickFlag |= 0x01;
								if(XAbsDif[temp1] > 50)
								{
									ClickFlag = 0x07;
								}
								if(ClickFlag & 0x06)
								{
									ClickFlag = 0x07;
								}
							}
						}
					}
				}
			}


			if(((Ydifdata[CurDataNum] ^ Ydifdata[temp1]) & 0x80) == 0x80)		//direction different
			{
				if(YAbsDif[CurDataNum] > YAbsDif[temp1])
				{
					tempA = YAbsDif[CurDataNum] - YAbsDif[temp1];
				}
				else
				{
					tempA = YAbsDif[temp1] - YAbsDif[CurDataNum];

				}
				if(tempA < (YAbsDif[temp1] >> 1))		//two difdata different in 1/2
				{
					if(StableTimes > 10)
					{
						comparedata = CLICKSTABLELIMIT;
					}
					else
					{
						comparedata = CLICKACTIVELIMIT;

					}
					if(YAbsDif[temp1] > comparedata)	//the indemitary data change fit the condition
					{
						if((Ydifdata[temp1] ^ Ydifdata[temp2]) & 0x80)		//before
						{
							ClickFlag |= 0x02;
							if(ClickFlag & 0x05)
							{
								ClickFlag = 0x07;
							}
							if(YAbsDif[temp1] > 50)
							{
								ClickFlag = 0x07;
							}
						}
						else
						{
							if((YAbsDif[temp1] >> 2) > YAbsDif[temp2])
							{
								ClickFlag |= 0x02;
								if(YAbsDif[temp1] > 50)
								{
									ClickFlag = 0x07;
								}
								if(ClickFlag & 0x05)
								{
									ClickFlag = 0x07;
								}
							}
						}
					}
				}
			}



			if(((Zdifdata[CurDataNum] ^ Zdifdata[temp1]) & 0x80) == 0x80)		//direction different
			{
				if(ZAbsDif[CurDataNum] > ZAbsDif[temp1])
				{
					tempA = ZAbsDif[CurDataNum] - ZAbsDif[temp1];
				}
				else
				{
					tempA = ZAbsDif[temp1] - ZAbsDif[CurDataNum];
				}
				if(tempA < (ZAbsDif[temp1] >> 1))		//two difdata different in 1/2
				{
					if(StableTimes > 10)
					{
						comparedata = CLICKSTABLELIMIT;
					}
					else
					{
						comparedata = CLICKACTIVELIMIT;

					}
					if(ZAbsDif[temp1] > comparedata)	//the indemitary data change fit the condition
					{
						if((Zdifdata[temp1] ^ Zdifdata[temp2]) & 0x80)		//before
						{
							ClickFlag |= 0x04;
							if(ZAbsDif[temp1] > 50)
							{
								ClickFlag = 0x07;
							}
							if(ClickFlag & 0x03)
							{
								ClickFlag = 0x07;
							}

						}
						else
						{
							if((ZAbsDif[temp1] >> 2) > ZAbsDif[temp2])
							{
								ClickFlag |= 0x04;
								if(ZAbsDif[temp1] > 50)
								{
									ClickFlag = 0x07;
								}
								if(ClickFlag & 0x03)
								{
									ClickFlag = 0x07;
								}
							}
						}
					}
				}
			}

			if(StableTimes > 10)
			{
				if(ClickFlag)
				{
					ClickInvalidTime = 7;
					ClickFlag = 0;
					returedata = 1;
				}
			}
			else
			{
				if((ClickFlag & 0x07) == 0x07)
				{
					ClickInvalidTime = 5;
					ClickFlag = 0;
					returedata = 1;

					//system("pause");
				}
				else
				{
					if(ClickFlag & 0x10)
					{
						ClickFlag &= 0xFE;
					}
					if(ClickFlag & 0x20)
					{
						ClickFlag &= 0xFD;
					}
					if(ClickFlag & 0x40)
					{
						ClickFlag &= 0xFB;
					}
					ClickFlag |= (ClickFlag << 4);
				}
			}


		}


ActivityEndPro:

		CurDataNum ++;
		if(CurDataNum == 8)
		{
			CurDataNum = 0;
		}
	}
	else
	{
		Xdifdata[CollectNum] = (signed char)(*clickdata) - XLastdata;
		Ydifdata[CollectNum] = (signed char)(*(clickdata + 1)) - YLastdata;
		Zdifdata[CollectNum] = (signed char)(*(clickdata + 2)) - ZLastdata;
		if(Xdifdata[CollectNum] & 0x80)
		{
			XAbsDif[CollectNum] = (unsigned char)(Xdifdata[CollectNum] ^ 0xff) + 1;
		}
		else
		{
			XAbsDif[CollectNum] = Xdifdata[CollectNum];
		}

		if(Ydifdata[CollectNum] & 0x80)
		{
			YAbsDif[CollectNum] = (unsigned char)(Ydifdata[CollectNum] ^ 0xff) + 1;
		}
		else
		{
			YAbsDif[CollectNum] = Ydifdata[CollectNum];
		}

		if(Zdifdata[CollectNum] & 0x80)
		{
			ZAbsDif[CollectNum] = (unsigned char)(Zdifdata[CollectNum] ^ 0xff) + 1;
		}
		else
		{
			ZAbsDif[CollectNum] = Zdifdata[CollectNum];
		}
		XLastdata = (signed char)(*clickdata);
		YLastdata = (signed char)(*(clickdata + 1));
		ZLastdata = (signed char)(*(clickdata + 2));
		CollectNum++;
		if(CollectNum == 8)
		{
			CurDataNum = 0;
		}
	}
	return returedata;
}
