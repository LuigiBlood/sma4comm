
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>
#include "sma4e.h"

#include "level_bin.h"
#include "power_bin.h"
#include "demo_bin.h"

static const char* gameID = SMA4E_COMM_GAMEID_E;
static u16 sendData = 0;
static u16 recvData = 0;
static u32 frameCounter = 0;
static u8 isIRQSerial = 0;
static u8 isDataSent = 0;
static u8 _ignoreVBlank = 0;
static u8 cardType = 0;

void serialIRQ() {
	//Send
	if (isDataSent)
	{
		REG_SIOMLT_SEND = sendData;
		isDataSent = 0;
	}
	
	//Recv
	if (!(REG_SIOCNT & BIT(6)))
		recvData = REG_SIOMULTI0;
	
	//Acknowledge
	if (!(REG_SIOCNT & BIT(6)))
		isIRQSerial = 1;
	REG_IF = IRQ_SERIAL;
}

void sendWord(u16 word)
{
	sendData = word;
	isDataSent = 1;
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
		isDataSent = 1;
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
			isDataSent = 1;
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

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	u8 mode = 0;
	char strText[31] = "";
	u32 offset = 0;
	u16* data;
	u32 checkSum = 0;

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	irqDisable(IRQ_SERIAL);
	
	isIRQSerial = 0;
	
	REG_RCNT = R_MULTI;
	REG_SIOCNT = SIO_115200 | SIO_MULTI | SIO_IRQ;
	
	irqSet(IRQ_SERIAL, serialIRQ);
	irqEnable(IRQ_SERIAL);

	consoleDemoInit();

	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	iprintf("\x1b[10;10HSMA4 US Test\n");
	VBlankIntrWait();
	while (!(REG_SIOCNT & SIO_SO_HIGH));

	while (1) {
		sprintf(strText, "\x1b[12;12HS:%04X\n", sendData);
		iprintf(strText);
		sprintf(strText, "\x1b[13;13HR:%04X\n", recvData);
		iprintf(strText);
		sprintf(strText, "\x1b[14;14HF:%04X\n", frameCounter);
		iprintf(strText);
		
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
					case SMA4E_COMM_CONNECT:
					case SMA4E_COMM_RECONNECT:
						sendWord(SMA4E_COMM_ID_FRA);
						offset = 0;
						checkSum = 0;
						break;
					case SMA4E_COMM_ID_FRA:
						//sendWord(*(u16*)gameID);
						sendArrayAndChecksum(gameID, 4, 0);
						break;
					//case 0x5841:
						//sendWord(*(u16*)(gameID+2));
						//break;
						
					case SMA4E_COMM_REQ_LVL:
						cardType = 0;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						break;
					case SMA4E_COMM_REQ_PWR:
						cardType = 1;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						break;
					case SMA4E_COMM_REQ_DMO:
						cardType = 2;
						sendWord(SMA4E_COMM_LAKITU_GONE);
						break;
						
					case SMA4E_COMM_LAKITU_MOVE:
						sendWord(SMA4E_COMM_LAKITU_MOVE);
						mode = 1;
						//frameCounter = 0x5E;
						frameCounter = 0x10;
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
					if (cardType == 0)
						data = (u16*)level_bin;
					else if (cardType == 1)
						data = (u16*)power_bin;
					else
						data = (u16*)demo_bin;
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
					mode = 0;
				}
			}
		}
	}
}


