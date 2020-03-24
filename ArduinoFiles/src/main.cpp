#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

#define BREAKOUT_RESET  8     
#define BREAKOUT_CS     6     
#define BREAKOUT_DCS    7      
#define CARDCS          9     
#define DREQ            2

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

void printDirectory(File dir, int numTabs);

int volL = 255;
int volR = 255;

Servo servo;
int servo_pos = 90;
bool servo_dir = false;

int min_dist = 80;
bool obj_found = false;

void setup() {
  SPI.begin();
  Serial.begin(9600);
  servo.attach(3);
  servo.write(servo_pos);

  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);

  if (! musicPlayer.begin()) {
     Serial.println(F("Couldn't find VS1053"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  
  }

  printDirectory(SD.open("/"), 0);
  musicPlayer.setVolume(10,10);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
  musicPlayer.startPlayingFile("/beep.mp3");
}

void loop() {
  float voltage = analogRead(A0);
  float distance = pow(19661*voltage, -1.083)*1000000000;
  
  servo.write(servo_pos);
  if (servo_pos < 40) servo_dir = true;
  if (servo_pos > 140) servo_dir = false;
  if (servo_dir) servo_pos++;
  else servo_pos--;

  if (distance < 30) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if (distance < min_dist) {
    min_dist = distance;
    obj_found = true;
    volL = 100 - (servo_pos - distance*3);
    volR = 100 - (180-servo_pos - distance*3);
  }

  if (musicPlayer.stopped() && obj_found) {
      musicPlayer.setVolume(volL,volR);
      musicPlayer.startPlayingFile("/beep3.mp3");
  } 
  if (!obj_found) {
    musicPlayer.stopPlaying();
  }

  if (distance > 80) {
    min_dist = 80;
    obj_found = false;
  }

  // Serial.print("Distance: ");
  // Serial.print(distance);
  // Serial.print(" Servo pos: ");
  // Serial.println(servo_pos);

  delay(40);
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

