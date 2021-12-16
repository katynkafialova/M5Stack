#define EEPROM_SIZE 1024 * 200 //Total size of the EEPROM memory used
#define BTN_PRESS 2000
#include <M5Core2.h>
#include "audio.h"
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include "whatsapp.h"
#include <math.h>


#define STAT_DFLT 0
#define STAT_SETT 1
#define STAT_ALRT 2
#define STAT_WIFI 3
#define STAT_TIME 4

#define BACK_COLOR BLACK


RTC_TimeTypeDef mytime;
RTC_TimeTypeDef TimeStruct;
float accX = 0.0F;  // Define variables for storing inertial sensor data
float accY = 0.0F;
float accZ = 0.0F;

bool fall = false;

Button* alrt_btns;

uint8_t data_0[DATA_SIZE * 100];

void  update_sensors() {
  M5.IMU.getAccelData(&accX, &accY, &accZ);
};


#define FALL_DELAY      2000  //Time at which a peak will be detected as a fall
#define MAX_RUN_DELAY   1000  //Time at which the running condition is reset
#define MIN_PEAK_DELAY  200    //Minimum time between to high measurements to be considered isolated peaks

#define FALL_THLD       3.0

bool peak_det = false;
bool run_det = false;
bool isok = true;
long peak_time = 0;

void  detect_fall(bool & f) {
  float _ACC = sqrt(accX * accX + accY * accY + accZ * accZ);

  if (isok)
    M5.Lcd.fillCircle(M5.Lcd.width() - 20, 10, 10, GREEN);
  else
    M5.Lcd.fillCircle(M5.Lcd.width() - 20, 10, 10, YELLOW);

  if (f)
    return;  // This function will never reset the fall state, do that elesewhere

  f = false;
  // Show the peak detection state.
  long peak_delay = millis() - peak_time; // Time since the last peak

  if (peak_delay > MIN_PEAK_DELAY) {
    if (_ACC > FALL_THLD) {
      peak_det = true;
      peak_time = millis();
    } else
      peak_det = false;


    if (!peak_det && !isok && (peak_delay > FALL_DELAY)) {
      f = true;
      return;
    }


    if (run_det) {
      isok = true;
      if (peak_det) {
        if (peak_delay < MAX_RUN_DELAY) { //
          run_det = true;
        }
        else {
          run_det = false;
          isok = false; // Might have fallen
        }
      }
      else { // Not peak_det
        if (peak_delay < MAX_RUN_DELAY) { // The user stopped running/ walking
          run_det = true;
        } else { // There is an isolated peak not related to running
          run_det = false;
        }
      }

    }
    else {// !run_det
      if (peak_det) {
        if (peak_delay < MAX_RUN_DELAY) { //
          run_det = true;
          isok = 1;
        } else {
          isok = false; // The person might have fallen
        }
      } else {
        // do nothing with isok
      }


    }
  }
};




void  update_time(RTC_TimeTypeDef & _time) {};

void display_def(RTC_TimeTypeDef & _time) {
  //M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);  //font color_white
  M5.Lcd.setCursor(3, 10); //cursor coordination
  M5.Lcd.setTextSize(3);  //font size_4
  M5.Lcd.print("Welcome,");
  M5.Lcd.setCursor(3, 75);
  M5.Lcd.print("Press left button to go to");
  M5.Lcd.setCursor(75, 125);
  M5.Lcd.setTextColor(YELLOW);  //font color_white
  M5.Lcd.print("settings");
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);  //font size_4
  print_time();

};
void display_sett() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);  //font color_white
  M5.Lcd.setCursor(3, 10); //cursor coordination
  M5.Lcd.setTextSize(2);  //font size_2
  M5.Lcd.print("Press left button to go to wifi settings");
  M5.Lcd.setCursor(3, 75);
  M5.Lcd.print("Press middle button to go to time settings");
  M5.Lcd.setCursor(3, 140);
  M5.Lcd.print("Press middle button to go back");
};

void display_time(RTC_TimeTypeDef & _time) {
  M5.Lcd.fillScreen(BLACK);
  set_time();
};

