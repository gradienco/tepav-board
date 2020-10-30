//deklarasi firebase
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <FirebaseJSON.h>

#define FIREBASE_HOST "https://tepav-6171f.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyB9VrEW4KzCoaF6HDMtNB9rh_uF6UyXTyA"
FirebaseData firebaseData;
unsigned long waktusebelum = 0;
FirebaseJson jsonData;

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
String macAddress;
String devicePath;
String userPath;
String humidityPath;
String temperaturePath;
String objectPath;
String uvPath;
String btnManualPath;
String lastPacketId;

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

void setup() {
  Serial.begin(9600);
  pinMode(sensor_pintu, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  pinMode(sinaruv, OUTPUT);
  pinMode(lockfront, OUTPUT);
  pinMode(lockback, OUTPUT);

  //inisiasi firebase
  WiFi.disconnect();
  delay(3000);
  Serial.println("START");
  WiFi.begin("HKTI PROVINSI", "tanijaya");
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print("..");
  }
  Serial.println("Connected");
  Serial.println("Your IP is");
  Serial.println((WiFi.localIP()));
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  dht.begin();

  digitalWrite(lockfront, 1);
  digitalWrite(lockback, 1);
  digitalWrite(sinaruv, 1);

  //Declare device id
  macAddress = WiFi.macAddress();
  devicePath = "/device/"+ macAddress;
  userPath = devicePath + "/user";
  temperaturePath =  devicePath + "/sensor/temperature";
  humidityPath = devicePath + "/sensor/humidity";
  objectPath = devicePath + "/sensor/object";
  uvPath = devicePath + "/sensor/uvi";
  btnManualPath = devicePath + "/action/btnManual";
  modePath = devicePath + "/modeSteril";
}

void loop() {

  /* --- LOCK UNLOCK PINTU --- */
  unsigned long waktusekarang = millis();
  if (waktusekarang - waktusebelum > 1000) { //ini untuk apa jo?*
    waktusebelum = waktusekarang;
    lock();// 0 == open 1 == lock
  }


  /* --- PEMBACAAN SENSOR --- */
  //delay(1000); // slow down serial port
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // int uv = ;
  Firebase.setFloat(firebaseData, humidityPath, h);
  Firebase.setFloat(firebaseData, temperaturePath, t);
  //Firebase.setInt(firebaseData, uvPath, uv);
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
    Firebase.setBool(firebaseData, objectPath, true);
    Serial.println("New packet detected");

    Firebase.getString(firebaseData, userPath);
    String user = firebaseData.stringData();
    //Post new data to log packet
    jsonData.set("status", "barang masuk");
    jsonData.set("device", macAddress);
    jsonData.set("user", user);
    if (Firebase.pushJSON(firebaseData, "/packet", jsonData)) {
      //Serial.println(firebaseData.dataPath());
      lastPacketId = firebaseData.pushName();
      //Serial.println(lastPacketId);
      Serial.println("New data posted to RTD");
      Serial.println(firebaseData.dataPath() + "/"+ lastPacketId);
    } else {
      Serial.println(firebaseData.errorReason());
    }
    
    // cek manual atau otomatis 1 == manual 0 == otomatis
    if (Firebase.getInt(firebaseData, modePath)) {
      if ((firebaseData.intData()) == 0) { //check mode otomatis
        Serial.println("System otomatis");
        sterilState = true;
        uvState = true;
      }
      else {
        Serial.println("System Manual");
      }
    }
  }
  else {
    // Serial.println("Tidak ada barang");
  }



  /* --- TOMBOL MANUAL DITEKAN --- */
  if (Firebase.getInt(firebaseData, btnManualPath)) {
    if ((firebaseData.intData()) == 1) { 
      Serial.println("Tombol manual steril ditekan");
      sterilState = true;
      uvState = true;
      Firebase.setInt(firebaseData, btnManualPath, 0);
    }
  }



  /* --- MODE STERILISASI --- */
  if (sterilState == true){ 
    Serial.println("Sedang sterilisasi");

    if (uvState == true) {
      digitalWrite(sinaruv, HIGH);
      Serial.println("Lampu UV aktif");
      Serial.println("Update status cleaning dimulai");
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/log/status", "clening dimulai");
      //TODO: timestamp
      uvState = false;
      previousMillis = millis();
    }

    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      Serial.println("Lampu UV nonaktif");
      Serial.println("Update status sudah steril");
      digitalWrite(sinaruv, LOW);
      Firebase.setString(firebaseData, "/packet/"+lastPacketId+"/log/status", "sudah steril");
      //TODO: timestamp
      sterilState = false;
    }
  } 

  delay(100); //delay all system

}
