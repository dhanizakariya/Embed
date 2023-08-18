#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <virtuabotixRTC.h>
#include <PS2Keyboard.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  4

#define MAX_CLK_PIN   13  // DOT MATRIX SCK
#define MAX_DATA_PIN  11  // DOT MATRIX MOSI
#define MAX_CS_PIN    10  // DOT MATRIX SS
#define RTC_CLK_PIN   7  // RTC CLK
#define RTC_DAT_PIN   6  // RTC DAT
#define RTC_RST_PIN   5  // RTC RST
#define DATA_PIN      3  // KEYBOARD DATA
#define IRQ_PIN       2  // KEYBOARD IRQ

MD_Parola P(HARDWARE_TYPE, MAX_CS_PIN, MAX_DEVICES);
virtuabotixRTC myrtc = virtuabotixRTC(RTC_CLK_PIN,RTC_DAT_PIN,RTC_RST_PIN);
PS2Keyboard KB;

String hours;
String minutes;
float ldr;
float temperature;
float sensorValue;

String temp_alarm_hours;
String temp_alarm_minutes;
int temp_alarm_duration = 5;
int selected_alarm;

String alarm_1_hours;
String alarm_2_hours;
String alarm_3_hours;
String alarm_4_hours;
String alarm_5_hours;
String alarm_1_minutes;
String alarm_2_minutes;
String alarm_3_minutes;
String alarm_4_minutes;
String alarm_5_minutes;

String temp_string;
char buf[99] = {""};

// 1 -> jam
// 2 -> pilihan menu
// 3 -> setting jam
// 4 -> pilihan alarm
// 5 -> seting waktu alarm
// 6 -> setting durasi
// 7 -> alarm aktif
int state = 1;

// 1 -> set jam
// 2 -> set alarm
int current_menu = 1;
int current_alarm_menu = 1;

int current_active_alarm = 0;

int current_alarm_on_off_menu = 1;

bool is_setting_hour = false;
bool is_setting_string = false;

struct alarm {
  bool active;
  int hours;
  int minutes;
  int duration;
};

void Display(String str)
{
//  Serial.println(str);
  str.toCharArray(buf, 99);
  P.displayReset();
  P.setTextBuffer(buf);
  P.displayAnimate();
}

void Clear()
{
  P.displayReset();
  P.displayClear();
}

void DisplayAnimate(String alarm, String str)
{
//  Serial.println(alarm);
  P.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
  P.setSpeed(50);
  P.setPause(500);
  str.toCharArray(buf, 99);
  P.setTextBuffer(buf);
  if (P.displayAnimate()) {
    Clear();
  }
}

alarm alarm1 = {false, 0, 0, 0};
alarm alarm2 = {false, 0, 0, 0};
alarm alarm3 = {false, 0, 0, 0};
alarm alarm4 = {false, 0, 0, 0};
alarm alarm5 = {false, 0, 0, 0};
alarm alarms[5] = {alarm1, alarm2, alarm3, alarm4, alarm5};

unsigned long cur_time;
unsigned long prev_time;

void StateAlarm()
{
  alarms[selected_alarm - 1].hours = temp_alarm_hours.toInt();
  alarms[selected_alarm - 1].minutes = temp_alarm_minutes.toInt();
  alarms[selected_alarm - 1].duration = temp_alarm_duration;
  alarms[selected_alarm - 1].active = true;
}


unsigned long temptot = 0;
      
void setup()
{
  Serial.begin(9600);
  P.begin();
  P.displayClear();
  P.setCharSpacing(1);
  attachInterrupt(digitalPinToInterrupt(DATA_PIN), interruptHandler, FALLING);
  KB.begin(DATA_PIN, IRQ_PIN, PS2Keymap_US);
}

