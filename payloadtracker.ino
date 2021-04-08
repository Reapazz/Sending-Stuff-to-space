#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include <string.h>
#include <util/crc16.h>
#define RADIOPIN 8

static const int RXPin = 7, TXPin = 6;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
#include <SPI.h>
#include <SD.h>
File myFile;

float latitude, longitude, temp;
float thour;
String dtime;

#include "RTClib.h"
RTC_DS3231 rtc;




void setup() {
  pinMode(RADIOPIN, OUTPUT);
  Serial.begin(9600);
  ss.begin(GPSBaud);

Serial.println("test");
 //Serial.print(F("Initializing SD card..."));

  if (!SD.begin(4)) {
   // Serial.println(F("initialization failed!"));
    while (1);
  }
  //Serial.println(F("initialization done."));
  myFile = SD.open("log3.txt", FILE_WRITE);


  if (! rtc.begin()) {
   Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    //Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  wdt_enable(WDTO_8S);
}



void loop() {
  Serial.flush();
  
  // Serial.flush();
  Serial.println("start");
  latitude = 0;
  longitude = 0;
  temp = 0;
  thour = 0;
  while (latitude == 0 || longitude == 0) {

    while (ss.available() > 0) {
      //Serial.println("test2");
      gps.encode(ss.read());
      if (gps.location.isUpdated()) {
       // Serial.println(F("test3"));
        //Serial.println(F(gps.location.lat()),6);
        
        latitude = (gps.location.lat());
        longitude = (gps.location.lng());
      //  Serial.println(latitude);



      }
    }
  }

  while (thour == 0) {
    DateTime now = rtc.now();
    dtime = (String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
    thour = now.hour();
    //Serial.println(dtime);
  }

  while (temp == 0) {

    //Serial.println("notemp");
    temp = rtc.getTemperature();


    //Serial.println ("TEMP: " + temp);
  }


  String msgfinal;
  msgfinal = ("////" +dtime + "///LAT: " + String(latitude, 5) + "///LON: " + String(longitude,5) + "///" + "TEMP: " + String(temp) );
 
// Serial.println(dtime+ "///LAT: " + String(latitude) + "///LON: " + String(longitude));
 // Serial.println(msgfinal);

  myFile = SD.open("log3.txt", FILE_WRITE);

  if (myFile) {

    myFile.println(msgfinal);
    // close the file:
    myFile.close();

  } else {
    // if the file didn't open, print an error:
    //Serial.println(F("error opening test.txt"));
  }


  wdt_reset();
  sendmsg(msgfinal);

  Serial.println("sent2");

}

void sendmsg (String msg) {
   Serial.flush();
  char datastring[80];
  //memset(0, datastring, sizeof(datastring));
  //snprintf(datastring, 200, "RTTY TEST BEACON RTTY TEST BEACON");
  snprintf(datastring,80,msg.c_str()); // Puts the text in the datastring
  unsigned int CHECKSUM = gps_CRC16_checksum(datastring);  // Calculates the checksum for this datastring
  char checksum_str[6];
  sprintf(checksum_str, "*%04X\n", CHECKSUM);
  strcat(datastring, checksum_str);
  Serial.println(datastring);
  rtty_txstring (datastring);
  
  delay(3500);
  Serial.println("sent1");
  delay(4000);

}




void rtty_txstring (char * string)
{


  /* Simple function to sent a char at a time to
     ** rtty_txbyte function.
    ** NB Each char is one byte (8 Bits)
  */

  char c;

  c = *string++;

  while ( c != '\0')
  {
    rtty_txbyte (c);
    c = *string++;
  }
}


void rtty_txbyte (char c)
{
  /* Simple function to sent each bit of a char to
    ** rtty_txbit function.
    ** NB The bits are sent Least Significant Bit first
    **
    ** All chars should be preceded with a 0 and
    ** proceded with a 1. 0 = Start bit; 1 = Stop bit
    **
  */


  int i;

  rtty_txbit (0); // Start bit

  // Send bits for for char LSB first

  for (i = 0; i < 7; i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
  {
    if (c & 1) {
      rtty_txbit(1);
    }

    else {
      rtty_txbit(0);
    }

    c = c >> 1;

  }

  rtty_txbit (1); // Stop bit
  rtty_txbit (1); // Stop bit
}

void rtty_txbit (int bit)
{

  if (bit)
  {
    // high
    digitalWrite(RADIOPIN, HIGH);
  }
  else
  {
    // low
    digitalWrite(RADIOPIN, LOW);

  }

  delayMicroseconds(3370); // 300 baud
  // delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
  // delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
  // largest value that will produce an accurate delay is 16383
  // See : http://arduino.cc/en/Reference/DelayMicroseconds
  //Serial.println("testmsg5");
}

uint16_t gps_CRC16_checksum (char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }

  return crc;

}
