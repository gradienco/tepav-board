#include <WiFi.h>
#include <FirebaseESP32.h>
#include <FirebaseJson.h>
#include <EEPROM.h>
#include <WiFiManager.h>

//Firebase
#define FIREBASE_HOST "https://tepav-6171f.firebaseio.com/"
#define FIREBASE_AUTH "PkkvZ8N0byvoGsmF5CYF5PojpZ3MU6iTTmOQO6ZF"
FirebaseData firebaseData;
unsigned long waktusebelum = 0; 
FirebaseJson jsonData;
String macAddress;
String devicePath;
String lastPacketId;

//Input/Output
#define lockfront 23
#define lockback 22
#define sinaruv 21
#define sensor_pintu 19
#define sensor_sharp A3
#define sensor_uv A0
#define uvlight 13
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int interval = 30; //30 minutes convert to milisecond
long previousMillis = 0;     
int ledUV = 1;
bool sterilState = false;
bool uvState = false;
int z = 0;

//Bluetooth
// #include "BluetoothSerial.h"
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif
// BluetoothSerial SerialBT;

//Wifi
const char* ssid     = "HKTI PROVINSI"; //default for developers
const char* password = "tanijaya";
String esid;
String epass;
char wifi[2][32];

void setup() {
  Serial.begin(9600);
  delay(10);

  //-------------------------Bluetooth Connection
  //SerialBT.begin("Tepav"); 
  //Serial.println("Bluetooth started!");

  //------------------------- Wifi Connection
  //WiFi.disconnect();
  Serial.println("Startup");
  delay(10);
  /* --- AUTOCONNECTION -- */
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  bool res = wm.autoConnect("TepavConnection");
  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("Wifi connected");
  }
  /* --- INIT WIFI + EEPROM -- */
  /*
  EEPROM.begin(512);
  Serial.println("Reading EEPROM ssid");
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM password");
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  //WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.begin(ssid, password);

  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print("..");
  }
  Serial.println("Connected");*/
  
  Serial.print("Your IP is: ");
  Serial.println((WiFi.localIP()));

  //--------------------------------Firebase Connection
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);//Declare device id
  macAddress = WiFi.macAddress();
  devicePath = "/device/"+ macAddress;
  Serial.print("Your MacAddr is: ");
  Serial.println(macAddress);
  Firebase.setString(firebaseData, devicePath + "/wifi/ssid", WiFi.SSID());
  
  //-------------------------------Input Output
  pinMode(sensor_pintu, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  pinMode(sinaruv, OUTPUT);
  pinMode(lockfront, OUTPUT);
  pinMode(lockback, OUTPUT);
  dht.begin();
  digitalWrite(lockfront, 1);
  digitalWrite(lockback, 1);
  digitalWrite(sinaruv, 1);
}

