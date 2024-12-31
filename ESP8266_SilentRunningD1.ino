/*
  ESP8266 (LOLIN(WEMOS)D1 R2
  Silent Running D#1

  FS: 2MB OTA:1019kb
  80MHz

  VID: 0x1A86
  PID: 0x7523


  Flash real id:   0016405E
  Flash real size: 4194304 bytes

  Flash ide  size: 4194304 bytes
  Flash ide speed: 40000000 Hz
  Flash ide mode:  DOUT
  Flash Chip configuration ok.

*/
const char* progversion = "Drone#1 V0.52";  //ota fs ntp ti neopixel (audio)
const char* ARDUINO_HOSTNAME = "D1";

//const char*  WIFI_SSID     ="wifi-ssid";
//const char*  WIFI_PASSWORD ="passwort";
#include "wifisetup.h"  //fest eingebunden WIFI_SSID+WIFI_PASSWORD

#include "data.h"       //index.htm

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>

#include "JeVe_EasyOTA_V2.h"  // https://github.com/jeroenvermeulen/JeVe_EasyOTA/blob/master/JeVe_EasyOTA.h
//include <JeVe_EasyOTA.h>  // https://github.com/jeroenvermeulen/JeVe_EasyOTA/blob/master/JeVe_EasyOTA.h
#include "FS.h"

#include "myNTP.h"
myNTP oNtp;

#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
AudioGeneratorMP3 *mp3gen;
AudioFileSourceSPIFFS *audiofile;
AudioOutputI2S *audioout ;
AudioFileSourceID3 *id3;
//LRC: statt D4(io02) D7(io13) benutzen?
String mp3dateilink = "";
uint8_t mp3plaingstatus=0;//0=nichts, 66=start fileplay, 1=playig
int volume = 25;
bool isAudioinit=false;
/*
const int preallocateCodecSize =29192;
void *preallocateCodec = NULL;
*/



String anidateilink = "";
String lastaniini = "/lastani.ini";
uint8_t aniplaingstatus=0;
unsigned long ani_startMillis=0;
unsigned long ani_waitMillis=1000;
File ani_datei;//NULL ->zeiger auf datei
uint8_t readbyte;

//-----------------------------Taster------------------------------------------------------
#define taster1_PIN 14
#define taster2_PIN 12
#define taster3_PIN 13
bool is_taster1=false;
bool is_taster2=false;
bool is_taster3=false;




//----------------------------------------------------------------------------------------

#define Neopixel_LED_PIN 0    //ESP8266 D1: Data an D3 GPIO0
#define Neopixel_LED_COUNT 5  //How many NeoPixels

uint8_t helligkeit=50;

uint8_t currentColor[Neopixel_LED_COUNT][3] = { { 0, 0, 0 } };//Array mit dera aktuellen Farbe aller LEDs - setzt sie auf rgb=0

bool saveneostatus = false;

Adafruit_NeoPixel strip(Neopixel_LED_COUNT, Neopixel_LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

#define check_wlanasclient 30000  //alle 30 Sekunden*2 gucken ob noch verbunden, wenn nicht neuer Versuch \
                                  //zwischen 30 und 60 Sekunden
unsigned long check_wlanasclient_previousMillis = 0;
#define anzahlVersuche 10  //nach 10 Versuchen im AP-Modus bleiben
#define keinAPModus true   //true=immer wieder versuchen ins WLAN zu kommen

#define actionheader "HTTP/1.1 303 OK\r\nLocation:/index.htm\r\nCache-Control: no-cache\r\n\r\n"

uint8_t MAC_array[6];
char MAC_char[18];
String macadresse = "";

EasyOTA OTA;
ESP8266WebServer server(80);
File fsUploadFile;  //Haelt den aktuellen Upload
File fsFile;        //abspeichern neopixelstatus/farben

bool isAPmode = false;
int anzahlVerbindungsversuche = 0;

unsigned long tim_zeitchecker = 15 * 1000;  //alle 15sec Timer checken
unsigned long tim_previousMillis = 0;
byte last_minute;

unsigned long currentMillis = 0;
unsigned long checkNEOPIXEL = 0;
unsigned long NEOPIXELTimeOut = 1 * 1000;  //alle 1sec checken, wenn AP-Mode

//---------------------------------------------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

//---------------------------------------------
void connectWLAN() {  
  anzahlVerbindungsversuche++;
  OTA.setup(WIFI_SSID, WIFI_PASSWORD, ARDUINO_HOSTNAME);  //connect to WLAN
  isAPmode = !(WiFi.status() == WL_CONNECTED);
  /*
  Serial.print("mode: ");
  if (isAPmode)
    Serial.println("AP");
  else
    Serial.println("client");
  */

  macadresse = "";
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    if (i > 0) macadresse += ":";
    macadresse += String(MAC_array[i], HEX);
  }
  /*
  Serial.print("MAC: ");
  Serial.println(macadresse);

  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  */
  if (isAPmode) {
     delay(100);
  } else {
    anzahlVerbindungsversuche = 0;  //erfolgreich verbunden, Zaehler auf 0 setzen
   }
}
//---------------------------------------------

