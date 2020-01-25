#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
//#include <string.h>
/*                   
 * SPI MOSI    MOSI         23 
 * SPI MISO    MISO         19 
 * SPI SCK     SCK          18
 * SCA                      21
 * SCL                      22
 */
#define RST_PIN           17           
#define SS_PIN            16          
#define LED_R             4         
#define LED_G             2
#define LED_B             15
#define Pumpenrelais      25
#define Durchflussmesser  36
#define debug true
#define LCD_ADRESS 0x27
#define MAXLINES 4    
#define LCD_CHARACTERS 20
//---VARIABLEN-------------------------------------------------
const byte ROWS = 4; //4 Reihen
const byte COLS = 4; //4 Zeilen
//Definieren der Symbole auf den Tasten der Tastenfelder
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {',','0','#','D'}
};
byte rowPins[ROWS] = {13, 5, 32, 33}; //Verbindung zu den Zeilenzugaben des Tastenfeldes
byte colPins[COLS] = {26, 27, 14, 12}; //An die Spaltenbelegungen der Tastatur anschließen
const char* ssid          = "SSID";     
const char* password      = "password";
const char* server        = "script.google.com";  // Server URL
const char* key           = "**********************************"; // google script key
char customKey            = '0';

//Schlepper:
uint8_t ClaasJaguar_970[4] = {0x00, 0x00, 0x00, 0x00};
uint8_t Fendt_GT225[4]     = {0x00, 0x00, 0x00, 0x00};
uint8_t Fendt_516[4]       = {0x00, 0x00, 0x00, 0x00};
uint8_t Fendt_826[4]       = {0x00, 0x00, 0x00, 0x00};
uint8_t Fendt_820[4]       = {0x00, 0x00, 0x00, 0x00};
uint8_t AST[4]             = {0x00, 0x00, 0x00, 0x00};
uint8_t Sonstige[7]        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//Autos:
uint8_t Auto_Thomas[4]     = {0x00, 0x00, 0x00, 0x00};
uint8_t Auto_Stefan[4]     = {0x00, 0x00, 0x00, 0x00};
uint8_t Auto_Kurt[4]       = {0x00, 0x00, 0x00, 0x00};
uint8_t Auto_Nadja[4]      = {0x00, 0x00, 0x00, 0x00};
uint8_t Auto_Frank[4]      = {0x00, 0x00, 0x00, 0x00};

int Dieselmenge          = 0;
int Durchfluss           = 0;
int Pulseproliter        = 1;
int Messzeit1            = 1000;
int Messzeit2            = 5000;
int Ausschaltzeit        = 10000;
int Abbrechzeit          = 100000;
int maxdurchfluss        = 65;    // [l/min]

unsigned long Pulse      = 0;
unsigned long lastPulse  = 0;
unsigned long settime1   = 0;
unsigned long settime2   = 0;
unsigned long settime3   = 0;
unsigned long settime4   = 0;
unsigned long settime5   = 0;
unsigned long settime6   = 0;

String vehicle            = "kein Fahrzeug";
String URL                = "https://script.google.com/macros/s/";
String Betriebsstunden    = "";
String Einheit            = "";

bool pulseState           = 0;
bool oldpulseState        = 0;
bool Fertiggetankt        = 0;
bool chipauflegen         = 0;
bool Pumpe                = 0;
bool Netzwerk             = 0;

byte Status               = 0;  //0=Einschaltstatus; 1=Bereit zum Tanken; 2=Chip identifiziert; 3=Arbeit ausgewählt; 
                                //4=Tanken; 5=Tanken abgeschlossen; 6=Daten übermittelt; 

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(LCD_ADRESS,LCD_CHARACTERS,MAXLINES);
WiFiClientSecure client;
MFRC522 mfrc522(SS_PIN, RST_PIN);   
//-------------------------------------------------------------
void setup()
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(Pumpenrelais, OUTPUT);
  pinMode(Durchflussmesser, INPUT);
  digitalWrite(LED_R, 1);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 0);
  digitalWrite(Pumpenrelais, 1);
  
  Serial.begin(115200);
  SPI.begin();                                                  
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("-----HIGHBEACH------");
  lcd.setCursor(0,1);
  lcd.print("-----TANKSTELLE----");
  lcd.setCursor(0,3);
  lcd.print("SW Ver 1.0");
  delay(800);
lcd.clear();
}
//---Loop------------------------------------------------------
void loop()
{
//Status Bereit zum Tanken
if (WiFi.status() == WL_CONNECTED && Netzwerk == 0){
  Netzwerk = 1;
  digitalWrite(LED_R, 0);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 1);
  lcd.noBlink();
  }
