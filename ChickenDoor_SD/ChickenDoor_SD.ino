// needs RTClib by adafruit
#include <avr/sleep.h>
#include <ds3231.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"

#define BUFF_MAX 256
#define DHTTYPE DHT22
#define DHTPIN 3

const int sch[13][4] = 
{
  {-1, -1, -1, -1},
  
  //Jan
  {7,0, 6,30}, 
  
  //Feb
  {6,45, 6,30},
  
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

File myFile;
DHT dht(DHTPIN, DHTTYPE);

struct ts t;
int hour;

// Standard setup( ) function ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);
  DS3231_clear_a1f();

  Serial.println("Setup completed.");
  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  dht.begin();

  DS3231_get(&t);
  
  //t.hour = 11;
  //t.min = 47;
  //t.sec = 00;
  
  //t.mday = 23;
  //t.mon = 12;
  //t.year = 2023;
  //DS3231_set(t);
  
}
// ------------------------------------------------------------------------------------------------------

// The loop blinks an LED when not in sleep mode ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop() {
  //static uint8_t oldSec = 99;
  char buff[BUFF_MAX];

  DS3231_get(&t);

  if(t.min == 0 and t.sec ==0)
  {
    snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d\n", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
    Serial.print(buff);
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      //Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    myFile = SD.open("test.txt", FILE_WRITE);
  
    // if the file opened okay, write to it:
    if (myFile) {
      //Serial.print("Writing to test.txt...");
      myFile.println(buff);
      myFile.println(f);
      myFile.println();
      // close the file:
      myFile.close();
      //Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      //Serial.println("error opening test.txt");
    }
   }

   if(digitalRead(4) == 1 or digitalRead(5) == 1 )
   {
    // read file
    myFile = SD.open("test.txt");
    if (myFile) {
      //Serial.println("test.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
   }

   if(t.hour == sch[t.mon][0] && t.min == sch[t.mon][1]){
    digitalWrite(4,HIGH);
    delay(60000);
    digitalWrite(4,LOW);
    }

    if(t.hour == sch[t.mon][2] + 12 && t.min == sch[t.mon][3]){
    digitalWrite(5,HIGH);
    delay(60000);
    digitalWrite(5,LOW);
    }

    //snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d\n", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
    //Serial.print(buff);

   delay(1000);
}