void setup() {
  //serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println(progversion);

  pinMode(taster1_PIN, INPUT);
  pinMode(taster2_PIN, INPUT);
  pinMode(taster3_PIN, INPUT);

  is_taster1=digitalRead(taster1_PIN);
  is_taster2=digitalRead(taster2_PIN);
  is_taster3=digitalRead(taster3_PIN);

  // preallocateCodec = malloc(preallocateCodecSize);

  //SPIFFS
  SPIFFS.begin();
 
  //dateiname der letzten animation
  fsFile = SPIFFS.open(lastaniini, "r");
  if (fsFile){
    anidateilink=fsFile.readString();
    fsFile.close();
  }

  //NEOPIXEL
  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  colorWipe(strip.Color(0, 0, 0), 0);
  strip.show();  // Turn OFF all pixels ASAP
  strip.setBrightness(helligkeit);

  initpixel();

  //OTA
  OTA.onMessage([](char* message, int line) {
    Serial.println(message);
  });

  connectWLAN();

  server.on("/action", handleAction);  //daten&befehle
  server.on("/", handleIndex);
  server.on("/index.htm", handleIndex);
  server.on("/index.html", handleIndex);
  server.on("/data.json", handleData);  //aktueller Status+Dateiliste
  server.on(
    "/upload", HTTP_POST, []() {
      server.send(200, "text/plain", "");
    },
    handleFileUpload);                //Dateiupload
  server.onNotFound(handleNotFound);  //Datei oder 404

  server.begin();
  Serial.println("HTTP server started");

  Serial.println("ready.");

  //NTP start
  oNtp.begin();

  initAudio();
  //mp3dateilink="/intro.mp3";
  //playmp3(mp3dateilink.c_str());
}


unsigned long taster_timeout=50;//ms
unsigned long taster1_presstime=0;  //alle LEDs durchschalten
unsigned long taster2_presstime=0;
unsigned long taster3_presstime=0;  //stop/restart Animation
bool last_taster1=false;
bool last_taster2=false;
bool last_taster3=false;

