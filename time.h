
/*
Copyright (c) 2021 by M5Stack
edited by katynkafialova for the purpose of the project Dropposite
*/
#include <M5Core2.h>

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;

char timeStrbuff[64];
void print_time();

void readTime(){
    M5.Rtc.GetTime(&RTCtime); //Gets the time in the real-time clock.
    M5.Rtc.GetDate(&RTCDate);
    print_time();
}

void print_time(){
   sprintf(timeStrbuff,"%d/%02d/%02d %02d:%02d:%02d",  //Stores real-time time and date data to timeStrbuff.   
                        RTCDate.Year,RTCDate.Month,RTCDate.Date,
                        RTCtime.Hours,RTCtime.Minutes,RTCtime.Seconds);
    M5.lcd.setCursor(3,200); //Move the cursor position to (x,y). 
    M5.Lcd.println(timeStrbuff);  //Output the contents of.  
}


void setupTime(){
  M5.Rtc.SetTime(&RTCtime); //and writes the set time to the real time clock.  

  RTCDate.Year = 2021;  //Set the date. 
  RTCDate.Month = 12;
  RTCDate.Date = 13;
  M5.Rtc.SetDate(&RTCDate);
}


bool time_setup = false, editing_hours =false, editing_min = false, editing_sec = false;
void set_time();


#define DELAY 500

void set_time(){
  print_time();
  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 1000)) {
    // Add value  
    M5.Lcd.println("Button A pressed");
    if(editing_hours){
        RTCtime.Hours++;
    }else if(editing_min){
        RTCtime.Minutes++;
    }else if(editing_sec){
        RTCtime.Seconds++;
    }
    delay(DELAY);
  } else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 1000)) {
    // Suctract value
    M5.Lcd.println("Button B pressed");
    if(editing_hours){
        RTCtime.Hours--;
    }else if(editing_min){
        RTCtime.Minutes--;
    }else if(editing_sec){
        RTCtime.Seconds--;
    }
    delay(DELAY);
  } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 1000)) {
  // Moves cursor
  M5.Lcd.println("Button C pressed");
    if(editing_hours){
        editing_hours = false;
        editing_min = true;
    }else if(editing_min){
        editing_min = false;
        editing_sec = true;
    }else if(editing_sec){
        editing_sec = false;
        time_setup = false;
        M5.Lcd.fillScreen(BLACK);
        setupTime();
    }
    delay(DELAY);
  }else{
    // Npothing was pressed
  }
}