else if (WiFi.status() != WL_CONNECTED){
  Netzwerk = 0;
  digitalWrite(LED_R, 1);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 0);
  lcd.clear();
  lcd.setCursor(0,0);                                           
  Serial.println("Warte auf WLAN-Verbindung");
  lcd.print("WLAN Verbinden");
  lcd.blink(); 
  WiFi.begin(ssid, password);
  delay(500);
 }
  if (Netzwerk == 1 && Status == 0){
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Verbunden !");
  lcd.setCursor(0,1);
  lcd.print("IP Adresse:");
  lcd.setCursor(0,2); 
  lcd.print(WiFi.localIP());
  Serial.println(WiFi.localIP());
  Status = 1;
delay(1500);  
}

if(Status == 1 && ((millis()-settime4)>1000)){
  settime4 = millis();
  digitalWrite(LED_R, 0);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 1); 
}
if(Status == 1 && ((millis()-settime4)>500)){
  digitalWrite(LED_R, 0);
  digitalWrite(LED_G, 1);
  digitalWrite(LED_B, 0); 
}

  if (Status == 1 && chipauflegen == 0 && Netzwerk ==1){
  mfrc522.PCD_Init();
  Serial.println("Bitte Chip auflegen");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Bitte Chip auflegen");
  chipauflegen = 1;
if(Dieselmenge != 0){
  lcd.setCursor(0,2);    
  lcd.print("Letzte getankte");
  lcd.setCursor(0,3);
  lcd.print("Menge: ");
  lcd.print(Dieselmenge);
  lcd.print(" Liter");
}
lcd.setCursor(19,0);
lcd.cursor_on();
lcd.blink();
}
//Status Chip identifiziert
//---RFID LESEN-----------------------------------------------
 if (Status == 1 && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ) {
  Status = 2;
   chipauflegen = 0;
  lcd.noBlink();
  lcd.cursor_off();
    Serial.print("Gelesene UID:");
 //  lcd.clear();
 // lcd.setCursor(3,0);
 // lcd.print("Gelesene UID:");
  //lcd.setCursor(19,0);
    uint8_t UIDread[mfrc522.uid.size];
    for (byte i = 0; i < mfrc522.uid.size; i++) {
       UIDread[i] = mfrc522.uid.uidByte[i];
      Serial.print(UIDread[i] < 0x10 ? " 0" : " ");
      Serial.print(UIDread[i], HEX);
      //lcd.print(UIDread[i] < 0x10 ? " 0" : " ");
     // lcd.print(UIDread[i], HEX);
    } 

Serial.println();
mfrc522.PICC_HaltA();

//---ID Zuweisung----------------------------------------------
      if(memcmp(UIDread, ClaasJaguar_970, sizeof(UIDread))==0){vehicle = "ClaasJaguar_970"; Einheit="h";}
else  if(memcmp(UIDread, Fendt_GT225,     sizeof(UIDread))==0){vehicle = "Fendt_GT225"; Einheit="h";}
else  if(memcmp(UIDread, Fendt_516,       sizeof(UIDread))==0){vehicle = "Fendt_516"; Einheit="h";}
else  if(memcmp(UIDread, Fendt_826,       sizeof(UIDread))==0){vehicle = "Fendt_826"; Einheit="h";}
else  if(memcmp(UIDread, Fendt_820,       sizeof(UIDread))==0){vehicle = "Fendt_820"; Einheit="h";}
else  if(memcmp(UIDread, AST      ,       sizeof(UIDread))==0){vehicle = "AST"; Einheit="Fahrzeug";}
else  if(memcmp(UIDread, Sonstige ,       sizeof(UIDread))==0){vehicle = "Sonstige"; Einheit="";;}
else  if(memcmp(UIDread, Auto_Thomas,     sizeof(UIDread))==0){vehicle = "Auto_Thomas"; Einheit="km";}
else  if(memcmp(UIDread, Auto_Stefan,     sizeof(UIDread))==0){vehicle = "Auto_Stefan"; Einheit="km";}
else  if(memcmp(UIDread, Auto_Kurt,       sizeof(UIDread))==0){vehicle = "Auto_Kurt"; Einheit="km";}
else  if(memcmp(UIDread, Auto_Nadja,      sizeof(UIDread))==0){vehicle = "Auto_Nadja"; Einheit="km";}
else  if(memcmp(UIDread, Auto_Frank,      sizeof(UIDread))==0){vehicle = "Auto_Frank"; Einheit="km";}

else {
vehicle = "kein Pool Fahrzeug";  
  digitalWrite(LED_R, 1);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 0);
  Status = 1;
}
if (Status == 2){
digitalWrite(LED_R, 0);
digitalWrite(LED_G, 1);
digitalWrite(LED_B, 0);
}
lcd.clear();
Serial.print("Erkanntes Fahrzeug: ");
lcd.setCursor(0,0);
lcd.print("Erkanntes Fahrzeug: ");
Serial.println(vehicle);
lcd.setCursor(0,1);
lcd.print(vehicle);
delay(1000);
lcd.setCursor(0,0);
lcd.print("                    ");
lcd.setCursor(0,0);
lcd.print("A:Abr|D:OK|C:Loesche");
lcd.setCursor(0,2);
lcd.print("Bitte ");
lcd.print(Einheit);
lcd.print(" eingeben:");
lcd.setCursor(0,3);
lcd.blink();
Betriebsstunden = "";
settime5 = millis();
}