uint8_t modus=0;//taster 1


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  OTA.loop();
  oNtp.update();
  isAPmode = !(WiFi.status() == WL_CONNECTED);

  currentMillis = millis();  //milliseconds since the Arduino board began running

  is_taster1=digitalRead(taster1_PIN);
  is_taster2=digitalRead(taster2_PIN);
  is_taster3=digitalRead(taster3_PIN);

  if(is_taster1){
    if(last_taster1!=is_taster1){//noch nicht ausgelöst
      if(taster1_presstime==0)taster1_presstime=currentMillis;//start press
      
      if(currentMillis-taster1_presstime>taster_timeout){
          //ist gedrückt
          if(last_taster1!=is_taster1){//onDown
              stopAni();
              modus++;
              if(modus==1){
                colorWipe(strip.Color(255, 0, 0), 100); 
              }
              if(modus==2){
                colorWipe(strip.Color(0, 255, 0), 100); 
              }
              if(modus==3){
                colorWipe(strip.Color(0, 0, 255), 100); 
              }
              if(modus==4){
                colorWipe(strip.Color(255, 255, 255), 100); 
              }
              if(modus==5){
                colorWipe(strip.Color(0, 0, 0), 0);
              }
              if(modus==6){
                 initpixel();//Wie zuletzt gespeichert
                 modus=0;
              }
          }
          last_taster1=is_taster1;
          taster1_presstime=0;
      }
    }
  }
  else{
    if(last_taster1!=is_taster1){//noch nicht ausgelöst
        if(taster1_presstime==0)taster1_presstime=currentMillis;//start up
        if(currentMillis-taster1_presstime>taster_timeout){
          //ist gedrückt
          if(last_taster1!=is_taster1){//onUp
             // colorWipe(strip.Color(0, 0, 0), 0); 
          }
          last_taster1=is_taster1;
          taster1_presstime=0;
      }
    }
  }
  
  
  if(is_taster2){
    if(last_taster2!=is_taster2){//noch nicht ausgelöst
      if(taster2_presstime==0)taster2_presstime=currentMillis;//start press
      
      if(currentMillis-taster2_presstime>taster_timeout){
          //ist gedrückt
          if(last_taster2!=is_taster2){//onDown
              //action
              if(helligkeit==50){
                helligkeit=100;
              }
              else
              if(helligkeit==100){
                helligkeit=175;
              }
              else
              if(helligkeit==175){
                helligkeit=255;
              }
              else{
                helligkeit=50;
              }
              strip.setBrightness(helligkeit);
              strip.show();
          }
          last_taster2=is_taster2;
          taster2_presstime=0;
      }
    }
  }
  else{
    if(last_taster2!=is_taster2){//noch nicht ausgelöst
        if(taster2_presstime==0)taster2_presstime=currentMillis;//start up
        if(currentMillis-taster2_presstime>taster_timeout){
          //ist gedrückt
          if(last_taster2!=is_taster2){//onUp
             // colorWipe(strip.Color(0, 0, 0), 0); 
          }
          last_taster2=is_taster2;
          taster2_presstime=0;
      }
    }
  }
 
 if(is_taster3){
    if(last_taster3!=is_taster3){//noch nicht ausgelöst
      if(taster3_presstime==0)taster3_presstime=currentMillis;//start press
      
      if(currentMillis-taster3_presstime>taster_timeout){
          if(last_taster3!=is_taster3){//onDown
              //action: stop/restart ani
              if(aniplaingstatus!=0){
                stopAni(); //stopp aniloop
                initpixel();//Wie zuletzt gespeichert
              }
              else{
                restartAni();
               }
          }
          last_taster3=is_taster3;
          taster3_presstime=0;
      }
    }
  }
  else{
    if(last_taster3!=is_taster3){//noch nicht ausgelöst
        if(taster3_presstime==0)taster3_presstime=currentMillis;//start up
        if(currentMillis-taster3_presstime>taster_timeout){
          if(last_taster3!=is_taster3){}//onUp
          last_taster3=is_taster3;
          taster3_presstime=0;
      }
    }
  }
 
  /*if(oNtp.hatTime() && currentMillis - tim_previousMillis > tim_zeitchecker){//Timer checken
      tim_previousMillis = currentMillis;
      if(last_minute!=oNtp.getminute()){//nur 1x pro min
        //checktimer();
        last_minute=oNtp.getminute();
      }
    }*/

  //WLAN-ceck
  unsigned long cwl = random(check_wlanasclient, check_wlanasclient + check_wlanasclient);  //x..x+15sec sonst zu viele Anfragen am AP
  if (currentMillis - check_wlanasclient_previousMillis > cwl) {
    //zeit abgelaufen
    check_wlanasclient_previousMillis = currentMillis;
    if (isAPmode) {  //apmode
      //neuer Verbindengsaufbauversuch
      if (anzahlVerbindungsversuche < anzahlVersuche || keinAPModus) {  //nur x-mal, dann im AP-Mode bleiben
        connectWLAN();
      }
    }
  }
  if(aniplaingstatus==0)handlingNeoPixelWLAN();//check ob AP-Modus ist, wenn ja Lampe blinken
  
  //handling LED animation
  if(aniplaingstatus==66){//eine Datei soll abgespielt werden
	 startAni(anidateilink.c_str());//init
  }
  if(aniplaingstatus==1){
	 aniloop();
  }
  
  //----handling audio----
  handelAudio();
   

  yield();
}

//--------------------
int neopixelstepp = 0;
void handlingNeoPixelWLAN() {

  if (isAPmode)
  if (currentMillis - checkNEOPIXEL > NEOPIXELTimeOut) {//wenn im AP-Mode Strip-LED 0 blinken
    checkNEOPIXEL = currentMillis;
    //1x je Sekunde //colorWipe(strip.Color(128, 0, 0), 0);
    if (neopixelstepp == 0) {      
      setPixelColorSave(0, 255, 0, 0);
      setPixelColorSave(1, 0, 0, 0);
      setPixelColorSave(2, 0, 0, 0);
    }
    if (neopixelstepp == 1) {
      setPixelColorSave(0, 0, 0, 0);
      setPixelColorSave(1, 128, 0, 0);
      setPixelColorSave(2, 0, 0, 0);
    }
    if (neopixelstepp == 2) {
      setPixelColorSave(0, 0, 0, 0);
      setPixelColorSave(1, 0, 0, 0);
      setPixelColorSave(2, 128, 0, 0);
    }
    strip.show();
    neopixelstepp++;
    if (neopixelstepp > 2) { neopixelstepp = 0; }
  }
}