void loop() {

  if ((!(WiFi.status() == WL_CONNECTED))) {
    Serial.println("Disconnected");
    delay(300000); //freeze until 5 minutes, then restart
    ESP.restart();
  }

  /* --- CONFIG VIA BLUETOOTH --- */
  // if (SerialBT.available()) {
  //   // Serial.println("Incoming Bluetooth Data");
  //   int x = 0;
  //   int y = 0;
  //   while (SerialBT.available() > 0){
  //     const char byteRead = SerialBT.read();
  //     if (byteRead==';') {
  //       break;
  //     } else if (byteRead==':') {
  //       x++;
  //       y = 0;
  //     } else {
  //       wifi[x][y] = byteRead;
  //       y++;
  //     }
  //     //Serial.write(SerialBT.read());    
  //   }
  //   // Serial.print("SSID: ");
  //   // Serial.println(wifi[0]);
  //   // Serial.print("Password: ");
  //   // Serial.println(wifi[1]);
  //   Serial.flush();
  //   //save to EEPROM
  //   // Serial.println("Writing eeprom ssid:");
  //   for (int i = 0; i < sizeof(wifi[0]); ++i)
  //       {
  //         EEPROM.write(i, wifi[0][i]);
  //         // Serial.print("Wrote: ");
  //         // Serial.println(wifi[0][i]);
  //       }
  //   // Serial.println("Writing eeprom pass:");
  //   for (int i = 0; i < sizeof(wifi[1]); ++i)
  //   {
  //     EEPROM.write(32 + i, wifi[1][i]);
  //     // Serial.print("Wrote: ");
  //     // Serial.println(wifi[1][i]);
  //   }
  //   EEPROM.commit();
  //   //push to Database
  //   Firebase.setString(firebaseData, devicePath + "/wifi/ssid", wifi[0]);
  //   Firebase.setString(firebaseData, devicePath + "/wifi/password", wifi[1]);
  //   //Firebase.setString(firebaseData, devicePath + "/duration", wifi[2]);
  //   // Serial.println("Data stored to Database");
  //   // Serial.println("Now Restarting...");
  //   delay(1000);
  //   ESP.restart();
  // }


  unsigned long waktusekarang = millis();
  if (waktusekarang - waktusebelum > 5000) { //non interupting delay 1 second
    Serial.println("start");
    waktusebelum = waktusekarang;

    z++;
    if (z == 60) { //update every 60 seconds
      Firebase.setTimestamp(firebaseData, devicePath + "/wifi/lastConnect");
      z = 0;
    }
    
    /* --- LOCK UNLOCK PINTU --- */
    if (Firebase.getInt(firebaseData, devicePath + "/action/frontDoor")) {
      // Serial.print("Lock front: ");
      // Serial.println((firebaseData.intData()));
      digitalWrite(lockfront, firebaseData.intData());
    }
//    if (Firebase.getInt(firebaseData, devicePath + "/action/backDoor")) {
//      // Serial.print("Lock back: ");
//      // Serial.println((firebaseData.intData()));
//      digitalWrite(lockback, firebaseData.intData());
//    }
//
    /* --- PEMBACAAN SENSOR --- */
    int h = dht.readHumidity();
    int t = dht.readTemperature();
    int uv = 9; //TODO:
    Firebase.setInt(firebaseData, devicePath + "/sensor/humidity", h);
    Firebase.setInt(firebaseData, devicePath + "/sensor/temperature", t);
    Firebase.setInt(firebaseData, devicePath + "/sensor/uvIndex", uv);
    //Serial.print("Humi: );
    //Serial.print(h);
    //Serial.print(", Temp: ");
    //Serial.println(t);
    // Check if any reads failed and exit early (to try again).
    // if (isnan(h) || isnan(t)) {
    //   Serial.println("Failed to read from DHT sensor!");
    //   return;
    // }
    Firebase.setBool(firebaseData, devicePath + "/sensor/object", false);

    /* --- TOMBOL MANUAL DITEKAN --- */
    if (Firebase.getInt(firebaseData, devicePath + "/action/manualSteril")) {
      if (firebaseData.intData() == 1) { 
        Serial.println("Tombol manual steril ditekan");
        sterilState = true;
        uvState = true;
        Firebase.setInt(firebaseData, devicePath + "/action/manualSteril", 0);
      }
    }
    Serial.println("end");
  }

  /* --- PENDETEKSIAN BARANG --- */
  float volts = analogRead(sensor_sharp) * 0.00048828125; // value from sensor sharp * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph
  //Firebase.setInt(firebaseData, devicePath + "/sensor/distance", distance);
  //Serial.print("Dist: ");
  //Serial.println(distance);
  
  // if (Firebase.getInt(firebaseData, devicePath + "/sensor/distance")) {
  //   distance = firebaseData.intData(); //MODE TEST!!! 
  // }   
  if (distance <= 20) { //if any packet entering box
    Firebase.setBool(firebaseData, devicePath + "/sensor/object", true);
    Serial.println("New packet detected");

    Firebase.getString(firebaseData, devicePath + "/user");
    String user = firebaseData.stringData();
    //Post new data to log packet
    jsonData.set("status", "entering");
    jsonData.set("device", macAddress);
    jsonData.set("user", user);
    if (Firebase.pushJSON(firebaseData, "/packet", jsonData)) {
      //Serial.println(firebaseData.dataPath());
      lastPacketId = firebaseData.pushName();
      //Serial.println(lastPacketId);
      Serial.print("New data posted to RTD: ");
      Serial.println(firebaseData.dataPath() + "/"+ lastPacketId);
    } else {
      Serial.println(firebaseData.errorReason());
    }
    
    // Cek Mode 1 == OTOMATIS 0 == MANUAL
    if (Firebase.getInt(firebaseData, devicePath + "/auto")) {
      if ((firebaseData.intData()) == 1) { 
        Serial.println("System Otomatis");
        sterilState = true;
        uvState = true;
      }
      else {
        Serial.println("System Manual");
      }
    }
    delay(5000); //avoid bottleneck
  }
  else {
    // Serial.println("Tidak ada barang");
  }


  /* --- MODE STERILISASI --- */
  if (sterilState == true){ 
    Serial.println("Sedang sterilisasi");

    if (uvState == true) {
      digitalWrite(sinaruv, HIGH);
      Serial.println("Lampu UV aktif");
      Serial.println("Update status cleaning dimulai");
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/status", "cleaning");
      uvState = false;
      previousMillis = millis();
    }

    unsigned long currentMillis = millis();
    if (Firebase.getInt(firebaseData, devicePath + "/duration")) {
      interval = firebaseData.intData();
    }
    //if(currentMillis - previousMillis > interval*60000) { //in minutes
    if(currentMillis - previousMillis > interval*1000) { //in seconds: MODE TEST!!
      Serial.println("Lampu UV nonaktif");
      Serial.println("Update status sudah steril");
      digitalWrite(sinaruv, LOW);
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/status", "sterilized");
      sterilState = false;
    }
  } 

  Serial.println("x");
  delay(10); //delay all system

}
