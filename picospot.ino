// Spotprice
#include <TFT_eSPI.h>     // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>         // Include Wifi
#include <ArduinoMqttClient.h>
#include <string>
#include "Free_Fonts.h"
#include "conf.h"

TFT_eSPI tft = TFT_eSPI();

WiFiClient client;
MqttClient mqttClient(client);

//Screen is 480x320
int xpos = 85; 
int ypos = 65;
std::string kwvalue;
double  price = 100;
std::string kwvalue_in;
std::string kwvalue_ut;
double  kw_in = 0;
double  kw_ut = 0;
// flag 0:in 1:ut
int ut; 

char spotprice[50];
char kwtext[50];
const char broker[]    = "kilowh.at";
int        port        = 8883;
String topic;
const char inTopic[]   = "sodraskog/lisas/Power/Instant/Active/Positive/value";
const char inTopic2[]  = "sodraskog/lisas/Power/Instant/Active/Negative/value";
const char spotTopic[]  = "/spotpris/pris";

const long interval = 20000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  // SETUP SCREEN
  tft.begin();
  tft.setRotation(1);
  Serial.begin(9600);
 

  // Connect to WIFI
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // Connect to MQTT server
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  if (!mqttClient.connect(broker, port)) {
    while (1);
  }  

  // Set up MQTT
  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);
  // subscribe to topics
  // the second parameter sets the QoS of the subscription, the the library supports subscribing at QoS 0, 1, or 2
  int subscribeQos = 0;
  mqttClient.subscribe(inTopic, subscribeQos);
  mqttClient.subscribe(inTopic2, subscribeQos);
  mqttClient.subscribe(spotTopic, subscribeQos);

  // Call first update
  tft.fillScreen(0x79E0); // Clear screen
  updateScreen();
}

void loop() {
   // call poll() regularly to allow the library to receive MQTT messages and avoids being disconnected by the broker
  mqttClient.poll();
  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    count++;
  }
  
}

// Reads and classifies MQTT messages and update relevant variables.
void onMqttMessage(int messageSize) {
  // use the Stream interface to print the contents, break it at colon to get just number
  topic = mqttClient.messageTopic(); 
  kwvalue = "";
  while (mqttClient.available()) {
    char c = (char)mqttClient.read();
    // stop when receives a colon: number is over
    if (c == ':'){ break; }
    kwvalue += c;
  }       
  // Depending on topic update Positive or Negative or Spotprice
  if(topic == inTopic2) {
    kw_ut = std::stof(kwvalue);
    kwvalue_ut = kwvalue;
    updateScreen();
  } else if (topic == inTopic) {
    kw_in = std::stof(kwvalue);
    kwvalue_in = kwvalue;
    updateScreen();
  } else {   
    price = std::stof(kwvalue);
  }
  
}

// Update screen with KW data
void updateScreen(){
  tft.setTextColor(0x0000, 0x79E0);
  if (kw_ut > 0){
      tft.setCursor(xpos, ypos);
      tft.setFreeFont(FSS24);
      sprintf(kwtext, "%0.03f", kw_ut);
      tft.drawString(kwtext, xpos, ypos, GFXFF);
      if (ut = 0){
        tft.setFreeFont(FSS18);
        tft.drawString("kw ut", 120, 160, GFXFF);
        ut = 1;
      }
  }else{
      tft.setCursor(xpos, ypos);
      tft.setFreeFont(FSS24);
      sprintf(kwtext, "%0.03f", kw_in);
      tft.drawString(kwtext, xpos, ypos, GFXFF);
      if (ut = 1){
        tft.setFreeFont(FSS18);
        tft.drawString("kw in", 140, 180, GFXFF);
        ut = 0;
      }
  }

}

// Changes background color dependeing on status
void updateColor(){
  
}
