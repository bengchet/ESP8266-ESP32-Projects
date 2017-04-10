#include <TimeLib.h>
#include <CytronWiFiShield.h>
#include <CytronWiFiClient.h>
#include <LCD4884.h>
#include "cytron.h"

const char *ssid = "Cytron-R&D";
const char *pass = "f5f4f3f2f1";

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  lcd.LCD_init();
  lcd.LCD_clear();
  lcd.backlight(ON);//Turn on the backlight
  lcd.LCD_draw_bmp_pixel(2,0,cytron,80, 24);
  lcd.LCD_write_string(2, 4, "Attendance", MENU_NORMAL);
  lcd.LCD_write_string(2, 5, "Record System", MENU_NORMAL);
  delay(2000);
  
  if(!wifi.begin(Serial2))
  {
    lcd.LCD_clear();
    lcd.LCD_write_string(0, 0, "Error", MENU_NORMAL);
    lcd.LCD_write_string(0, 1, "talking to", MENU_NORMAL);
    lcd.LCD_write_string(0, 2, "shield", MENU_NORMAL);
    while(1);
  }
  lcd.LCD_clear();
  lcd.LCD_write_string(0, 1, "Start", MENU_NORMAL);
  lcd.LCD_write_string(0, 2, "connecting", MENU_NORMAL);
  lcd.LCD_write_string(0, 3, "to WiFi", MENU_NORMAL);
  if(!wifi.connectAP(ssid, pass))
  {
    lcd.LCD_clear();
    lcd.LCD_write_string(0, 0, "Error", MENU_NORMAL);
    lcd.LCD_write_string(0, 1, "connecting", MENU_NORMAL);
    lcd.LCD_write_string(0, 2, "to WiFi", MENU_NORMAL);
    while(1);
  } 
  lcd.LCD_clear();
  lcd.LCD_write_string(0, 0, "Connected to:", MENU_NORMAL);
  lcd.LCD_write_string(0, 1, wifi.SSID().c_str(), MENU_NORMAL);
  lcd.LCD_write_string(0, 3, "IP Address: ", MENU_NORMAL);
  String s;
  IPAddress ip = wifi.localIP();
  for(int i = 0;i<3;i++)
  {
    s += ip[i];
    s += ".";
  }
  s += ip[3];
  lcd.LCD_write_string(0, 4, s.c_str() ,MENU_NORMAL);

  time_t getWiFiTime = clientTest();
  setTime(getWiFiTime);
  lcd.LCD_clear();
  
}

time_t prevDisplay = 0;

void loop() {
  // put your main code here, to run repeatedly:
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();  
    }
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  lcd.LCD_write_string( 0, 1, "Date", MENU_NORMAL);
  printDate(24, 2);
  lcd.LCD_write_string( 0, 3, "Time", MENU_NORMAL);
  printTime(36, 4);
}



void printDate(int _row, int _col){
  // utility for digital clock display: prints preceding colon and leading 0
  String d = (day()<10?"0":"") + (String)day() +"/";
  d += (month()<10?"0":"") + (String)month() + "/";
  d += (String)year();
  lcd.LCD_write_string(_row, _col, d.c_str(), MENU_NORMAL);
}

void printTime(int _row, int _col){
  String d = (hour()<10?"0":"") + (String)hour() +":";
  d += (minute()<10?"0":"") + (String)minute() +":";
  d += (second()<10?"0":"") + (String)second();
  lcd.LCD_write_string(_row, _col, d.c_str(), MENU_NORMAL);
}

time_t clientTest()
{
 RETRY:
 
  tmElements_t t;
  const char destServer[] = "www.adafruit.com";
  ESP8266Client client;
  if (!client.connect(destServer, 80))
  {
    lcd.LCD_clear();
    lcd.LCD_write_string(0, 0, "Failed to", MENU_NORMAL);
    lcd.LCD_write_string(0, 1, "connect to", MENU_NORMAL);
    lcd.LCD_write_string(0, 2, "server", MENU_NORMAL);
    client.stop();
    goto RETRY;
  }
  
  const char *httpRequest = "GET /testwifi/index.html HTTP/1.1\r\n"
                           "Host: www.adafruit.com\r\n"
                           "Connection: close\r\n\r\n";
                             
  if(!client.print(httpRequest))
  {
    lcd.LCD_clear();
    lcd.LCD_write_string(0, 0, "Sending data", MENU_NORMAL);
    lcd.LCD_write_string(0, 1, "failed", MENU_NORMAL);
    client.stop();
    goto RETRY;
  }

  // set timeout approximately 5s for server reply
  int i=5000;
  while (client.available()<=0&&i--)
  {
    delay(1);
    if(i==1) {
      lcd.LCD_clear();
      lcd.LCD_write_string(0, 0, "Receive", MENU_NORMAL);
      lcd.LCD_write_string(0, 1, "Timeout", MENU_NORMAL);
      goto RETRY;
      }
  }
  String s;
  const char *monthToNum[12]= {"Jan", "Feb", "Mar", "Apr", "May",
  "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  wifi.find("Date: ");wifi.find(", ");
  t.Day = client.readStringUntil(' ').toInt();
  s = client.readStringUntil(' ');
  t.Year = (client.readStringUntil(' ').toInt() - 1970);
  t.Hour = client.readStringUntil(':').toInt() + 8;
  t.Minute = client.readStringUntil(':').toInt();
  t.Second = client.readStringUntil(' ').toInt();
  while (client.available()>0)
    client.read();
  for(int i=0; i< 12; i++)
  {
    if(s.indexOf(monthToNum[i])!=-1)
      t.Month = i + 1;
  }
  client.stop();

  return makeTime(t);

}