//------------Data IO--------------------

void handleData() {  // data.json
  String message = "{\r\n";
  String aktionen = "";

  if (mp3plaingstatus!=0){
      message += "\"bussy\":\"mp3-";
      message +=mp3plaingstatus;
      message += "\"}\n";
      server.sendHeader("Access-Control-Allow-Origin", "*");  //wenn vom HTTPS-Seiten aufgerufen wird!
      server.send(200, "text/plain", message);
      return;
  }/**/

//mp3plaingstatus

  //uebergabeparameter?
  uint8_t i;
  for (i = 0; i < server.args(); i++) {
    if (server.argName(i) == "settimekorr") {
      oNtp.setTimeDiff(server.arg(i).toInt());
      aktionen += "set_timekorr ";
    }
  }

  message += "\"neoleds\":[";
  for (i = 0; i < Neopixel_LED_COUNT; i++) {
    //uint32_t color = strip.getPixelColor(11);
    if (i > 0) message += ",";
    message += "[";
    message += String(currentColor[i][0]);
    message += ",";
    message += String(currentColor[i][1]);
    message += ",";
    message += String(currentColor[i][2]);
    message += "]";
  }
  message += "],\r\n";


  message += "\"hostname\":\"" + String(ARDUINO_HOSTNAME) + "\",\r\n";
  message += "\"aktionen\":\"" + aktionen + "\",\r\n";
  message += "\"aniplaingstatus\":\"" + String(aniplaingstatus) + "\",\r\n";
  message += "\"anidateilink\":\"" + anidateilink + "\",\r\n";


  message += "\"progversion\":\"" + String(progversion) + "\",\r\n";
  message += "\"cpu_freq\":\"" + String(ESP.getCpuFreqMHz()) + "\",\r\n";
  message += "\"chip_id\":\"" + String(ESP.getChipId()) + "\",\r\n";
  message += "\"flashchiprealsize\":\"" + String(ESP.getFlashChipRealSize()) + "\",\r\n";

  message += "\"saveneostatus\":\"";
  if (saveneostatus)
    message += "true";
  else
    message += "false";
  message += "\",\r\n";

  message += "\"isAPmode\":\"";
  if (isAPmode)
    message += "true";
  else
    message += "false";
  message += "\",\r\n";


  //--Uhrzeit/Datum--

  byte ntp_stunde = oNtp.getstunde();
  byte ntp_minute = oNtp.getminute();
  byte ntp_secunde = oNtp.getsecunde();

  message += "\"lokalzeit\":\"";
  if (ntp_stunde < 10) message += "0";
  message += String(ntp_stunde) + ":";
  if (ntp_minute < 10) message += "0";
  message += String(ntp_minute) + ":";
  if (ntp_secunde < 10) message += "0";
  message += String(ntp_secunde);
  message += "\",\r\n";

  message += "\"datum\":{\r\n";
  message += " \"tag\":" + String(oNtp.getwochentag()) + ",\r\n";
  message += " \"year\":" + String(oNtp.getyear()) + ",\r\n";
  message += " \"month\":" + String(oNtp.getmonth()) + ",\r\n";
  message += " \"day\":" + String(oNtp.getday()) + ",\r\n";
  message += " \"timekorr\":" + String(oNtp.getUTCtimediff()) + ",\r\n";
  if (oNtp.isSummertime())
    message += " \"summertime\":true\r\n";
  else
    message += " \"summertime\":false\r\n";
  message += "},\r\n";

  //led-status
  message += "\"portstatus\":{";
  message +="\"taster1\":";
  if(is_taster1)
     message +="true,";
     else
     message +="false,";
 message +="\"taster2\":";
  if(is_taster2)
     message +="true,";
     else
     message +="false,";
 message +="\"taster3\":";
  if(is_taster3)
     message +="true";
     else
     message +="false";
  message += "},\r\n";  //Portstatus
 
  message += "\"macadresse\":\"" + macadresse + "\",\r\n";

  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
    message += "\"fstotalBytes\":" + String(fs_info.totalBytes) + ",\r\n";
    message += "\"fsusedBytes\":" + String(fs_info.usedBytes) + ",\r\n";

    message += "\"fsused\":\"";
    message += float(int(100.0 / fs_info.totalBytes * fs_info.usedBytes * 100.0) / 100.0);
    message += "%\",\r\n";
  }
  //files
  message += "\"files\":[\r\n";
  String fileName;
  Dir dir = SPIFFS.openDir("/");
  uint8_t counter = 0;
  while (dir.next()) {
    fileName = dir.fileName();
    if (counter > 0) message += ",\r\n";
    message += " {";
    message += "\"fileName\":\"" + fileName + "\", ";
    message += "\"fileSize\":" + String(dir.fileSize());
    message += "}";
    if (counter == 0) counter = 1;
  };
  message += "\r\n]\r\n";
  //--

  message += "\r\n}";

  server.sendHeader("Access-Control-Allow-Origin", "*");  //wenn vom HTTPS-Seiten aufgerufen wird!
  server.send(200, "text/plain", message);
  //Serial.println("send data.json");
}