void loop()
{
  P.setTextAlignment(PA_CENTER);
  
  switch (state) {
    case 1:
      myrtc.updateTime();
      // cek alarm
      for (int i = 0; i < 5; i++) {
        if (alarms[i].hours == myrtc.hours && alarms[i].minutes == myrtc.minutes && alarms[i].active) {
          state = 7;
          current_active_alarm = i + 1;
          prev_time = millis();
        }
      }
      temptot = 0;
      for(int x=0; x<500 ; x++){
          temptot = temptot + analogRead(A1);
      }
      sensorValue = temptot/500; //calculating average
      temperature = (sensorValue/1024)*500;
      
      ldr = analogRead(A0);
      P.setIntensity(ldr / 69);
      
      if (String(myrtc.hours).length() > 1) {
        hours = String(myrtc.hours);
      } else {
        hours = "0" + String(myrtc.hours);
      }
      if (String(myrtc.minutes).length() > 1) {
        minutes = String(myrtc.minutes);
      } else {
        minutes = "0" + String(myrtc.minutes);
      }
      if (10 <= myrtc.seconds && myrtc.seconds < 15  || 40 <= myrtc.seconds && myrtc.seconds < 45 ) {
        Display((String(temperature) + "C"));
        delay(400);
        temptot = 0;
      } else {
        Display((hours + " " + minutes));
        delay(500);
        Display ((hours + ":" + minutes));
        delay(500);
      }
      break;
    case 2:
      switch (current_menu) {
        case 1:
          DisplayAnimate("",String("Set Time"));
          break;
        case 2:
          DisplayAnimate("",String("Set Alarm"));
          break;
      }
      break;
    case 3:
      if (hours.length() == 1) {
        hours = "0" + hours;
      }
      if (minutes.length() == 1) {
        minutes = "0" + minutes;
      }
      if (is_setting_hour) {
        Display((hours + ":" + minutes));
        delay(500);
        Display(("   :" + minutes));
        delay(500);
        break;
      } else {
        Display((hours + ":" + minutes));
        delay(500);
        Display((hours + ":   "));
        delay(500);
        break;
      }
    case 4:
      switch (current_alarm_menu) {
        case 1:
          Display(String("1"));
          break;
        case 2:
          Display(String("2"));
          break;
        case 3:
          Display(String("3"));
          break;
        case 4:
          Display(String("4"));
          break;
        case 5:
          Display(String("5"));
          break;
      }
      break;
    case 5:
      if (temp_alarm_hours.length() == 1) {
        temp_alarm_hours = "0" + temp_alarm_hours;
      }
      if (temp_alarm_minutes.length() == 1) {
        temp_alarm_minutes = "0" + temp_alarm_minutes;
      }
      if (is_setting_hour) {
        Display((temp_alarm_hours + ":" + temp_alarm_minutes));
        delay(500);
        Display(("   :" + temp_alarm_minutes));
        delay(500);
        break;
      } else {
        Display((temp_alarm_hours + ":" + temp_alarm_minutes));
        delay(500);
        Display((temp_alarm_hours + ":   "));
        delay(500);
        break;
      }
      break;
    case 6:
      Display((String(temp_alarm_duration) + " s"));  
      break;
    case 7:
      if (millis() - prev_time > (alarms[current_active_alarm - 1].duration * 1000)) {
        alarms[current_active_alarm - 1].active = false;
        state = 1;
        }else {
        switch (current_active_alarm) {
          case 1:
            DisplayAnimate("alarm 1",String("07211940000016"));
            break;
          case 2:
            DisplayAnimate("alarm 2",String("Muhammad Zakariya Nur Ramdhhani"));
            break;
          case 3:
            DisplayAnimate("alarm 3",String("Muhammad Zakariya Nur Ramdhani | 0721194000016"));
            break;
          case 4:
          for(int x = alarms[current_active_alarm - 1].duration; x > 0 ;x--){
            Display((String(x) + " s"));
            delay(1000);
          }        
            break;
          case 5:
            DisplayAnimate("alarm 5",String(temp_string)); 
            break;
          }
        }
      break; 
    case 8:
      DisplayAnimate("",String(temp_string));
      break;
      
  }
  P.setTextEffect(PA_PRINT, PA_NO_EFFECT);
  
}

