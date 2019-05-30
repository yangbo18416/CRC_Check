#include <stdio.h>
#include "math.h"
#include <string.h>
#define CRC8_MUL 0X07    //多项式 x8+x2+x1+1  --去除最高位，因为最高位每次异或都为0
#define CRC16_MUL 0x1021//多项式 x16 + x15 + x2 + 1
#define CRC16_MUL_2 0x8408//多项式 x16 + x15 + x2 + 1
#define CRC32_MUL 0x04c117db //多项式x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
unsigned int CRC_Table[256];
void InvertUint8(unsigned char *dbuf,unsigned char *srcbuf)
{
	unsigned char i=0;
	unsigned char temp=0x00;
	for(i=0;i<8;i++)
	{
		temp=temp>>1;
		if(*dbuf&0x80)
		{
			temp|=0x80;
		}
		*dbuf<<=1;
	}
	*srcbuf = temp;
}
void InvertUint16(unsigned short *dbuf,unsigned short *srcbuf)
{
	unsigned char i=0;
	unsigned short temp=0x00;
	for(i=0;i<16;i++)
	{
		temp=temp>>1;
		if(*dbuf&0x8000)
		{
			temp|=0x8000;
		}
		*dbuf<<=1;
	}
	*srcbuf = temp;
}
void CRC8_TableInit(unsigned char InitVal)
{
	unsigned char CRC_Val,LoopTime;
	unsigned int CRC_Index;
	CRC_Val = InitVal;
	for(CRC_Index=0;CRC_Index<256;CRC_Index++)
	{
		CRC_Val = CRC_Val ^ CRC_Index;
		for(LoopTime=0;LoopTime<8;LoopTime++)
		{
			if(CRC_Val&0X80)
			{
				CRC_Val = (CRC_Val<<1)^CRC8_MUL;
			}
			else
			{
				CRC_Val <<=1;
			}
		}
		CRC_Table[CRC_Index] = CRC_Val;
		CRC_Val = InitVal;
	}
}

void CRC16_TableInit(unsigned int InitVal)
{
	unsigned char LoopTime;
	unsigned int CRC_Index,CRC_Val;
	CRC_Val = InitVal;
	for(CRC_Index=0;CRC_Index<256;CRC_Index++)
	{
		CRC_Val = CRC_Val ^ (CRC_Index<<8);
		for(LoopTime=0;LoopTime<8;LoopTime++)
		{
			if(CRC_Val&0X8000)
			{
				CRC_Val = (CRC_Val<<1)^CRC16_MUL;
			}
			else
			{
				CRC_Val <<=1;
			}
		}
		CRC_Table[CRC_Index] = CRC_Val&0XFFFF;
		CRC_Val = InitVal;
	}
}
/*
	CRC15_X25
	初始值为0XFFFF,高位在前，低位在后，最后的结果与0XFFFF异或，输入输出的数值都要进行字节取反。
	生成表格时，初始值为0，结果也不需要进行异或
*/
void CRC16X25_TableInit(unsigned int InitVal)
{
	unsigned char LoopTime,tempCrc;
	unsigned short CRC_Val,CRC_Index;
	for(CRC_Index=0;CRC_Index<256;CRC_Index++)
	{
	    CRC_Val = InitVal;
		tempCrc = (unsigned char)CRC_Index;
		InvertUint8(&tempCrc,&tempCrc);
		CRC_Val = CRC_Val ^((unsigned short)(tempCrc)<<8);
		for(LoopTime=0;LoopTime<8;LoopTime++)
		{
			if(CRC_Val&0X8000)
			{
				CRC_Val = (CRC_Val<<1)^CRC16_MUL;
			}
			else
			{
				CRC_Val <<=1;
			}
		}
		InvertUint16(&CRC_Val,&CRC_Val);
		CRC_Table[CRC_Index] = CRC_Val;
	}
}
void CRC32_TableInit(unsigned int InitVal)
{
	unsigned char LoopTime;
	unsigned int CRC_Index,CRC_Val;
	CRC_Val = InitVal;
	for(CRC_Index=0;CRC_Index<256;CRC_Index++)
	{
		CRC_Val = CRC_Val ^ (CRC_Index<<24);
		for(LoopTime=0;LoopTime<8;LoopTime++)
		{
			if(CRC_Val&0X80000000)
			{
				CRC_Val = (CRC_Val<<1)^CRC32_MUL;
			}
			else
			{
				CRC_Val <<=1;
			}
		}
		CRC_Table[CRC_Index] = CRC_Val;
		CRC_Val = InitVal;
	}
}
//直接计算法--网上摘抄
unsigned short CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen)
{
  unsigned short wCRCin = 0xFFFF;
  unsigned short wCPoly = 0x1021;
  unsigned char wChar = 0;
  
  while (usDataLen--) 	
  {
        wChar = *(puchMsg++);
        InvertUint8(&wChar,&wChar);
        wCRCin ^= (wChar << 8);
        for(int i = 0;i < 8;i++)
        {
          if(wCRCin & 0x8000)
            wCRCin = (wCRCin << 1) ^ wCPoly;
          else
            wCRCin = wCRCin << 1;
        }
  }
  InvertUint16(&wCRCin,&wCRCin);
  return (wCRCin^0xFFFF) ;
}
//查表法
unsigned short crc16x25_Check(unsigned char *pdata,unsigned int len)
{
	unsigned int count=0;
	unsigned short crc=0XFFFF;  //设定初值为0xFFFFF
	for(count=0;count<len;count++)
	{
		crc = (crc>>8) ^ CRC_Table[((crc) ^ *(unsigned char *)pdata++)&0x00FF];
	}
	crc = crc^0xffff;
	return crc;
}

int main()
{
    unsigned int CRC_Index,i;
	unsigned char dat=0x20;
	CRC_Index = 0x20;
	unsigned char testdat[10]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    CRC16X25_TableInit(0);
    printf("Result=0x%04x\n",CRC16_X25(&dat,1));
	printf("Result=0x%04x\n",crc16x25_Check(&dat,1));
    for(CRC_Index=0;CRC_Index<32;CRC_Index++)
    {
        for(i=0;i<8;i++)
        {
           printf("0x%04x,    ",CRC_Table[CRC_Index*8+i]);
        }
        printf("\n");
    }  
	printf("crctestdat=0x%04x\n",crc16x25_Check(testdat,10));
	printf("crctestdat=0x%04x\n",CRC16_X25(testdat,10));
}
