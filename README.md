# OLEDClock

Arduino clock that draws time on a 128×32 SSD1306 OLED via Adafruit GFX/SSD1306, with a DS1307 RTC and a notched-shaft encoder to set hours and minutes. MsTimer2 services the encoder button. Visual Micro `.vcxproj` is included so the sketch can be opened from Visual Studio.

**Source last updated:** 2022-05-08  
**Language:** C++ / Arduino  
**Target:** Arduino AVR with I2C OLED + RTC and rotary encoder  
**Output:** Arduino sketch plus Visual Micro `.vcxproj`

## Solution structure

| Project | Language | Type | Purpose |
|---------|----------|------|---------|
| `OLED_Clock` | C++ / Arduino | sketch | SSD1306 OLED clock with DS1307 RTC and encoder set-time modes |
| `OLED_Clock.vcxproj` | C++ | Visual Micro project | Opens the sketch inside Visual Studio |

## How to open

Open `OLED_Clock/OLED_Clock.ino` in the Arduino IDE, or `OLED_Clock/OLED_Clock.vcxproj` in Visual Studio with Visual Micro.

## Requirements

- Arduino IDE
- Visual Studio 2010 or later

## Attribution and provenance

Dave Robinson / VaderConsulting sketch from the Arduino archive. Uses Adafruit GFX/SSD1306, RTClib, MsTimer2, and Lester Lo NSEncoder (sibling library repos).

## License

MIT © 2026 VaderConsulting for Dave's sketch. See `LICENSE`.
