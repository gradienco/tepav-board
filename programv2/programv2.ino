//deklarasi firebase
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <FirebaseJSON.h>
#include <EEPROM.h>

#define FIREBASE_HOST "https://tepav-6171f.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyB9VrEW4KzCoaF6HDMtNB9rh_uF6UyXTyA"
FirebaseData firebaseData;
unsigned long waktusebelum = 0;
FirebaseJson jsonData;
//Firebase variable
String macAddress;
String devicePath;
/*
const char* userPath;
const char* humidityPath;
const char* temperaturePath;
const char* objectPath;
const char* uvPath;
const char* btnManualPath;
const char* modePath;
const char* durationPath;
const char* ssidPath;
const char* passwordPath;
const char* lockFrontPath;
const char* lockBackPath;
*/
String lastPacketId;

#define lockfront 23
#define lockback 22
#define sinaruv 21
#define sensor_pintu 19
#define sensor_sharp A3
#define sensor_uv A0
#define uvlight 13

int state = 0;
int barangbersih = 0;
unsigned long interval = 30; //30 minutes convert to milisecond
long previousMillis = 0;     
bool ledState = LOW;   
int ledUV = 1;
bool sterilState = false;
bool uvState = false;

//dht22
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Bluetooth
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

//Wifi
const char* ssid     = "HKTI PROVINSI"; //default for developers
const char* password = "tanijaya";
String esid;
String epass;
char wifi[3][32];

