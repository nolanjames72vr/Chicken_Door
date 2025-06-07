#include <DS3231.h>

const int sch[13][4] = 
{
  {-1, -1, -1, -1},
  
  //Jan
  {6,30, 7,0}, 
  
  //Feb
  {6,30, 7,0},
  
  //Mar
  {6,30, 7,30},

  //Apr
  {6,30, 8,30},
  
  //May
  {6,0, 9,0},

  //Jun
  {5,30, 9,30},
  
  //Jul
  {5,45, 9,30},
  
  //Aug
  {6,15, 9,0},
  
  //Sep
  {6,30, 8,45},

  //Oct
  {6,30, 7,30},
  
  //Nov
  {6,30, 7,30},
  
  //Dec
  {6,30, 7,0}
};

int OpenRelay = 4;
int CloseRelay = 5;

DS3231  rtc(SDA, SCL);
Time t;
int month;
String d;
String sub_d;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  
  // Setup
  //rtc.setDOW(SATURDAY);
  // Hour, Min, Sec (24HR)
  //rtc.setTime(10, 24, 0);
  // Day, Month, Year
  //rtc.setDate(31, 5, 2025);
  
  pinMode(OpenRelay, OUTPUT);
  digitalWrite(OpenRelay, LOW);
  pinMode(CloseRelay, OUTPUT);
  digitalWrite(CloseRelay, LOW);
}

void loop() {
  t = rtc.getTime();
  d = rtc.getDateStr();
  sub_d = d.substring(3,5);
  month = sub_d.toInt();
  //Serial.println(rtc.getTimeStr());
  //Serial.println(rtc.getDateStr());
  delay (1000);
    
  if(t.hour == sch[month][0] && t.min == sch[month][1]){
    digitalWrite(OpenRelay,HIGH);
    delay(60000);
    digitalWrite(OpenRelay,LOW);
    }

  if(t.hour == sch[month][2] + 12 && t.min == sch[month][3]){
    digitalWrite(CloseRelay,HIGH);
    delay(60000);
    digitalWrite(CloseRelay,LOW);
    }
}