void interruptHandler() {
  switch (state) {
    case 1:
      if (KB.available()) {
        char c = KB.read();
        Serial.println(c);
        if (c == PS2_ENTER) {
          state = 2;
        }
      }
      break;
    case 2:
      switch (current_menu) {
        case 1:
          if (KB.available()) {
            char c = KB.read();
            if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
              current_menu = 2;
            } else if (c == PS2_ENTER) {
              state = 3;
              is_setting_hour = true;
            } else if (c == PS2_ESC) {
              state = 1;
            }
          }
        case 2:
          if (KB.available()) {
            char c = KB.read();
            if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
              current_menu = 1;
            } else if (c == PS2_ENTER) {
              state = 4;
            } else if (c == PS2_ESC) {
              state = 1;
              current_menu = 1;
            }
          }
          break;
      }
      break;
    case 3:
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_ESC) {
          state = 2;
        } else if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
          is_setting_hour = !is_setting_hour;
        } else if (c == PS2_UPARROW) {
          if (is_setting_hour) {
            if (hours.toInt() < 23 && hours.toInt() >= 0) {
              hours = String(hours.toInt() + 1);
            } else if (hours.toInt() == 23) {
              hours = String(0);
            }
          } else {
            if (minutes.toInt() < 59 && minutes.toInt() >= 0) {
              minutes = String(minutes.toInt() + 1);
            } else if (minutes.toInt() == 59) {
              minutes = String(0);
            }
          }
        } else if (c == PS2_DOWNARROW) {
          if (is_setting_hour) {
            if (hours.toInt() <= 23 && hours.toInt() > 0) {
              hours = String(hours.toInt() - 1);
            } else if (hours.toInt() == 0) {
              hours = String(23);
            }
          } else {
            if (minutes.toInt() <= 59 && minutes.toInt() > 0) {
              minutes = String(minutes.toInt() - 1);
            } else if (minutes.toInt() == 59) {
              minutes = String(59);
            }
          }
        } else if (c == PS2_ENTER) {
          myrtc.setDS1302Time(00, minutes.toInt(), hours.toInt(), 1, 4, 6, 2001);
          state = 1;
          is_setting_hour = true;
        }
      }
      break;
    case 4:
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_LEFTARROW) {
          if (current_alarm_menu == 1) {
            current_alarm_menu = 5;
          } else {
            current_alarm_menu -= 1;
          }
        } else if (c == PS2_RIGHTARROW) {
          if (current_alarm_menu == 5) {
            current_alarm_menu = 1;
          } else {
            current_alarm_menu += 1;
          }
        } else if (c == PS2_ENTER) {
          state = 5;
          temp_alarm_hours = hours;
          temp_alarm_minutes = minutes;
          selected_alarm = current_alarm_menu;
        } else if (c == PS2_ESC) {
          state = 3;
          current_alarm_menu = 1;
        }
      }
      break;
    case 5:
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_ESC) {
          state = 4;
        } else if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
          is_setting_hour = !is_setting_hour;
        } else if (c == PS2_UPARROW) {
          if (is_setting_hour) {
            if (temp_alarm_hours.toInt() < 23 && temp_alarm_hours.toInt() >= 0) {
              temp_alarm_hours = String(temp_alarm_hours.toInt() + 1);
            } else if (temp_alarm_hours.toInt() == 23) {
              temp_alarm_hours = String(0);
            }
          } else {
            if (temp_alarm_minutes.toInt() < 59 && temp_alarm_minutes.toInt() >= 0) {
              temp_alarm_minutes = String(temp_alarm_minutes.toInt() + 1);
            } else if (temp_alarm_minutes.toInt() == 59) {
              temp_alarm_minutes = String(0);
            }
          }
        } else if (c == PS2_DOWNARROW) {
          if (is_setting_hour) {
            if (temp_alarm_hours.toInt() <= 23 && temp_alarm_hours.toInt() > 0) {
              temp_alarm_hours = String(temp_alarm_hours.toInt() - 1);
            } else if (temp_alarm_hours.toInt() == 0) {
              temp_alarm_hours = String(23);
            }
          } else {
            if (temp_alarm_minutes.toInt() <= 59 && temp_alarm_minutes.toInt() > 0) {
              temp_alarm_minutes = String(temp_alarm_minutes.toInt() - 1);
            } else if (temp_alarm_minutes.toInt() == 59) {
              temp_alarm_minutes = String(59);
            }
          }
        } else if (c == PS2_ENTER) {
          
          state = 6;
          is_setting_hour = true;
        }
      }
      break;
    case 6:
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_UPARROW) {
          temp_alarm_duration += 1;
        } else if (c == PS2_DOWNARROW && temp_alarm_duration > 1) {
          temp_alarm_duration -= 1; 
        } else if (c == PS2_ESC) {
          state = 5;
        } else if (c == PS2_ENTER) {
          if (selected_alarm == 5) {
            temp_string = "";
            state = 8;
          } else {
            state = 1;
            StateAlarm();
          }
        }
      }
      break;
    case 7 :
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_ENTER) {
          state = 1;
          alarms[current_active_alarm - 1].active = false;
        }
      }
    case 8 :
      if (KB.available()) {
        char c = KB.read();
        if (c == PS2_ENTER) {
          StateAlarm();
          state = 1;
        } else {
          temp_string = temp_string + String(c);
          Clear();
        }
      }
      break;
           
  }
}
