/*
 * Datum: 25.05.2017
 * Original Version by Poldi
 * modified by LukeLER
 * 
 * Event Abfragen:  
 * - Webserver im loop()  
 * - Taster über tasterEvents()  
 * - Zeit über   timeEvents()  
 * --> setzten von torON & lichtON StatusBits
 * 
 * Auswertung der StatusBits in  
 * - statusUmsetztenTor()   für Tor  
 * - statusUmsetztenLicht() für Licht
 * 
 * printDate() --> printet aktuelle Zeit der RTC 
 * getDate()   --> Auslesen der Aktuellen Zeit und speichern in Variablen
 * 
 * ###geschrieben und getestet für Arduino Leonardo###
 * 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#define DS1307_ADDRESS 0x68

byte mac[] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x24 };  // entspricht einer MAC von 84.85.88.16.0.36
byte ip[]  = { 192, 168, 178, 19 };                   // IP-Adresse
byte gateway[] = { 192, 168, 178, 1 };                // Gateway
byte subnet[]  = { 255, 255, 255, 0 };
EthernetServer server(80);

boolean reading = false;
byte lastpressed = 0;

int torSchaltStatus = 0;
int torStatus = 0;
int letzteLED = 0;
  
byte torPort = 4;
byte lichtPort = 5;
byte torTasterPort = 6;
byte lichtTasterPort = 7;
byte torLEDPort = 9;
byte lichtLEDPort = 13;
byte powerLEDPort = 8;
  
//Events
int torOffenStunde = 6;
int torOffenMinute = 0;
int torOffenSekunde = 10; //verhindert mehrmaliges Schalten pro Minute
int torZuStunde = 23;
int torZuMinute = 0;
int torZuSekunde = 10;
  
int lichtAnStunde = 6;
int lichtAnMinute = 0;
int lichtAusStunde = 23;
int lichtAusMinute = 0;

String readString = String(100);      // string for fetching data from address
boolean torON = false;                // status flag
boolean lichtON = false;

int second;
int minute;
int hour; //24 hour time
int weekDay; //0-6 -> sunday - Saturday
int monthDay;
int month;
int year;

void setup(){

  //Relais
  pinMode(torPort, OUTPUT); 
  digitalWrite(torPort, HIGH);
  pinMode(lichtPort, OUTPUT);
  digitalWrite(lichtPort, HIGH);
  //Taster
  pinMode(torTasterPort, INPUT);
  digitalWrite(torTasterPort, HIGH);
  pinMode(lichtTasterPort, INPUT);
  digitalWrite(lichtTasterPort, HIGH);
  //LED
  pinMode(torLEDPort, OUTPUT);
  pinMode(lichtLEDPort, OUTPUT);
  pinMode(powerLEDPort, OUTPUT);
  digitalWrite(powerLEDPort, HIGH);
  
  Wire.begin();
  Serial.begin(9600); 
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only sometimes
  //}
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop(){
  getDate();
  printDate(); //serial auf Konsole
  timeEvents();
  statusUmsetzenTor(); 
  statusUmsetzenLicht();
  tasterEvents();
  
//========WEBSERVER================================================================
  // Create a client connection 
  EthernetClient client = server.available();
  if (client) {
    boolean did = false;
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
  while (client.connected()) {
  if (client.available()) {

char c;
Serial.println(c);
c = client.read();
        Serial.println(c);

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info
if(reading){
  switch (c) {
            case 'h':
              if(did == false){
                if(lastpressed != 1) {
                    Serial.println("===================H===================");
                    torON = true;
                    did = true;
                    lastpressed = 1;
                    torSchaltStatus = 1; // Initiieren des richtigen Schaltbefehls
              }}
              break;
            case 'z':
              if(did == false){
                if(lastpressed != 2) {
                    Serial.println("===================Z===================");
                    torON = false;
                    did = true;
                    lastpressed = 2;
                    torSchaltStatus = 1;
              }}
              break;
            case 'e':
              if(did == false){
                if(lastpressed != 3) {
                    Serial.println("===================E===================");
                    lichtON = true;
                    did = true;
                    lastpressed = 3;
              }}
              break;
            case 'a':
              if(did == false){
                if(lastpressed != 4) {
                    Serial.println("===================A===================");
                    lichtON = false;
                    did = true;
                    lastpressed = 4;
              }}
              break;
            }
          }
if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }
//--------------------------HTML------------------------
if(!sentHeader){
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
//client.println("Refresh: 5");  //automatisches neuaufrufen nach 5 Sekunden
client.println();
client.print("<html><head>");
client.print("<title>GarageREMOTEcontrol</title>");
client.println("</head>");
client.print("<body bgcolor='#444444'>");
//---Überschrift---
client.println("<br><hr />");
client.println("<h1><div align='left'><font color='#2076CD'>GarageREMOTEcontrol 1.0</font color></div></h1>");
client.println("<hr /><br>");
//---Überschrift---
//---Ausgänge schalten---
client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Verf&uuml;gbare Aktoren Schaufenster</font></div>");
client.println("<br>");
client.println("<table border='1' width='500' cellpadding='5'>");
client.println("<tr bgColor='#222222'>");
 client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Beschattung<br></font></td>");
 client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='hochfahren'></form></td>");
 client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='zufahren'></form></td>");
 if (torON)
   client.println("<td align='center'><font color='green' size='5'>OBEN");
 else
   client.println("<td align='center'><font color='#CFCFCF' size='5'>UNTEN");
client.println("</tr>");
client.println("<tr bgColor='#222222'>");
 client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Beleuchtung<br></font></td>");
 //client.println("<td align='center' bgcolor='#222222'><form method=get><a href=\"/?a\" target=\"inlineframe\">ON</a></form></td>");
 client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='einschalten'></form></td>");
 client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='ausschalten'></form></td>");
 if (lichtON)
   client.println("<td align='center'><font color='green' size='5'>EIN");
 else
   client.println("<td align='center'><font color='#CFCFCF' size='5'>AUS");
client.println("</tr>");
client.println("</tr>");
client.println("</table>");
client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='1'>Build vom 25.5.2017<br></font></td>");
client.println("<br>");
//client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Aktuelle Uhrzeit: xx.xx.xx am xx.xx.xxxx</font></div>");
client.print("<div align='left'><font face='Verdana' color='#FFFFFF'>Aktuelle Uhrzeit: ");
  client.print(monthDay);
  client.print("/");
  client.print(month);
  client.print("/");
  client.print("20");
  client.print(year);
  client.print(" ");
  client.print(hour);
  client.print(":");
  client.print(minute);
  client.print(":");
  client.print(second);
  client.println(" </font></div>");
client.println("<br>");
//client.println("<form method=get><input type=submit name=all value='weiterer Button'></form>");
client.println("</body></html>");
sentHeader = true;
}

        
//---Ausgänge schalten---
//clearing string for next read
//readString="";
//stopping client

}}
delay(1);
client.stop();
did = false;
}}
//========loop() Ende==============================================================

byte bcdToDec(byte val)  
{
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

void getDate()
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);

  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}


void printDate()
{
  //print the date EG   3/1/11 23:59:59
  Serial.print(month);
  Serial.print("/");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
}

void tasterEvents()
{
  while(digitalRead(torTasterPort) == HIGH)
  {
    torSchaltStatus = 1;
    delay(200);
    //Serial.println("Taster gedrueckt");
    //Serial.println("1");
  }
  
  while(digitalRead(lichtTasterPort) == HIGH && letzteLED == 0)
  {
    letzteLED = 1;
    lichtON = true;
    delay(200);
    //Serial.println("2");
  }
  while(digitalRead(lichtTasterPort) == HIGH && letzteLED == 1)
  {
    letzteLED = 0;
    lichtON = false;
    delay(200);
    //Serial.println("3");
  }  
}

void timeEvents()
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);

  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());
  
  //=====================================================================
  //Tor
  if(hour == torOffenStunde)
  { if(minute == torOffenMinute)
    { if(second == torOffenSekunde)
      { if(torON == false)
          {torSchaltStatus = 1;}
      } 
    } 
  }
  if(hour == torZuStunde)
  { if(minute == torZuMinute)
    { if(second == torZuSekunde)
      { if(torON == true)
          {torSchaltStatus = 1;} //Beide male einschalten, da Tor über Strompuls getriggert wird
      }  
    } 
  }
  
  //Licht
  if(hour == lichtAnStunde)
  { if(minute == lichtAnMinute)
    {lichtON = true;}   
  }
  if(hour == lichtAusStunde)
  { if(minute == lichtAusMinute)
    {lichtON = false;} 
  } 
}

void statusUmsetzenTor()
{
  while(torSchaltStatus == 1 && torStatus == 0)
    {
      digitalWrite(torLEDPort, HIGH);
      digitalWrite(torPort, LOW);
      delay(1000);
      digitalWrite(torPort, HIGH);
      delay(1000);
      torSchaltStatus = 0; 
      torStatus = 1;
      torON = true;      
      //Serial.println("geschaltet1");
    }
    while(torSchaltStatus == 1 && torStatus == 1) //Für korrekte Status LED Tor
    {
      digitalWrite(torLEDPort, LOW);
      digitalWrite(torPort, LOW);
      delay(1000);
      digitalWrite(torPort, HIGH);
      delay(1000);
      torSchaltStatus = 0; 
      torStatus = 0;  
      torON = false;
      //Serial.println("geschaltet2");
    }
}
void statusUmsetzenLicht()
{
   if(lichtON == true)
     {
       digitalWrite(lichtPort, LOW);
       digitalWrite(lichtLEDPort, HIGH);  //Status LED
       lichtON = true;
     }
     else if(lichtON == false)
     {
       digitalWrite(lichtPort, HIGH);
       digitalWrite(lichtLEDPort, LOW); //Status LED
       lichtON = false;
     }
}