void handleIndex() {  //Rueckgabe HTML
  //$h1gtag $info
  int pos1 = 0;
  int pos2 = 0;
  String s;
  String tmp;

  String message = "";

  while (indexHTM.indexOf("\r\n", pos2) > 0) {
    pos1 = pos2;
    pos2 = indexHTM.indexOf("\r\n", pos2) + 2;
    s = indexHTM.substring(pos1, pos2);

    //Tags gegen Daten ersetzen
    if (s.indexOf("$h1gtag") != -1) {
      s.replace("$h1gtag", progversion);  //Ueberscherschrift=Prog-Version
    }

    //Liste der Dateien
    if (s.indexOf("$filelist") != -1) {

      //Serial.println("Get DIR.");
      tmp = "<table class=\"files\">\n";
      String fileName;
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        fileName = dir.fileName();
        tmp += "<tr>\n";
        tmp += "\t<td><a target=\"_blank\" href =\"" + fileName + "\"";
        tmp += " >" + fileName.substring(1) + "</a></td>\n\t<td class=\"size\">" + formatBytes(dir.fileSize()) + "</td>\n\t<td class=\"action\">";
        tmp += "<a href =\"" + fileName + "?delete=" + fileName + "\" class=\"fl_del\"> l&ouml;schen </a>\n";
        tmp += "\t</td>\n</tr>\n";
      };

      FSInfo fs_info;
      tmp += "<tr><td colspan=\"3\">";
      if (SPIFFS.info(fs_info)) {
        tmp += formatBytes(fs_info.usedBytes).c_str();  //502
        tmp += " von ";
        tmp += formatBytes(fs_info.totalBytes).c_str();  //2949250 (2.8MB)   formatBytes(fileSize).c_str()
        tmp += " (";
        tmp += float(int(100.0 / fs_info.totalBytes * fs_info.usedBytes * 100.0) / 100.0);
        tmp += "%)";
        /*tmp += "<br>\nblockSize:";
          tmp += fs_info.blockSize; //8192
          tmp += "<br>\npageSize:";
          tmp += fs_info.pageSize; //256
          tmp += "<br>\nmaxOpenFiles:";
          tmp += fs_info.maxOpenFiles; //5
          tmp += "<br>\nmaxPathLength:";
          tmp += fs_info.maxPathLength; //32*/
      }
      tmp += "</td></tr></table>\n";
      s.replace("$filelist", tmp);
    }
    message += s;
  }

  server.send(200, "text/html", message);
}

