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
 * RFIDLibrary.ino
 *
 * Created: 11/14/2017 7:26:06 PM
 *  Author: Jay Wilhelm jwilhelm@ohio.edu
 */ 
#include <Arduino.h>
#include "ManchesterDecoder.h"
#include "ETAGLowPower.h"
#include <RTCZero.h>

#define BUTTON_PIN  3 //5 for the xplained D21
#define pLED 13

//#define EM4095
//ETAG BOARD
#define serial SerialUSB
#define ShutdownPin 8
#define demodOut 30

/*#define serial Serial
#define ShutdownPin 7 //test board
#define demodOut 8 */

#define STANDARD_BUFFER_FILL_TIME 100 //tweak the dwell time (ms)

#define CollectedBitMinCount 120 //used to tweak read speed, reduces chances of good read
ManchesterDecoder gManDecoder(demodOut,ShutdownPin,ManchesterDecoder::EM4095,CollectedBitMinCount);

/* Create an rtc object */
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 25;
const byte month = 9;
const byte year = 15;
void alarmMatch()
{
  //do nothing at alarm but cause wakeup
}
void SetRTC_AlarmDelta(void)
{
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(16, 0, 10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
    
  rtc.attachInterrupt(alarmMatch);
  // Set the XOSC32K to run in standby
  SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;

  // Configure EIC to use GCLK1 which uses XOSC32K
  // This has to be done after the first call to attachInterrupt()
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_EIC) |
  GCLK_CLKCTRL_GEN_GCLK1 |
  GCLK_CLKCTRL_CLKEN;   
  
  //gManDecoder.EnableMonitoring();
}
void setup() 
{
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,HIGH);
  
	serial.begin(115200);
	serial.println("running");

  //pinMode(BUTTON_PIN,INPUT);
  //attachInterrupt(BUTTON_PIN, ding, RISING);
  
  ETAGLowPower::LowPowerSetup();
  rtc.begin(); // initialize RTC 24H format
}


void loop() 
{  
  gManDecoder.DisableMonitoring();
  gManDecoder.ChipOff();  
  SetRTC_AlarmDelta();
  //enter low power mode
  //configured board used GPIO pins
  //slows the clocks
  //disables the systick (causes wakeups)
  //sleep
  //wakeup
  //power the clocks back up
  //setup USB 
  ETAGLowPower::PowerDownSleepWait();
  //exited out of sleep and back running
  
  gManDecoder.ChipOn();
  gManDecoder.EnableMonitoring(); //re-enable the interrupt
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on 

  delay(5000);//allow USB to show up
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off 

  delay(STANDARD_BUFFER_FILL_TIME);
  //digitalWrite(PIN_LED,!digitalRead(PIN_LED));
  serial.print("Check: ");
  serial.println(gManDecoder.GetBitIntCount());
	int p_ret = gManDecoder.CheckForPacket();//check if there is data in the interrupt buffer
	
	if(p_ret > 0)
	{
		EM4100Data xd; //special structure for our data
		int dec_ret = gManDecoder.DecodeAvailableData(&xd); //disable the interrupt and process available data
		if(dec_ret == DECODE_PARITY_FAILED)
    {
      serial.println("!Packet found, but failed parity");
      PrintTagID(xd);
    }
    else if(dec_ret != DECODE_FOUND_GOOD_PACKET)
    {
      serial.println("!Packet not found");
      return;
    }
    else{
      serial.println("!Packet found good");
      //PrintTagID(xd);
      String bintag = gManDecoder.GetFullBinaryString(xd);
      String hextag = gManDecoder.GetDecodedHexNumberAsString(xd);
      serial.println(hextag);
      uint32_t tagNum = gManDecoder.GetTagNumber(xd);
      serial.println(tagNum);
      uint8_t cardID = gManDecoder.GetCardIDNumber(xd);
      serial.println(cardID);
    }
	}
}
