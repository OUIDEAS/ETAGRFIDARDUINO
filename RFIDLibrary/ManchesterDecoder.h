/****************************************************************************
 *
 *   Copyright (c) 2018 Jay Wilhelm. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name ETAGRFIDArduino nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/*
 * ManchesterDecoder.h
 *
 * Created: 11/14/2017 7:26:06 PM
 *  Author: Jay Wilhelm jwilhelm@ohio.edu
 */ 


#ifndef MANCHESTERINCLUDE_H_
#define MANCHESTERINCLUDE_H_

//struct  __attribute__((__packed__)) my_struct {
#pragma pack(push, 1)
typedef struct{
	bool parity:1;
	byte data_nibb:4;
	byte xxxxx:3;
} __attribute__((__packed__)) EM4100Line;
#pragma pack(pop)
#pragma pack(push, 1)
typedef struct{
	EM4100Line lines[10];
	//EM4100Parity parity;
	bool stop_bit:1;
	byte colparity:4;
	byte xxxx:3;
} __attribute__((__packed__)) EM4100Data;
#pragma pack(pop)




void dprintf(char *fmt, ... );

void INT_manchesterDecode(void);
int has_even_parity(uint16_t x,int datasize);
int CheckManchesterParity(EM4100Data *xd);
void PrintTagID(EM4100Data &xd);

#define MinIntSizeForCheck 128

class ManchesterDecoder
{
public:
	#define zShortLow 150
	#define zShortHigh 350

	#define zLongLow 400
	#define zLongHigh 600
	int gFoundPackets = 0;
	volatile uint8_t    gClientPacketBufWithParity[11];
	//volatile uint8_t	gPacketRead = 0;
  enum ChipType{ Unknown=0,EM4095, U2270B};
  enum TimeClass { tUnknown = 0,
    tShort,
  tLong};

private:
	uint8_t mPIN_demodout	=	8,mPIN_shutdown=7;
	uint8_t headerFound = 0;
	uint8_t headerCount = 0;
	uint8_t syncState	= 0;
	uint32_t intCount = 0;

	uint32_t	lastTime;
	uint32_t	secondLastTime;
	int8_t		lastTimeClass;
	//int8_t		secondLastTimeClass;
	int8_t		lastValue;
	int8_t		secondLastValue;
  ChipType  mChipType;
	volatile uint8_t		dataBuf[11];
	volatile uint8_t		dataBinWrite;
	volatile uint8_t		dataBufWrite;
	volatile uint8_t		dataBinCount;
  int                 mMinNumberOfInts=MinIntSizeForCheck;
 
public:
	ManchesterDecoder(uint8_t demodPin,uint8_t shutdownPin,ChipType iChip,int minintnum);
  int CheckForPacket(void); 
  int DecodeAvailableData(EM4100Data *bufout);
  int DisableMonitoring(void);
  void EnableMonitoring(void);
  int GetBitIntCount(void);
  void ChipOn(void);
  void ChipOff(void); 
  uint32_t  GetTagNumber(EM4100Data &xd);
  uint8_t   GetCardIDNumber(EM4100Data &xd);

  String GetDecodedHexNumberAsString(EM4100Data &xd);
  String GetFullBinaryString(EM4100Data &xd);


private:
  void ChipPower(int state);
  
	void ResetMachine();
	int UpdateMachine(int8_t currPin, uint32_t currTime,int8_t timeClass);
	int UpdateMachineUsingClass(int8_t currPin, int8_t timeClass);
	int StoreNewBit(int8_t newB);
	int HandleIntManchester(int8_t fVal, int8_t fTimeClass);


};
#define DECODE_UNKNOWN            0
#define DECODE_FOUND_GOOD_PACKET  1
#define DECODE_NO_PACKET          2
#define DECODE_PARITY_FAILED      4
#define DECODE_NOT_ENOUGH_DATA    -1

#endif /* MANCHESTERINCLUDE_H_ */