void handleAction() {  //Rueckgabe JSON
  /*
      /action?set=LEDWLANON       blaue LED einschalten
      /action?set=LEDWLANOFF      blaue LED ausschalten
      /action?led=0&rgb=L49480a    erste neopixel-LED Farbe setzen
      /action?set=saveneostatusON   neo-led-status speichern (um Ram zu schonen wenig benutzen)
      /action?set=saveneostatusOFF   neo-led-status speichern (um Ram zu schonen wenig benutzen)
  */
  String message = "{\n";

  
  /*if (mp3plaingstatus!=0){//während Audio spielt, bussy zurückgeben, keine Befehle annehmen
      message += "\"bussy\":\"mp3-";
      message +=mp3plaingstatus;
      message += "\"}\n";
      server.send(200, "text/plain", message);
      return;
  }*/

  message += "\"Arguments\":[\n";

  uint8_t AktionBefehl = 0;
  uint8_t keyOK = 0;
  uint8_t aktionresult = 0;

  int ziellednr = Neopixel_LED_COUNT;  //Aufruf immer setzen einer LED
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  //Übergabeparameter in Befehl oder Daten überführen
  for (uint8_t i = 0; i < server.args(); i++) {
    if (i > 0) message += ",\n";
    message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"";

    if (server.argName(i) == "set") {
      if (server.arg(i) == "LEDWLANON") AktionBefehl = 7;
      if (server.arg(i) == "LEDWLANOFF") AktionBefehl = 8;
      if (server.arg(i) == "saveneostatusON") AktionBefehl = 9;
      if (server.arg(i) == "saveneostatusOFF") AktionBefehl = 10;
    }

    if (server.argName(i) == "led") {
      ziellednr = server.arg(i).toInt();
    }
    if (server.argName(i) == "rgb") {
      if (server.arg(i).length() == 7) {
        red = strtol(server.arg(i).substring(1, 3).c_str(), NULL, 16);
        green = strtol(server.arg(i).substring(3, 5).c_str(), NULL, 16);
        blue = strtol(server.arg(i).substring(5, 7).c_str(), NULL, 16);
      }
    }

    if (server.argName(i) == "mp3"){
       //stopAudio();
       //if(mp3plaingstatus==0){
          mp3dateilink="/";
          mp3dateilink+=server.arg(i);
          mp3plaingstatus=66;

          message += ",\n\"befehl\":\"OK\"";
       //}else{
       //   message += ",\n\"befehl\":\"ERR\"";
       //}
    }
	
	
    if (server.argName(i) == "ani"){
		  message += ",\n\"befehl\":\"OK\"";
      anidateilink="/";
      anidateilink+=server.arg(i);

      //aktuelle ani-Dateiname speichern, für Taster 3
      fsFile = SPIFFS.open(lastaniini, "w");
      if (fsFile){
        fsFile.write(anidateilink.c_str());
        fsFile.close();
      }

		  aniplaingstatus=66;//abzuspielende ani-dateiname gemerkt, weiter im loop
    }
    if (server.argName(i) == "stop"){
		  message += ",\n\"befehl\":\"OK\"";
      stopAni();
      //alten LED-Status wieder herstellen
      initpixel();
    }
	
    message += "}";
  }
  message += "\n]";

  
  if(ziellednr != Neopixel_LED_COUNT) {//0...<Neopixel_LED_COUNT
   if(aniplaingstatus==0){
    setPixelColorSave(ziellednr, red, green, blue);
      strip.show();
      message += ",\n\"befehl\":\"OK\"";
    }else{
      message += ",\n\"befehl\":\"bussy\",\n\"bussy\":\"aniisplaying\"";
    }
  }

  if (AktionBefehl > 0) {
    aktionresult = 0;
   
    if (AktionBefehl == 9) {  //"saveneostatusON"
      saveneostatus = true;
      aktionresult = AktionBefehl;
    }
    if (AktionBefehl == 10) {  //"saveneostatusOFF"
      saveneostatus = false;
      aktionresult = AktionBefehl;
    }
  
   
    message += ",\n\"befehl\":\"";
    if (aktionresult > 0)
      message += "OK";
    else
      message += "ERR";
    message += "\"";
  }

  message += "\n}";
  server.send(200, "text/plain", message);
}

String getContentType(String filename) {  // ContentType fuer den Browser
  if (filename.endsWith(".htm")) return "text/html";
  //else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".rgb")) return "application/octet-stream"; //Farbspeicher für neopixel
  else if (filename.endsWith(".ani")) return "application/octet-stream"; //Speicher für neopixel-animation
 /*else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";*/
  return "text/plain";
}

void handleFileUpload() {  // Dateien ins SPIFFS schreiben
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (filename.length() > 30) {//die letzten 30 Zeichen nehmen (um Dateienung mit zu nehmen)
      int x = filename.length() - 30;
      filename = filename.substring(x, 30 + x);
    }
    filename = server.urlDecode(filename);
    filename = "/" + filename;

    fsUploadFile = SPIFFS.open(filename, "w");
    if (!fsUploadFile) {
      //Serial.println("!! file open failed !!");  //**************
      server.sendContent("HTTP/1.1 900 file open failed\r\nLocation:/\r\nCache-Control: no-cache\r\n\r\n");
      yield();
      return;
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    yield();
   
    bool gosetup=true;
    for (uint8_t i = 0; i < server.args(); i++) {//TODO ?????
      if(server.argName(i) == "rel") {
        if(server.arg(i)=="no")gosetup=false;		//"./upload?rel=no"
      }
    }
	
    //303=Seite umlenken
    if(gosetup)
      //"HTTP/1.1 303 OK\r\nLocation:/index.htm\r\nCache-Control: no-cache\r\n\r\n"//Seite neu laden
      server.sendContent(actionheader);
      else
      server.sendContent("HTTP/1.1 303 OK\r\nLocation:/\r\nCache-Control: no-cache\r\n\r\n");
  }
}

bool handleFileRead(String path) {  //Datei loeschen oder uebertragen
  //Datei löschen
  if (server.hasArg("delete")) {
    SPIFFS.remove(server.arg("delete"));  //hier wir geloescht
    server.sendContent(actionheader);     //Seite neu laden
    return true;
  }

  //Datei ausliefern
  path = server.urlDecode(path);
  if (SPIFFS.exists(path)) {
    File sendfile = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(sendfile, getContentType(path));
    sendfile.close();
    return true;
  }
  
   return false;
}

