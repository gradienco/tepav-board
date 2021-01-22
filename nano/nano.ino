#include <EEPROM.h>
#define sensor_sharp A3
#define relay_uv 21
#define led_sterilize 13
#define led_detector 12

//[TODO] Save to EEPROM
int mode_auto = 1; //1 == OTOMATIS 0 == MANUAL
int timer_duration = 30;

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Startup");

  EEPROM.begin(512);
  //Load last state EEPROM

  pinMode(relay_uv, OUTPUT);
  pinMode(led_sterilize, OUTPUT);
  pinMode(led_detector, OUTPUT);
  digitalWrite(relay_uv, 1);
  digitalWrite(led_sterilize, 0);
  digitalWrite(led_detector, 0);
}

void loop() {
  /* --- PENDETEKSIAN BARANG --- */
  float volts = analogRead(sensor_sharp) * 0.00048828125; // value from sensor sharp * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph

  if (distance <= 20) { //if any packet entering box
    //[TODO] Send new packet to ESP
    Serial.println("New packet detected");

    if (mode_auto == 1) { 
        Serial.println("System Otomatis");
        sterilState = true; //mode sterilisasi
        uvState = true;
      }
      else {
        Serial.println("System Manual");
      }
  } else {
      // Serial.println("Tidak ada barang");
  }

  /* --- MODE STERILISASI --- */
  if (sterilState == true){ 
    Serial.println("Sedang sterilisasi");

    if (uvState == true) {
      digitalWrite(relay_uv, HIGH);
      digitalWrite(led_sterilize, HIGH);
      Serial.println("Lampu UV aktif");
      Serial.println("Update status cleaning dimulai");
      //[TODO] Send cleaning state to ESP
      uvState = false;
      previousMillis = millis();
    }

    if(currentMillis - previousMillis > sterilState*1000) { //in seconds: MODE TEST!!
      Serial.println("Lampu UV nonaktif");
      Serial.println("Update status sudah steril");
      digitalWrite(relay_uv, LOW);
      digitalWrite(led_sterilize, LOW);
      //[TODO] Send finish state to ESP
      sterilState = false;
    }
  }

  /* --- RECEIVE CONFIG --- */
  


  
  delay(10);
}