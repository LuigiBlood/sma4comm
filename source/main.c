
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sma4e.h"

#include "level_bin.h"
#include "power_bin.h"
#include "demo_bin.h"

#include "cardCollection.h"//Trying to compile? Run the Card Collection Compiler in the tools folder to create this file.



static const char* gameID = SMA4E_COMM_GAMEID_E;
static u16 sendData = 0;
static u16 recvData = 0;
static u32 frameCounter = 0;
static u8 isIRQSerial = 0;
static u8 isDataSent = 0;
static u8 _ignoreVBlank = 0;
static u8 cardType = 0;

static u8 OFFSET_DELAY = 0;


static int dotCodeIndex = 0;
int keys_pressed, keys_released;


void serialIRQ() {
	
	
	//Recv
	if (!(REG_SIOCNT & BIT(6)))
		recvData = REG_SIOMULTI0;
	
	//Acknowledge
	if (!(REG_SIOCNT & BIT(6)))
		isIRQSerial = 1;
	REG_IF = IRQ_SERIAL;
	
	//Send
	if (isDataSent > 0)
	{
		//u16 tmp = isDataSent;
		isDataSent--;
		REG_SIOMLT_SEND = sendData;
		if (isDataSent > 0)
		{
			IntrWait(0, IRQ_SERIAL);
		}
		/*for (u16 i = 0; i < tmp; i++)
		{
			IntrWait(0, IRQ_SERIAL);
		}*/
		
		
		
	}
}

void sendWord(u16 word)
{
	sendData = word;
	isDataSent = 1+OFFSET_DELAY;
}

u16 recvWord()
{
	return recvData;
}



void sendArrayAndChecksum(u16* data, u32 size, u8 check)
{
	u32 checkSum = 0;
	irqDisable(IRQ_VBLANK);
	for (u32 i = 0; i < size; i += 2)
	{
		IntrWait(0, IRQ_SERIAL);
		isIRQSerial = 0;
		if (!check)//For some reason, the game id part needs a delay in the IRQ sending, but nothing else.
		{
			isDataSent = 2+OFFSET_DELAY;
		}
		else
		{
			isDataSent = 1+OFFSET_DELAY;
		}
		sendData = *data;
		checkSum += *data;
		data += 1;
		
		
		
		
		
	}
	if (check)
	{
		for (u32 i = 0; i < 2; i++)
		{
			IntrWait(0, IRQ_SERIAL);
			isIRQSerial = 0;
			isDataSent = 1+OFFSET_DELAY;
			if (i == 0)
			{
				sendData = (u16)checkSum;
			}
			else
			{
				sendData = (u16)(checkSum >> 16);
			}
		}
	}
	irqEnable(IRQ_VBLANK);
}