void handleNotFound() {
  //--check Dateien im SPIFFS--
  if (!handleFileRead(server.uri())) {
    //--404 als JSON--
    String message = "{\n \"error\":\"File Not Found\", \n\n";
    message += " \"URI\": \"";
    message += server.uri();
    message += "\",\n \"Method\":\"";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\",\n";
    message += " \"Arguments\":[\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      if (i > 0) message += ",\n";
      message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"}";
    }
    message += "\n ]\n}";
    server.send(404, "text/plain", message);
  }
}


//------------------NEO-PIXEL------------------

void setPixelColorSave(int pixel, uint8_t red, uint8_t green, uint8_t blue) {
  strip.setPixelColor(pixel, strip.gamma32(strip.Color(red, green, blue)));
  // Speichere die Farbwerte
  currentColor[pixel][0] = red;
  currentColor[pixel][1] = green;
  currentColor[pixel][2] = blue;

  if (saveneostatus) {
    //save to mem
    String filename = "/neo";
    filename += String(pixel);
    filename += ".rgb";

    fsFile = SPIFFS.open(filename, "w");
    if (fsFile){
      fsFile.write((uint8_t*)&red, sizeof(red));
      fsFile.write((uint8_t*)&green, sizeof(green));
      fsFile.write((uint8_t*)&blue, sizeof(blue));
      fsFile.close();
    }
  }
}

void initpixel() {
  String filename = "";

  uint8_t red, green, blue;
  File theFile;

  if (Neopixel_LED_COUNT > 0) {
    for (int i = 0; i < Neopixel_LED_COUNT; i++) {
      filename = "/neo";
      filename += String(i);
      filename += ".rgb";
      theFile = SPIFFS.open(filename, "r");
      if (theFile) {
        theFile.read((uint8_t*)&red, sizeof(red));
        theFile.read((uint8_t*)&green, sizeof(green));
        theFile.read((uint8_t*)&blue, sizeof(blue));
        theFile.close();
        currentColor[i][0] = red;
        currentColor[i][1] = green;
        currentColor[i][2] = blue;
        strip.setPixelColor(i, strip.gamma32(strip.Color(red, green, blue)));
      }
    }
    strip.show();
  }
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}
 

//------------------NEO-PIXEL-ani------------------
void stopAni(){
 	if(ani_datei)ani_datei.close();
  aniplaingstatus=0;
}
void restartAni(){
  if (anidateilink.length() > 0){//animation neustart, wenn DAtei ausgewählt wurde
    stopAni();
    aniplaingstatus=66;
  }
}
void startAni(const char *filename){	
	//wenn datei offen schließen
	if(ani_datei)ani_datei.close();

	//datei öffen
	//ani_datei ->zeiger auf datei
	ani_datei = SPIFFS.open(filename, "r");		
	ani_startMillis=currentMillis;
	ani_waitMillis=1;
	aniplaingstatus=1;//66=startplay, 0=noplay, 1=isplaying
	
}
void  aniloop(){
	//currentMillis//global
	//ani_startMillis=0;//zeitpunkt start letzter waitsequenz
	//ani_waitMillis=1500;//aus datei
	if(aniplaingstatus==1){
		uint8_t byte1;
		uint8_t byte2;
		uint8_t counter;
		uint8_t br;
		uint8_t bg;
		uint8_t bb;
		
    if (currentMillis - ani_startMillis > ani_waitMillis) {//Wartezeit überschritten?
 			if (ani_datei && ani_datei.available()) {
			  //nächste Befehle einlesen
				readbyte = ani_datei.read();//P,L,T oder E
				
				//P... nr.rgb	->einzelnes Pixel
				if(readbyte=='P'){
					//4 bytes
					//5.='#'
					byte2=ani_datei.read();
					br=ani_datei.read();
					bg=ani_datei.read();
					bb=ani_datei.read();
					strip.setPixelColor(byte2, strip.gamma32(strip.Color(br, bg, bb)));
          strip.show();
				}
				//S... c rgbrgbrgb ->mehrere LEDs im Streifen
				if(readbyte=='S'){
					//1Byte=anzahl; 3*anzahl Bytes
					byte1=ani_datei.read();//Anzahl
					for (counter = 0; counter < byte1; counter++) {
						br=ani_datei.read();
						bg=ani_datei.read();
						bb=ani_datei.read();
						//->set 
						strip.setPixelColor(counter, strip.gamma32(strip.Color(br, bg, bb)));
					}
          strip.show();
				}
				if(readbyte=='T'){
					//2 byte 0..65535 (max ~65 Sekunden)
					//3.='#'
					byte1=ani_datei.read();//H
					byte2=ani_datei.read();//L
					ani_waitMillis = (byte1 << 8) | byte2;
			 	  if(ani_waitMillis==0)ani_waitMillis=2000;//16;//min 16ms~60fps
					ani_startMillis= currentMillis;//aktuelle Zeit
				}
				//T... ms ->warten
				
				//L =loop, datei von vorne einlesen
			  /*if(readbyte=='L'){
          ani_datei.seek(0, SeekSet);
        }*/

				if(readbyte=='E'){
          //end
        }

			}else {
				if(ani_datei){
          //ani_datei.close();
          ani_datei.seek(0, SeekSet);
        }
				//aniplaingstatus=0;
				//aniplaingstatus=66;//loop
			}
			
		}
		//wenn fertig Datei schließen		
	}
}
//----------------Audio-------------
// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
 /*void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string){// n.b.
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }  
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();

  //Serial.println(string);
  }
*/
void initAudio(){//#2 nur 1x
  //out = new AudioOutputI2SDAC();
  //wav = new AudioGeneratorWAV();

   audioout =new AudioOutputI2S();//nur 1x

}

