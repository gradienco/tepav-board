//deklarasi firebase
#include <WiFi.h>
#include <FirebaseESP32.h>
#define FIREBASE_HOST "https://iotbox-e3bbd.firebaseio.com/"
#define FIREBASE_AUTH "T9cz72ToRljGDoF5CpgBrss5yH6m5dflf2CRo9OZ"
FirebaseData firebaseData;
unsigned long waktusebelum = 0;

#define lockfront 23
#define lockback 22
#define sinaruv 21

#define sensor_pintu 19
#define sensor_sharp A3
#define sensor_uv A0
#define uvlight 13

int state = 0;
int barangbersih = 0;

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
}

void loop() {
  lock();// 0 == open 1 == lock

  //sensor sharp
  float volts = analogRead(sensor_sharp) * 0.00048828125; // value from sensor * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph
  //Serial.println(distance);
  //delay(1000); // slow down serial port
  if (distance <= 20 && barangbersih == 0) {
    Firebase.setBool(firebaseData, "/device/device_a/sensor/object", true);
    Firebase.setString(firebaseData, "/packet/packet_a/log/status", "barang masuk");
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Firebase.setFloat(firebaseData, "/device/device_a/sensor/humidity", h);
    Firebase.setFloat(firebaseData, "/device/device_a/sensor/temperature", t);
    //    Serial.print(h);
    //    Serial.print("    ");
    //    Serial.println(t);
    delay(1000);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // cek manual atau otomatis 1 == manual 0 == otomatis
    if (Firebase.getInt(firebaseData, "system_manual")) {

      unsigned long waktusekarang = millis();

      if ((firebaseData.intData()) == 1) {
        //Serial.println("System Manual");
        //  cek tombol ditekan atau tidak 1== ditekan 0==tidak ditekan
        if (Firebase.getInt(firebaseData, "manual_button")) {

          if ((firebaseData.intData()) == 1) {
            Serial.println("Sinar uv hidup");
            digitalWrite(sinaruv, 0);
            delay(3000);
            digitalWrite(sinaruv, 1);
            delay(1000);
          }
          else {
            Serial.println("Sinar Uv Mati");
            digitalWrite(sinaruv, 1);
            delay(1000);
          }
        }
      }
      else {
        Serial.println("System Otomatis");
      }

    }

  }
  else {
    Serial.println("Gaada Barang");
  }
}