//---Stunden eingeben------------------------------------------
if (Status == 2){  
if ((millis()-settime5)>Abbrechzeit){
Status = 1;  
}
if((millis()-settime4)>1000){
  settime4 = millis();
  digitalWrite(LED_R, 0);
  digitalWrite(LED_G, 0);
  digitalWrite(LED_B, 1); 
}
if((millis()-settime4)>500){
  digitalWrite(LED_R, 0);
  digitalWrite(LED_G, 1);
  digitalWrite(LED_B, 0); 
}
customKey = customKeypad.getKey();
if(customKey){
Serial.print("Tastendruck: ");
Serial.println(customKey);
settime5 = millis();
}
if(customKey == ',' ||customKey == '0' ||customKey == '1' ||customKey == '2' ||customKey == '3' ||customKey == '4' ||customKey == '5' ||customKey == '6' ||customKey == '7' ||customKey == '8' ||customKey == '9'){
Betriebsstunden += customKey;
Serial.print("Betriebsstunden: ");
Serial.print(Betriebsstunden);
Serial.print(" ");
Serial.println("Einheit");
lcd.setCursor(0,3);
lcd.print("                    ");
lcd.setCursor(0,3);
lcd.print(Betriebsstunden);
lcd.print(" ");
lcd.print(Einheit);
delay(100);
}
if(customKey == 'C'){
Betriebsstunden = Betriebsstunden.substring(0,Betriebsstunden.length()-1);
Serial.print("Betriebsstunden: ");
Serial.print(Betriebsstunden);
Serial.print(" ");
Serial.println(Einheit);
lcd.setCursor(0,3);
lcd.print("                    ");
lcd.setCursor(0,3);
lcd.print(Betriebsstunden);
lcd.print(" ");
lcd.print(Einheit);
delay(100);
}
if (customKey == 'D'){
Status = 3;  
lcd.noBlink();
lcd.print(" OK");
Serial.println("OK");
delay(1000);
}
if (customKey == 'A'){
Status = 3;  
lcd.noBlink();
lcd.print(" Abbrechen !");
Serial.println("Abbrechen");
Status = 1;
delay(2000);    
}
}
//Status Tanken
//---Logik-----------------------------------------------------
if(Status == 3 && Pumpe == 0){
Serial.println("Pumpe AN");
lcd.clear();
lcd.setCursor(3,0);
lcd.print("Pumpe AN");
digitalWrite(LED_R, 0);
digitalWrite(LED_G, 1);
digitalWrite(LED_B, 0);
settime1 = millis();
settime2 = 0;  
settime3 = millis(); 
Dieselmenge = 0;
Durchfluss  = 0;
Pulse = 0;
digitalWrite(Pumpenrelais, 0);
Status = 4;

lcd.setCursor(17,0);
lcd.print("l/h");
lcd.setCursor(0,2);
lcd.print("Getankte Menge:");
lcd.setCursor(0,3);
lcd.print(Dieselmenge);
lcd.print(" Liter");

}
//---Diesel Menge----------------------------------------------
if(Status == 4){
pulseState = digitalRead(Durchflussmesser);

if(pulseState == 1 && oldpulseState == 0){
oldpulseState = 1;
if((millis()-settime6)>(60000/maxdurchfluss)){
Pulse ++; 
}
settime6 = millis();
}
if(pulseState == 0 && oldpulseState == 1){
oldpulseState = 0;
}
if(lastPulse != Pulse){
lastPulse = Pulse;
Dieselmenge = Pulse/Pulseproliter;
Durchfluss = (60000/(millis()-settime2));
settime2 = millis();
lcd.setCursor(0,3);
lcd.print(Dieselmenge);
lcd.print(" Liter");
Serial.println(Dieselmenge);
}
if((millis()-(settime2))>4000){
Durchfluss = 0;  
}
if(Durchfluss < 5 || Pulse == 0){
Durchfluss = 0;
digitalWrite(LED_R, 1);
digitalWrite(LED_G, 0);
digitalWrite(LED_B, 1);
}
else {
settime3 = millis();
digitalWrite(LED_R, 0);
digitalWrite(LED_G, 1);
digitalWrite(LED_B, 0);
}
if(Durchfluss > 99)Durchfluss = 99;

if((millis()-settime1)>Messzeit1){ 
settime1 = millis();
lcd.setCursor(14,0);
lcd.print("   ");
lcd.setCursor(14,0);
lcd.print(Durchfluss);
Serial.println(Durchfluss);
if(Durchfluss == 0){
int Zeitbisaus = ((((millis()-settime3)/1000)*(-1))+9);  
lcd.setCursor(9,3);
lcd.print("AUS in: ");
lcd.print(Zeitbisaus); 
lcd.print(" s"); 
}
else {
lcd.setCursor(9,3);
lcd.print("           ");
}
}
}
//Status Tanken abgeschlossen
if(Status == 4 && (millis()-settime3)>Ausschaltzeit) {
digitalWrite(Pumpenrelais, 1);  
if (Dieselmenge == 0){ 
Serial.print("Getankte Menge = 0 ==> Keine Daten übermittelt");
lcd.clear();
lcd.setCursor(3,0);
lcd.print("Pumpe AUS");
Serial.println("Pumpe AUS");
lcd.setCursor(0,1);
lcd.print("Keine Daten");
lcd.setCursor(0,2);
lcd.print("uebertragen");
lcd.setCursor(0,3);
lcd.print("Getankte Menge = 0");
digitalWrite(LED_R, 1);
digitalWrite(LED_G, 0);
digitalWrite(LED_B, 0);
Status = 1;
delay(10000);
}
else{
lcd.clear();
lcd.setCursor(3,0);
lcd.print("Pumpe AUS");
Serial.println("Pumpe AUS");
Serial.print("getankte Menge: ");
Serial.println(Dieselmenge);
lcd.setCursor(0,2);
lcd.print("Getankte Menge:");
lcd.setCursor(3,3);
lcd.print(Dieselmenge);
lcd.print(" Liter");
digitalWrite(LED_R, 0);
digitalWrite(LED_G, 0);
digitalWrite(LED_B, 1);
Status = 5;
}
}
//Status Daten übermittelt
//---Script-Link-Zusammenbauen---------------------------------
if (Status == 5){
String URL="https://script.google.com/macros/s/";
  URL += key;
  URL += "/exec?";
  URL += "1_Spalte=";
  URL += vehicle;
  URL += "&2_Spalte=";
  URL += Betriebsstunden;
  URL += "&3_Spalte=";
  URL += Dieselmenge;
//  URL += "&4_Spalte=";
//  URL += Dieselmenge;
//  URL += "&5_Spalte=";
//  URL += "500";
//  URL += "&6_Spalte=";
//  URL += "600";

if(Netzwerk == 1){
  String movedURL;
  String line;
  if (debug)Serial.println("Verbinde zum script.google.com");
  if (!client.connect(server, 443))
  {
    if (debug) Serial.println("Verbindung fehlgeschlagen!");
  }
  if (debug) Serial.println("Verbindung hergestellt");
  client.println("GET " + URL);
  client.println("Host: script.google.com" );
  client.println("Connection: close");
  client.println();
  client.stop();
Serial.println("---> Werte in Tabelle übermittelt <---");
digitalWrite(LED_R, 0);
digitalWrite(LED_G, 1);
digitalWrite(LED_B, 0); 
lcd.setCursor(0,1); 
lcd.print("D");
delay(200);
lcd.print("a");
delay(200);
lcd.print("t");
delay(200);
lcd.print("e");
delay(200);
lcd.print("n ");
delay(200);
lcd.print("u");
delay(200);
lcd.print("e");
delay(200);
lcd.print("b");
delay(200);
lcd.print("e");
delay(200);
lcd.print("r");
delay(200);
lcd.print("t");
delay(200);
lcd.print("r");
delay(200);
lcd.print("a");
delay(200);
lcd.print("g");
delay(200);
lcd.print("e");
delay(200);
lcd.print("n");
delay(1000);
lcd.print(" !");
delay(15000);
Status = 1;
}
//Wenn keine Wlan Verbindung besteht
else{
digitalWrite(LED_R, 1);
digitalWrite(LED_G, 0);
digitalWrite(LED_B, 0);
lcd.clear();
lcd.setCursor(0,0); 
lcd.print("!! ACHTUNG !!");
lcd.setCursor(0,1);
lcd.print("WLAN nicht verbunden"); 
Status = 1;
delay(100000);
}
}
}
