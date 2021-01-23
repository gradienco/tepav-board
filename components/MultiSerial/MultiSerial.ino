/*
  Multiple Serial test

  Receives from the main serial port, sends to the others.
  Receives from serial port 1, sends to the main serial (Serial 0).

  This example works only with boards with more than one serial like Arduino Mega, Due, Zero etc.

  The circuit:
  - any serial device attached to Serial port 1
  - Serial Monitor open on Serial port 0

  created 30 Dec 2008
  modified 20 May 2012
  by Tom Igoe & Jed Roach
  modified 27 Nov 2015
  by Arturo Guadalupi

  This example code is in the public domain.
*/

unsigned long previousMillis = 0;
int x=0;
int str[2]={0,0};

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial.println("Start");
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial2.available()) {
    //int inByte = Serial2.read();
    //Serial.write(inByte);
    delay(10);

    while (Serial2.available() > 0) {
        int byteRead = Serial2.read();
        delay(10);
        Serial.write(byteRead);

        if (byteRead=='M'){
            Serial.println("Tombol manual steril ditekan");
            //sterilState = true;
            //uvState = true;
            //break;
        }
        else if(isdigit(byteRead))
            str[x] = (str[x]*10)+(byteRead-48);
        else if (byteRead==':'){ //separate data
            //break;
            x++;
        }
        else if (byteRead==';'){
            //1 = config mode_auto
            //2 = config timer_duration
            if (str[0] == 1) {
                //mode_auto = str[1];
                Serial.print("Mode auto: ");
                Serial.println(str[1]);
                //EEPROM.write(1, str[1]);
            }
            else if (str[0]== 2) {
                //timer_duration = str[1];
                Serial.print("Timer duration: ");
                Serial.println(str[1]);
                //EEPROM.write(2, str[1]);
            }
            x = 0; //reset string every ; handle bottleneck
            str[0]=0;str[1]=0; //cache var
        }
    }
    x = 0; //reset string
    Serial2.flush();
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial2.write(inByte);
  }

//  unsigned long currentMillis = millis();
//  if(currentMillis - previousMillis > 1000) {
//    previousMillis = millis();
//    Serial2.write("2:");
//    Serial2.write("100");
//    Serial2.write(";");
//    Serial2.write(4);
//  }

  delay(5000);
}