int okay(int delay_s)
{
  M5.Lcd.clearDisplay();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(3, 30);
  M5.Lcd.printf("Waiting %i s", delay_s );
  delay(delay_s * 1000);
  fall  = false;
  return 1;
};
int nookay(int delay_s)
{
  M5.Lcd.clearDisplay();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(3, 30);
  M5.Lcd.printf("Stay calm,");
  M5.Lcd.setCursor(3, 60);
  M5.Lcd.printf("the ambulance");
  M5.Lcd.setCursor(3, 90);
  M5.Lcd.printf("is coming.");
  delay(delay_s * 1000);
  fall  = false;
  return 1;
};
void display_alrt(RTC_TimeTypeDef & _time, Button* &buttons) {
  ButtonColors on_colors = {RED, WHITE, WHITE};
  ButtonColors off_colors = {BLUE, WHITE, WHITE};

  if (buttons == NULL) {
    Serial.println("Defining the ALRT UI.");
    buttons = new Button[2] {
      Button(0, 60, 160, 180, false , "YES", off_colors, on_colors, TL_DATUM),
      Button(160, 60, 160, 180, false, "NO", off_colors, on_colors, BL_DATUM)
    };
  }

  Serial.println("Drawing ALRT UI");
  M5.Lcd.fillScreen(WHITE); // background  color_white
  buttons[0].draw();
  buttons[1].draw();
  Serial.println("DefiniFinished drawing the buttons.");
  M5.Lcd.fillRect(0, 0, 320, 60, BLACK);
  M5.Lcd.setTextColor(WHITE);//text color_white
  M5.Lcd.setCursor(45, 30);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("Are you okay?");
  M5.Lcd.setTextSize(4);
  Serial.println("Finished Drawing ALRT UI");

  delay(1000);
};

void display_wifi() {
  M5.Lcd.setCursor(60, 50);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(WHITE);//text color_white
  if (WiFi.status() == WL_CONNECTED)
  {
    M5.Lcd.printf("Connected");
  } else if (WiFi.status() != WL_CONNECTED)
  {
    M5.Lcd.printf("Disconnected");
  }

};


int read_UI_def() {
  int next_state = STAT_DFLT;
  if (M5.BtnA.wasReleased() ) {
    next_state = STAT_SETT;
  } else if (M5.BtnB.wasReleased() ) {
    // Do somehting
  } else if (M5.BtnC.wasReleased() ) {
    // Do somehting
  }


  return next_state;
}

int read_UI_sett() {
  int next_state = STAT_SETT;

  if (M5.BtnA.wasReleased() ) {
    next_state = STAT_WIFI;
  } else if (M5.BtnB.wasReleased()) {
    next_state = STAT_TIME;
  } else if (M5.BtnC.wasReleased()) {
    next_state = STAT_DFLT;
  }


  return next_state;
}

int read_UI_alrt(Button * btns) {
  int next_state = STAT_ALRT;

  while (1) {
    M5.update();
    Event& e = M5.Buttons.event;
    if (e & (E_MOVE | E_RELEASE), btns[0]) //yes ask again in 30seconds
    {
      int ok = okay(5); // Ask again after 30 seconds and return 1 if its ok
      if (ok) {
       
        Serial.printf("Send Ok Message. Next State = %i \n", next_state);
        message_to_whatsapp("There was a small accident of falling, however, based on the response of the patient everything should be okay. We recommend to check up on the patient soon, preferably within next 10 minutes and again in an hour. Have a nice day.");
        next_state = STAT_DFLT;
        delete [] btns;
        return next_state;
        break;
      }
      else {
        Serial.println("Send Warning Message.");
        message_to_whatsapp("There was an accident of falling and the user didnt respond. Please get in touch.");
        break;
      }
    } else if (e & (E_MOVE | E_RELEASE), btns[1]) //no means voice
    { int nook = nookay(5); // Ask again after 30 seconds and return 1 if its ok
      if (nook) {
      message_to_whatsapp("Emergency, patient fell down");
      next_state = STAT_DFLT;
      delete [] btns;
      return next_state;
      break;
    }
  }

  return next_state;
}
}

int read_UI_wifi() {
  int next_state = STAT_WIFI;
  next_state = STAT_SETT; // Because not implememnted
  return next_state;
}

//int read_UI_time(){
// M5.Lcd.fillScreen(BLACK);
// set_time();
// int next_state = STAT_DFLT;
//  return next_state;
// }

void setup() {
  M5.begin();
  Serial.begin(115200);
  M5.IMU.Init();
  setup_audio();
  // Wifi setting
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}
int state = 0;


void loop() {

  M5.update();
  update_sensors();
  detect_fall(fall);
  if (fall) {
    Serial.println("Fall detected.");
    state = STAT_ALRT;
  }
  update_time(mytime);

  TouchPoint_t pos = M5.Touch.getPressPoint();

  switch (state) {

    case STAT_DFLT: {
        // M5.update();
        readTime();
        display_def(mytime);
        state = read_UI_def();
      }
      break;
    case STAT_SETT: {
        display_sett();
        state = read_UI_sett();
      }

      break;
    case STAT_ALRT: {
        display_alrt(mytime, alrt_btns);
        state = read_UI_alrt(alrt_btns);
        Serial.printf("Next State = %i \n", state);
      }
      break;

    case STAT_WIFI: {
        display_wifi();
        delay(500);
        state = read_UI_wifi();
      }
      break;

    case STAT_TIME: {

        display_time(mytime);
        // state = read_UI_time();
      }
      break;

    default:
      state = 0;
      break;
  }
}
