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

  digitalWrite(lockfront, 1);
  digitalWrite(lockback, 1);
  digitalWrite(sinaruv, 1);

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
}

void loop() {
  lock();// 0 == open 1 == lock

  //sensor sharp
  float volts = analogRead(sensor_sharp) * 0.00048828125; // value from sensor * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph
  //Serial.println(distance);
  delay(1000); // slow down serial port
  if (distance <= 20 && barangbersih == 0) {
    Serial.println("Ada Barang");
  }
  else{
    Serial.println("Gaada Barang");
  }
}
