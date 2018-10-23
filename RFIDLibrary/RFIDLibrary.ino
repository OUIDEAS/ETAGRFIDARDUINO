/****************************************************************************
 *
 *   Copyright (c) 2017 Jay Wilhelm. All rights reserved.
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

#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)

/* Create an rtc object */
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 25;
const byte month = 9;
const byte year = 15;
void ding()
{
	volatile int x = 43;
	x++;
}
void alarmMatch()
{
	volatile int x = 43;
	x++;
	//Serial.println("Alarm Match!");
}
void LowPower_SetUSBMode(void)
{
	//begin USB disable for low power
	USBDevice.detach();
	USBDevice.standby();
	USB->DEVICE.CTRLA.bit.RUNSTDBY = 0;
	PM->APBBMASK.reg &= ~PM_APBBMASK_USB;
	
	USB->DEVICE.CTRLA.bit.SWRST = 1;	//reset
	USB->DEVICE.CTRLA.bit.ENABLE = 0;	//disable
	USB->DEVICE.CTRLB.bit.UPRSM = 0;	//nowake host
	USB->DEVICE.CTRLB.bit.SPDCONF = 0;
	#ifdef PIN_LED_TXL
	pinMode(PIN_LED_TXL, INPUT);
	pinMode(PIN_LED_RXL, INPUT);
	#endif
	PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 0;
	PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 0;
	//end USB disable
}
void SetRTC_AlarmDelta(void)
{
	rtc.setTime(hours, minutes, seconds);
	rtc.setDate(day, month, year);

	rtc.setAlarmTime(16, 0, 10);
	rtc.enableAlarm(rtc.MATCH_HHMMSS);
}

void LowPower_SetGPIO(void)
{
	//PA21 SD_CS + RTC INT (ext pullup) (input)
	//PA22 SCL (ext pullup) (input)
	//PA23 SDA (ext pullup) (input)
	pinMode(7,INPUT);
	pinMode(16,INPUT);
	pinMode(17,INPUT);

	//PA08 Flash CS
	//PB10 MOSI (SD, Flash)
	//PB11 SCK (SD, Flash)
	//PA12 MISO (SD, Flash)
	//PA28 SD FET (low)
	pinMode(2,INPUT);
	pinMode(21,OUTPUT);	//THIS ONE CAUSES HIGH POWER IF INPUT
	digitalWrite(21,LOW);
	pinMode(20,OUTPUT);digitalWrite(20,HIGH);//THIS ONE CAUSES HIGH POWER IF INPUT
	pinMode(18,INPUT);
	pinMode(32,INPUT);

	//PA27 LED
	pinMode(LED_BUILTIN,OUTPUT);
	digitalWrite(LED_BUILTIN,HIGH);
	

	//PB03 RFID Read
	//PA07 RFID Shutdown #2
	//PA06 RFID Shutdown #1
	pinMode(30,OUTPUT);digitalWrite(30,LOW);//THIS ONE CAUSES HIGH POWER IF INPUT
	//return; //915 to 866uA here
	
	
	pinMode(46,INPUT);
	pinMode(8,INPUT);
	//The following change from INPUT to OUTPUT causes the RFID chip to pulse on/off at 100 mA

	/*pinMode(46,OUTPUT);
	pinMode(8,OUTPUT);
	digitalWrite(46,HIGH);//EM4095 HIGH = off, LOW = on, but at increased power
	digitalWrite(8,HIGH);*/

	
	//PA02 Battery read
	pinMode(24,INPUT);

	
	//PA24 USB+	
	//PA25 USB-
	//powered from battery -> ~7mA when active
	//powered from battery -> 750 uA (PowerDebugger)
	//powered from USB -> 580 uA (PowerDebugger)
	//powered from USB -> 11 mA (PowerDebugger)
}


void setup() 
{
	LowPower_SetUSBMode(); //has to be here or something else upsets low power

  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,HIGH);
  
	serial.begin(115200);
	serial.println("running");

  pinMode(BUTTON_PIN,INPUT);
  attachInterrupt(BUTTON_PIN, ding, RISING);
  rtc.begin(); // initialize RTC 24H format
  SetRTC_AlarmDelta();
  
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

void PowerDownSleepWait()
{
  LowPower_SetGPIO();
  LowPower_SetUSBMode();

  //digitalWrite(LED_BUILTIN,digitalRead(5));
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;
  
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __DSB();
  __WFI();
  
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
  
  SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk;
  delay(50);
  USBDevice.init();
  USBDevice.attach();
}
void loop() 
{  
  gManDecoder.DisableMonitoring();
  gManDecoder.ChipOff();  
  SetRTC_AlarmDelta();
  PowerDownSleepWait();
  //exited out of sleep and back running
  
  gManDecoder.ChipOn();
  gManDecoder.EnableMonitoring(); //re-enable the interrupt
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on 

  delay(2500);//allow USB to show up
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off 

  delay(STANDARD_BUFFER_FILL_TIME);
  //digitalWrite(PIN_LED,!digitalRead(PIN_LED));
  serial.print("Check: ");
  serial.println(gManDecoder.GetBitIntCount());
	int p_ret = gManDecoder.CheckForPacket();//check if there is data in the interrupt buffer
	if(p_ret == 0)
  {
    gManDecoder.EnableMonitoring(); //re-enable the interrupt
    delay(STANDARD_BUFFER_FILL_TIME);
    p_ret = gManDecoder.CheckForPacket();
    serial.println("Second Check");
  }
	
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
