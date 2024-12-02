// library : Adafadafruit unified sensor , DHT sensor library from Adafruit , MQ135
// board : Aruino ESP32 Boards , esp32

#include "DHT.h"
// #include "MQ135.h"

#include <Firebase_ESP_Client.h>
#include <HTTPClient.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// #define PIN_MQ135 12

// MQ135 mq135_sensor(PIN_MQ135);

#define DHTPIN 4

#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);

#define WIFI_SSID "KORKAEW22_2.4G"
#define WIFI_PASSWORD "21322133"

#define API_KEY "AIzaSyAiM5ZKwGSgOHp9vzCjyoE9BxRA2G8E-YY"
#define DATABASE_URL "https://smarthome-embedded-default-rtdb.asia-southeast1.firebasedatabase.app/" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

String Web_App_URL = "https://script.google.com/macros/s/AKfycbwBCspWDDEPi5bJbPhNWHIUtKszkuQeqpWWQ2ik2ddPPE4lfASfQpKf1Y7z4BjBy8LJ/exec?";

unsigned int lastUpdateMills = 0;
unsigned int sendDataPrevMillis = 0;
unsigned int get = 0;

String Status_Read_Sensor = "-";
bool signupOK = false;

float h = 0;
float t = 0;
float correctedPPM = 0;

unsigned short int preRoomStatus = 0;
unsigned short int telephoneUse = 0;
unsigned short int telephoneLock = 0;

#define TXD1 21
#define RXD1 19

HardwareSerial mySerial(2);

void setup() {
  Serial.begin(4800);
  int retries = 0;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(1000);
    retries++;
    Serial.println("Connecting...");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to Wi-Fi. Check credentials or signal.");
    while (true); // Halt the program or reset.
  }

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  mySerial.begin(2400, SERIAL_8N1, RXD1, TXD1);  // UART setup
  Serial.println("ESP32 UART Receiver");

  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateMills >= 1000) {
    lastUpdateMills = currentMillis;
    h = dht.readHumidity();
    t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      Status_Read_Sensor = 'Some Fail';
      return;
    } else {
      Status_Read_Sensor = 'Success';
    }

    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);

    // float rzero = mq135_sensor.getRZero();
    // float correctedRZero = mq135_sensor.getCorrectedRZero(t, h);
    // float resistance = mq135_sensor.getResistance();
    // float ppm = mq135_sensor.getPPM();
    // correctedPPM = mq135_sensor.getCorrectedPPM(t, h);

    float rzero = 0;
    float correctedRZero = 0;
    float resistance = 0;
    float ppm = 0;
    correctedPPM = 0;

    Serial.print(h);
    Serial.print(t);
    Serial.println("ppm");

    Firebase.RTDB.setFloat(&fbdo,"office/humidity", h);
    Firebase.RTDB.setFloat(&fbdo,"office/temperature_celsius", t);
    Firebase.RTDB.setFloat(&fbdo,"office/temperature_fahrenheit", f);
    Firebase.RTDB.setFloat(&fbdo,"office/corrected_ppm", correctedPPM);
  }

  if (currentMillis - sendDataPrevMillis >= 60000) {
    sendDataPrevMillis = currentMillis;
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&srs=" + Status_Read_Sensor;
    Send_Data_URL += "&temp=" + String(t);
    Send_Data_URL += "&humd=" + String(h);
    Send_Data_URL += "&ppm=" + String(correctedPPM);
    Send_Data_URL += "&tuse=" + String(telephoneUse);
    Send_Data_URL += "&tlock=" + String(telephoneLock);

    HTTPClient http;
    http.begin(Send_Data_URL); // Initialize HTTP request
    int httpResponseCode = http.GET(); // Send GET request
    // Check the response
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString(); // Get response payload
      Serial.println("Response:");
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }


  String message = mySerial.readStringUntil('\n');
  if (mySerial.available()) {
    Serial.println(message);
    float ss = message.toFloat();
    Firebase.RTDB.setFloat(&fbdo, "/telroom/2/isAvailability", ss);

    if(preRoomStatus == 0 && ss == 1) {
      telephoneUse += 1;
    } else if (preRoomStatus == 0 && ss == 2) {
      telephoneLock += 1;
    } else if (preRoomStatus == 1 && ss == 2) {
      telephoneLock += 1;
    }
    preRoomStatus = ss;
  }
}


