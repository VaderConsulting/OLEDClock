// References
// https://github.com/lesterlo/Notched-Shaft-Encoder
// https://forum.arduino.cc/index.php?topic=681420.0
// https://www.skaarhoj.com/FreeStuff/GraphicDisplayImageConverter.php
// ==============================INCLUDES=========================================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#include <MsTimer2.h>
#include <NSEncoder.h>

#include <TM1637TinyDisplay.h>

// ======================================================================
// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======================================================================
// Rotary Encoder Inputs
#define ENCODER_S1_PIN 2
#define ENCODER_S2_PIN 3
#define ENCODER_KEY_PIN 10

// *******************************ENUMS**********************************

typedef enum
{
  Running,
  Set_Hours,
  Set_Minutes
} ClockStates;

// *******************************FIELDS*********************************
// Clock
RTC_DS1307 rtc;
DateTime LastTime;
ClockStates ClockState;
ClockStates LastClockState;

// **********************************************************************
// Rotary encoder
//                    A,               B,             Btn, StepsPerNotch, EnableDoublePress
NSEncoder_btn enc(ENCODER_S1_PIN, ENCODER_S2_PIN, ENCODER_KEY_PIN, 4, false);
int enc_position = 0;
int Last_enc_position = 0;
String display_str = "Open";

//Timer isr
void isr_mstimer2()
{
  enc.btn_task();
}

void setup() {
  while (!Serial); // for Leonardo/Micro/Zero
  {
    Serial.begin(57600);
  }

  // **********************************************************************
  //Clock
  if (!rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // **********************************************************************
  // Display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen
  oled.display();
  delay(5000); // Pause for 5 seconds

  oled.setTextSize(3);
  oled.setTextColor(WHITE);
  
  oled.dim(true); // or false

  // **********************************************************************
  // Rotary encoder
  // Set the button monitoring parameter

  enc.setBtnDebounceInterval(30);
  enc.setBtnPressInterval(500);
  enc.setBtnHoldInterval(1000);

  //You must setup this timer to enable button monitoring
  MsTimer2::set(enc.getBtnDebounceInterval(), isr_mstimer2); //Set 30 ms clock
  MsTimer2::start();

  ClockState = Running;
  Serial.println("Clock: Running");
}

void loop() {
  DateTime now = rtc.now();
  
  // **********************************************************************
  NSEncoder_btn::BTN_STATE enc_btn;

  if(enc.get_Position(enc_position)) //If value is updated
  {
      Serial.print("Enc. Pos: ");
      Serial.println(enc_position);

      if (enc_position > Last_enc_position)
      {
        Serial.println("INCREASE");

        uint32_t UnixTime = now.unixtime();
        switch (ClockState)
        {
          case Set_Hours: 
            rtc.adjust(UnixTime + 3600);
            break;
          case Set_Minutes:
            rtc.adjust(UnixTime + 60);
            ResetSeconds(rtc.now());
            break;
        }
      }
      else
      {
        Serial.println("DECREASE");

        uint32_t UnixTime = now.unixtime();
        switch (ClockState)
        {
          case Set_Hours: 
            rtc.adjust(UnixTime - 3600);
            break;
          case Set_Minutes:
            rtc.adjust(UnixTime - 60);
            ResetSeconds(rtc.now());
            break;
        }
      }

      now = rtc.now();
      Last_enc_position = enc_position;
  }

  enc_btn = enc.get_Button();
  switch(enc_btn)
  {
    case NSEncoder_btn::OPEN:
      display_str = "Open";
    break;
    
    case NSEncoder_btn::PRESSED:
      display_str = "Pressed";

      switch (ClockState)
      {
        case Set_Hours: // Hours -> Minutes
          ClockState = Set_Minutes;
          break;
        case Set_Minutes: // Minutes -> Running
          ClockState = Running;
          break;
      }
      break;

    case NSEncoder_btn::RELEASED:
      display_str = "Released";
      break;
    
    case NSEncoder_btn::HELD:
      display_str = "Held";
      switch (ClockState)
      {
        case Running:
          ClockState = Set_Hours;
          break;
        default:
          ClockState = Running;
          break;
      }
      break;
    
    case NSEncoder_btn::DOUBLE_PRESSED:
      display_str = "Double Pressed";
      break;
  }
  
  if(enc_btn != NSEncoder_btn::OPEN)
  {
    Serial.print("Btn status: ");
    Serial.println(display_str);
  }

  //now = rtc.now();

  if (LastTime != now)
  {
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);
  
    oled.clearDisplay();
    oled.setCursor(5, 10);

    // If we are setting the hours, invert the text
    if (ClockState == Set_Hours)
    {
      oled.setTextColor(BLACK, WHITE);
    }
    else
    {
      oled.setTextColor(WHITE);
    }
    
  // HOUR
    if (now.hour() < 10)
    {
      oled.print(' ');
      oled.print(now.hour(), DEC);
    }
    else if (now.hour() > 12)
    {
      oled.print(' ');
      oled.print(now.hour() - 12, DEC);
    }
    else
    {
      oled.print(now.hour(), DEC);
    }

    // The colon is always rendered normally
    oled.setTextColor(WHITE);
    oled.print(':');

  // If we are setting the minutes, invert the text
    if (ClockState == Set_Minutes)
    {
      oled.setTextColor(BLACK, WHITE);
    }
    else
    {
      oled.setTextColor(WHITE);
    }
  
  // MINUTE
    if (now.minute() < 10)
    {
      oled.print('0');
      oled.print(now.minute(), DEC);
    }
    else
    {
      oled.print(now.minute(), DEC);
    }
  
    oled.display();
    
    LastTime = now;
  }

  if (LastClockState != ClockState)
  {
    switch (ClockState)
    {
      case Running:
        Serial.println("Clock: Running");
        break;
      case Set_Hours:
        Serial.println("Clock: Set_Hours");
        break;
      case Set_Minutes:
        Serial.println("Clock: Set_Minutes");
        break;
    }
    LastClockState = ClockState;
    delay(500);
  }
}

void ResetSeconds(DateTime now)
{
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), 0));
}