void setup() {
  Serial.begin(9600);
  delay(10);

  //-------------------------Bluetooth Connection
  SerialBT.begin("Tepav"); 
  //Serial.println("Bluetooth started!");

  //------------------------- Wifi Connection
  WiFi.disconnect();
  EEPROM.begin(512);
  //Serial.println("Startup");
  delay(10);
  //ssd load from eeprom
  //Serial.println("Reading EEPROM ssid");
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  // Serial.println();
  // Serial.print("SSID: ");
  // Serial.println(esid);
  // Serial.println("Reading EEPROM password");
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  // Serial.print("PASS: ");
  // Serial.println(epass);
  WiFi.begin(esid.c_str(), epass.c_str());
  //WiFi.begin(ssid, password);

  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print("..");
  }
  // Serial.println("Connected");
  // Serial.println("Your IP is");
  // Serial.println((WiFi.localIP()));

  //--------------------------------Firebase Connection
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);//Declare device id
  macAddress = WiFi.macAddress();
  devicePath = "/device/"+ macAddress;
  /*
  userPath = devicePath + "/user";
  temperaturePath =  devicePath + "/sensor/temperature";
  humidityPath = devicePath + "/sensor/humidity";
  objectPath = devicePath + "/sensor/object";
  uvPath = devicePath + "/sensor/uvi";
  btnManualPath = devicePath + "/action/btnManual";
  modePath = devicePath + "/mode";
  durationPath = devicePath + "/duration";
  ssidPath = devicePath + "/wifi/ssid";
  passwordPath = devicePath + "/wifi/password";
  lockFrontPath = devicePath + "/action/frontDoor";
  lockBackPath = devicePath + "/action/backDoor";
  */

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

  /* --- LOCK UNLOCK PINTU --- */
  if (SerialBT.available()) {
    // Serial.println("Incoming Bluetooth Data");
    int x = 0;
    int y = 0;
    while (SerialBT.available() > 0){
      const char byteRead = SerialBT.read();
      if (byteRead==';') {
        break;
      } else if (byteRead==':') {
        x++;
        y = 0;
      } else {
        wifi[x][y] = byteRead;
        y++;
      }
      //Serial.write(SerialBT.read());    
    }
    // Serial.print("SSID: ");
    // Serial.println(wifi[0]);
    // Serial.print("Password: ");
    // Serial.println(wifi[1]);
    Serial.flush();
    //save to EEPROM
    // Serial.println("Writing eeprom ssid:");
    for (int i = 0; i < sizeof(wifi[0]); ++i)
        {
          EEPROM.write(i, wifi[0][i]);
          // Serial.print("Wrote: ");
          // Serial.println(wifi[0][i]);
        }
    // Serial.println("Writing eeprom pass:");
    for (int i = 0; i < sizeof(wifi[1]); ++i)
    {
      EEPROM.write(32 + i, wifi[1][i]);
      // Serial.print("Wrote: ");
      // Serial.println(wifi[1][i]);
    }
    EEPROM.commit();
    //push to Database
    Firebase.setString(firebaseData, devicePath + "/wifi/ssid", wifi[0]);
    Firebase.setString(firebaseData, devicePath + "/wifi/password", wifi[1]);
    //Firebase.setString(firebaseData, devicePath + "/duration", wifi[2]);
    // Serial.println("Data stored to Database");
    // Serial.println("Now Restarting...");
    delay(1000);
    ESP.restart();
  }

  /* --- LOCK UNLOCK PINTU --- */
  unsigned long waktusekarang = millis();
  if (waktusekarang - waktusebelum > 1000) { //ini untuk apa jo?*
    waktusebelum = waktusekarang;
    lock();// 0 == open 1 == lock
  }


  /* --- PEMBACAAN SENSOR --- */
  //delay(1000); // slow down serial port
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  int uv = 9;
  Firebase.setInt(firebaseData, devicePath + "/sensor/humidity", h);
  Firebase.setInt(firebaseData, devicePath + "/sensor/temperature", t);
  Firebase.setInt(firebaseData, devicePath + "/sensor/uvi", uv);
  //Serial.print("Humi: );
  //Serial.print(h);
  //Serial.print(", Temp: ");
  //Serial.println(t);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  /* --- PENDETEKSIAN BARANG --- */
  float volts = analogRead(sensor_sharp) * 0.00048828125; // value from sensor sharp * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph
  //Serial.print("Dist: ");
  //Serial.println(distance);
  if (distance <= 20 && barangbersih == 0) { //if any packet entering box
    Firebase.setBool(firebaseData, devicePath + "/sensor/object", true);
    // Serial.println("New packet detected");

    Firebase.getString(firebaseData, devicePath + "/user");
    String user = firebaseData.stringData();
    //Post new data to log packet
    jsonData.set("status", "incoming");
    jsonData.set("device", macAddress);
    jsonData.set("user", user);
    if (Firebase.pushJSON(firebaseData, "/packet", jsonData)) {
      //Serial.println(firebaseData.dataPath());
      lastPacketId = firebaseData.pushName();
      //Serial.println(lastPacketId);
      // Serial.println("New data posted to RTD");
      // Serial.println(firebaseData.dataPath() + "/"+ lastPacketId);
    } else {
      // Serial.println(firebaseData.errorReason());
    }
    
    // cek manual atau otomatis 1 == manual 0 == otomatis
    if (Firebase.getInt(firebaseData, devicePath + "/mode")) {
      if ((firebaseData.intData()) == 0) { //check mode otomatis
        // Serial.println("System otomatis");
        sterilState = true;
        uvState = true;
      }
      else {
        // Serial.println("System Manual");
      }
    }
  }
  else {
    // Serial.println("Tidak ada barang");
  }



  /* --- TOMBOL MANUAL DITEKAN --- */
  if (Firebase.getInt(firebaseData, devicePath + "/action/manualSteril")) {
    if ((firebaseData.intData()) == 1) { 
      // Serial.println("Tombol manual steril ditekan");
      sterilState = true;
      uvState = true;
      Firebase.setInt(firebaseData, devicePath + "/action/manualSteril", 0);
    }
  }



  /* --- MODE STERILISASI --- */
  if (sterilState == true){ 
    // Serial.println("Sedang sterilisasi");

    if (uvState == true) {
      digitalWrite(sinaruv, HIGH);
      // Serial.println("Lampu UV aktif");
      // Serial.println("Update status cleaning dimulai");
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/log/status", "cleaning");
      //TODO: timestamp
      uvState = false;
      previousMillis = millis();
    }

    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      // Serial.println("Lampu UV nonaktif");
      // Serial.println("Update status sudah steril");
      digitalWrite(sinaruv, LOW);
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/log/status", "sterilized");
      //TODO: timestamp
      sterilState = false;
    }
  } 

  delay(100); //delay all system

}
