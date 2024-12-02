
#include <ESP32Servo.h>
Servo myservo; //ประกาศตัวแปรแทน Servo

const int inputPin = 4;    // Digital input pin to check
const int openButton = 5;
const int survoPin = 15;

const int ledRed = 18;
const int ledYellow = 19;
const int ledGreen = 21;

// Variables for timing
unsigned long previousMillis = 0; 
unsigned long switchMillis = 0;
unsigned long sentMillis = 0;
const long interval = 10000; // 10 second interval

int roomStatus = 0; // 0:Green 1:Yellow 2:Red
bool switchStatus = false;

#define TXD1 12
#define RXD1 13

HardwareSerial mySerial(1);

int counter = 0;


void setup() {
  myservo.attach(survoPin); 
  pinMode(inputPin, INPUT);   // Set inputPin as input
  pinMode(openButton, INPUT); 
  pinMode(ledGreen, OUTPUT);  
  pinMode(ledYellow, OUTPUT); 
  pinMode(ledRed, OUTPUT); 
  // pinMode(ledPin, OUTPUT);    // Set ledPin as output
  Serial.begin(4800);       // Start serial communication
  Serial.println("Test");
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledRed, LOW);
  mySerial.begin(2400, SERIAL_8N1, RXD1, TXD1); 
  myservo.write(0);
}

void loop() {
  unsigned long currentMillis = millis();
  // Serial.println(currentMillis);
  int openButtonRead = digitalRead(openButton);
  if(!openButtonRead && !switchStatus && currentMillis - switchMillis >= 1000){
    switchMillis = currentMillis;
    switchStatus = true;
    if(roomStatus == 2) {
      roomStatus = 0;
      Serial.println("Room Unlock");
      digitalWrite(ledRed, LOW); 
      digitalWrite(ledYellow, LOW); 
      digitalWrite(ledGreen, HIGH);
      myservo.write(0);
    }
    else {
      roomStatus = 2;
      Serial.println("Room Lock");
      digitalWrite(ledRed, HIGH); 
      digitalWrite(ledYellow, LOW); 
      digitalWrite(ledGreen, LOW);
      myservo.write(90);
    }
  }
  if(switchStatus && openButtonRead) {
    switchStatus = false;
  }

  // delay(2000);
  
  if (currentMillis - previousMillis >= interval && roomStatus == 1) {
    roomStatus = 0;
    Serial.println("Room Available");

    previousMillis = currentMillis;
    digitalWrite(ledGreen, HIGH); // Toggle LED
    digitalWrite(ledYellow, LOW); 
    digitalWrite(ledRed, LOW);
    // Serial.println("1 second elapsed");
  }

  int peopleActive = digitalRead(inputPin);
  if(peopleActive && roomStatus != 2){
    Serial.println("Sensor detect people");
    previousMillis = currentMillis;
    if(roomStatus == 0) {
      roomStatus = 1;
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, HIGH); 
      digitalWrite(ledRed, LOW);
    }
  }

  if (currentMillis - sentMillis >= 500) {
    sentMillis = currentMillis;

    mySerial.println(String(roomStatus));
    Serial.println("Sent: " + String(roomStatus));
  }
}