void stopAudio(){ 
 if(isAudioinit){
    audioout->SetGain(0.0);
    audioout->stop();

    if(id3->isOpen()){
      id3->close();
    }
    if(audiofile->isOpen()){ 
        audiofile->close();
    }
    if (mp3gen->isRunning()){
        mp3gen->stop();
      }

    //#2 delete audioout;
    delete mp3gen;
    delete audiofile;
    delete id3;
 /*
    audioout=NULL;
    mp3gen=NULL;
    audiofile=NULL;
    id3=NULL;
    */

    /*
      decoder->stop(); //stop playing
      delete decoder; //free decoder an its memory
      decoder = NULL; 



      AudioGeneratorMP3 *mp3gen;
      AudioFileSourceSPIFFS *audiofile;
      AudioOutputI2S *out ;
      AudioFileSourceID3 *id3;

      // Serial.print("start play ");
      // Serial.println(filename);
      //Audio
      //AudioOutputI2S(int port=0, int output_mode=EXTERNAL_I2S, int dma_buf_count = 8, int use_apll=APLL_DISABLE);

    */
    isAudioinit=false;
  }
  
  mp3plaingstatus=0;
}

void handelAudio(){//loop
  if(isAudioinit && mp3plaingstatus==1){//audio läuft
    if (mp3gen->isRunning()) {
      if (!mp3gen->loop()){
        stopAudio();//->0
        }
    } 
  }/**/
  if(mp3plaingstatus==66){//play from SPDIF-from www
    playmp3(mp3dateilink.c_str());
    //-->mp3plaingstatus=1  + isAudioinit
  }  
}
/*
   //defaults
    mono = false;
    lsb_justified = false;
    bps = 16;
    channels = 2;
    hertz = 44100;
    bclkPin = 26; //IO01 ?
    wclkPin = 25; //IO03 ?
    doutPin = 22; //SD_CLK
    mclkPin = 0;
    SetGain(1.0); 
*/
//https://github.com/earlephilhower/ESP8266Audio

void playmp3(const char *filename){
  stopAudio();//alles zurücksetzen

  //neu 
 //#2 audioout =new AudioOutputI2S();
// audioout =new AudioOutputI2S(0,EXTERNAL_I2S,8,APLL_DISABLE);
//int port=0, int output_mode=EXTERNAL_I2S, int dma_buf_count = 8, int use_apll=APLL_DISABLE

  //set the pins to b e used for I2S: Bit-clock, left/right clock, datastream
  // audioout->SetPinout(BCLK, LRCLK, DOUT);//26,25,27
 //#2 audioout->SetGain(((float)volume)/100.0);

  mp3gen = new AudioGeneratorMP3();
  //mp3gen = new AudioGeneratorMP3(preallocateCodec, preallocateCodecSize);

  audiofile = new AudioFileSourceSPIFFS(filename);
  audiofile->open(filename);
  id3 = new AudioFileSourceID3(audiofile);
  //id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");//n.g.
 //#2 delay(50);
 
  mp3gen->begin(id3, audioout);		
  delay(50);
  audioout->SetGain(((float)volume)/100.0);

  isAudioinit=true;
  mp3plaingstatus=1;//plaing
  
}