void combine_strings(char* s1, char* s2, char* result) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int i, j = 0;
    for (i = 0; i < len1 && i+j < 64; i++) {
        result[i+j] = s1[i];
    }
    for (j = 0; j < len2 && i+j < 64; j++) {
        result[i+j] = s2[j];
    }
    result[i+j] = '\0';
}
int wrapAround(int num, int length) {
	if (length == 0)
	{
		return num;
	}
    while (num < 0) {
        num += length;
    }
    while (num >= length) {
        num -= length;
    }
    
    return num;
    
}
void resetIRQ()
{
	
	irqInit();
	irqEnable(IRQ_VBLANK);
	irqDisable(IRQ_SERIAL);
	
	isIRQSerial = 0;
	
	REG_RCNT = R_MULTI;
	REG_SIOCNT = SIO_115200 | SIO_MULTI | SIO_IRQ;
	
	irqSet(IRQ_SERIAL, serialIRQ);
	irqEnable(IRQ_SERIAL);
}
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	u8 mode = 255;
	char strText[64] = "";
	u16 debugCode = 0;
	u16 debugWord = 0x0000;
	u16* data;
	
	bool initialized = false;

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	resetIRQ();

	consoleDemoInit();

	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	
	VBlankIntrWait();
	while (!(REG_SIOCNT & SIO_SO_HIGH));

	while (1) {
		
		scanKeys();

		keys_pressed = keysDown();
		keys_released = keysUp();
		if (mode == 0xFF)
		{
			int pDCI = dotCodeIndex;
			if (keys_pressed & KEY_RIGHT)
			{
				dotCodeIndex++;
			}
			else if (keys_pressed & KEY_LEFT)
			{
				dotCodeIndex--;
			}
			if (pDCI != dotCodeIndex)
			{
				iprintf("\x1b[2J");//consoleCls();
			}
			if (keys_pressed & KEY_A)
			{
				mode = 0;
				debugCode = 0;
				
				iprintf("\x1b[2J");//consoleCls();
				
				resetIRQ();
				
				
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{//
				int maxDC;
				char moniker[][2] = {"L","P","D"};
				char y[][2] = {"0","2","4"};
				char value[64] = "";
				
				switch (i)
				{
					case 0:
						maxDC = levelTable_Count;//sizeof(levelTable) / sizeof(levelTable[0]);
						strcpy(value,levelTable_Names[wrapAround(dotCodeIndex,maxDC)]);
					break;
					case 1:
						maxDC = powerupTable_Count;
						strcpy(value,powerupTable_Names[wrapAround(dotCodeIndex,maxDC)]);
					break;
					case 2:
						maxDC = demoTable_Count;
						strcpy(value,demoTable_Names[wrapAround(dotCodeIndex,maxDC)]);
					break;
				}//
				
				sprintf(strText,"\x1b[%s;0H%s:%s",y[i],moniker[i],value);//,maxDC,wrapAround(dotCodeIndex,maxDC),dotCodeIndex);
				iprintf(strText);
				iprintf("\x1b[7;0HChoose dot-code to be scanned.\nPress A to lock it in for sending.");
				//,maxDC,wrapAround(dotCodeIndex,maxDC),dotCodeIndex);
				/*for (int a = 0; a < sizeof(strText)/sizeof(strText[0]); a++)
				{
					if (strText[a] == "")
					{
						strText[a] = " ";//Whitespace out leftover text.
					}
				}*/
				
				}
			}
		}

		
		
		iprintf("\x1b[10;10HSMA4 US Test\n");
		
		sprintf(strText, "\x1b[12;12HS:%04X\n", sendData);
		iprintf(strText);
		//registerToLog(strText,debugLog,prevWord);
		sprintf(strText, "\x1b[13;13HR:%04X\n", recvData);
		iprintf(strText);
		//registerToLog(strText,debugLog,prevWord);
		sprintf(strText, "\x1b[14;14HF:%04X\n", (int)frameCounter);
		iprintf(strText);
		//registerToLog(strText,debugLog,prevWord);
		sprintf(strText, "\x1b[15;15HE:%04X\n", debugCode);
		iprintf(strText);
		sprintf(strText, "\x1b[16;16HM:%04X\n", mode);
		iprintf(strText);
		//registerToLog(strText,debugLog,prevWord);
		
		//sprintf(strText,"\x1b[0;0HL:%s\n", debugLog);
		//iprintf(strText);
		
		
		/* for (int i = 0; i < 5; i++) {
        printf("Address of levelTable[%d]: %p\n", i, (void*)levelTable[i]);
		}*/
		
		
		//prevWord = recvWord();
		
		if (!_ignoreVBlank)
		{
			VBlankIntrWait();
			
			if (frameCounter > 0)
				frameCounter--;
		}
		
		if (isIRQSerial)
		{
			isIRQSerial = 0;
			if (mode == 0)
			{
				switch (recvWord())
				{
					case SMA4E_COMM_REPEATPLEASE://Hardware only.
						if (initialized)
						{
							isDataSent = 1;//Keep delaying if they call FFFF.
						}
						else
						{
							sendWord(SMA4E_COMM_ID_USA);
						}
						//sendWord(0);
					break;
					case SMA4E_COMM_CONNECT:
					case SMA4E_COMM_RECONNECT:
						initialized = true;
						sendWord(SMA4E_COMM_ID_USA);
						debugCode = 1;
						break;
					case SMA4E_COMM_ID_USA:
					
						//sendWord(*(u16*)gameID);
						sendArrayAndChecksum(gameID, 4, 0);//Two bytes, 0x5841 and 0x4534. AX4E with endian order swapped? 
						debugWord = debugWord + 1;
						debugCode = 2;
						//frameCounter = 2;
						break;
					case SMA4E_COMM_ID_FRA:
						//sendWord(*(u16*)gameID);
						sendArrayAndChecksum(gameID, 4, 0);
						debugCode = 3;
						break;
					case 0x5841://Don't know why... Lakitu Status request?
					//case 0x4534:
						/*if (debugCode > 0)
						{
							if (debugCode < 0xE0)
							{
								debugCode = debugCode + 0xE0;
							}
							sendArrayAndChecksum(gameID, 4, 0);//sendWord(recvWord());
						}*/
						/*
						0x5841,
0xFBFB,
0x4534,Matches the commented code, but why does it fail???
0xF3F3
4 possible answers. Unlikely the first since why would we resend the protocol when we already have on connect?
						*/
						//sendWord(*(u16*)(gameID+2));
						//sendWord(recvWord());//sendArrayAndChecksum(recvWord(), 4, 0);
						//sendArrayAndChecksum(gameID+2, 4, 0);
						//sendArrayAndChecksum(SMA4E_COMM_ID_ENG,4,0);
						
						//sendWord(debugWord);//*(u16*)(gameID+2));
						//sendWord(SMA4E_COMM_ID_USA);
					/*if (debugCode == 2)
					{
						sendWord(*(u16*)(gameID+2));
					}
						debugCode = 8;*/
						//sendWord(0x5841);//*(u16*)(gameID+2));//0xF3F3);//0x4534);//*(u16*)(gameID+2));
						break;
						
					case SMA4E_COMM_REQ_LVL:
						cardType = 0;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						debugCode = 4;
						break;
					case SMA4E_COMM_REQ_PWR:
						cardType = 1;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						debugCode = 5;
						break;
					case SMA4E_COMM_REQ_DMO:
						cardType = 2;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						debugCode = 6;
						break;
						
					case SMA4E_COMM_LAKITU_MOVE:
						sendWord(SMA4E_COMM_LAKITU_MOVE);
						mode = 1;
						//frameCounter = 0x5E;
						frameCounter = 0x10;
						debugCode = 7;
						break;
					case SMA4E_COMM_LAKITU_HERE:
						sendWord(SMA4E_COMM_ID_USA);
					break;
					default:
						//sendWord(0);
						if (debugCode < 0x90)
						{
							debugCode = debugCode + 0x90;
						}
						
						break;
				}
			}
			else if (mode == 1)
			{
				if (frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_LAKITU_HERE);
					mode = 2;
					frameCounter = 0x10;
				}
			}
			else if (mode == 2)
			{
				if (recvWord() == SMA4E_COMM_WAIT_LVL && frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_SCANNED);
					mode = 3;
					frameCounter = 5;
				}
				else if (recvWord() == SMA4E_COMM_WAIT_PWR && frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_SCANNED);
					mode = 3;
					frameCounter = 5;
				}
				else if (recvWord() == SMA4E_COMM_WAIT_DMO && frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_SCANNED);
					mode = 3;
					frameCounter = 5;
				}
				else
				{
					sendWord(SMA4E_COMM_LAKITU_HERE);
				}
			}
			else if (mode == 3)
			{
				if (frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_SCANNED);
				}
				
				if (recvWord() == SMA4E_COMM_RDY_DATA)
				{
					_ignoreVBlank = 1;
					sendWord(SMA4E_COMM_SEND_DATA);
					mode = 4;
					if (cardType == 0 && levelTable_Count > 0)
					{
						data = (u16*)(levelTable[wrapAround(dotCodeIndex,levelTable_Count)]);//Refer to the collection as to whether to offset the data for headers or not.
					}
					else if (cardType == 1 && powerupTable_Count > 0)
					{
						data = (u16*)(powerupTable[wrapAround(dotCodeIndex,powerupTable_Count)]);
					}
					else if (demoTable_Count > 0)
					{
						data = (u16*)(demoTable[wrapAround(dotCodeIndex,demoTable_Count)]);
					}
				}
			}
			else if (mode == 4)
			{
				//Send Card Data
				sendArrayAndChecksum(data, 0x7CE, 1);
				mode = 7;
			}
			else if (mode == 5)
			{
				//Cancel
				mode = 8;
				sendWord(SMA4E_COMM_CANCEL);
				frameCounter = 5;
			}
			else if (mode == 7)
			{
				if (recvWord() == SMA4E_COMM_RECV_GOOD)
				{
					_ignoreVBlank = 0;
					mode = 8;
				}
				else
					sendWord(SMA4E_COMM_SENT);
			}
			else if (mode == 8)
			{
				//Lakitu is going back to GBA 1
				if (frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_LAKITU_MOVE);
					mode = 9;
					frameCounter = 60*1;
				}
			}
			else if (mode == 9)
			{
				//Lakitu is on GBA 1
				if (frameCounter <= 0)
				{
					sendWord(SMA4E_COMM_LAKITU_GONE);
					mode = 255;
					initialized = false;
				}
			}
		}
	}
}


