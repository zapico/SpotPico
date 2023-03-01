// Spotprice
#include <TFT_eSPI.h>     // Hardware-specific library
#include <SPI.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>       // Include Wifi
#include <ArduinoMqttClient.h>
#include <string>
#include "conf.h" 

#define inTopic "smartmeter/meterkwh/Power/Instant/Active/Positive/value"
#define inTopic2 "smartmeter/meterkwh/Power/Instant/Active/Negative/value"
#define broker "kilowh.at"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
WiFiClient client;
MqttClient mqttClient(client);
// Add fonts to LittleFS beforehand
#define AA_FONT_SMALL "G86"
#define AA_FONT_LARGE "G120"
//#define red 0x2e46 //Reversed green
//#define green 0xd906 //reversed red
//#define blue 0x79E0 //reversed brown!

//Screen is 480x320
int xpos = 150;
int ypos = 65;

// Variables used for storing kw values and price
std::string kwvalue;
std::string kwvalue_in;
std::string kwvalue_ut;
String topic;
double  kw_in = 0.01;
double  kw_ut = 0.01;
int screen_flag = 0; // flag 0:in 1:ut
char kwtext[10];

// Configuration for MQTT
int port  = 8883;
const long interval = 10000;
unsigned long previousMillis = 0;
int count = 0;


void setup() {
  // Start up filesystem
  LittleFS.begin();
  WiFi.persistent(true);
  // Setup screen
  tft.begin();
  tft.setRotation(1);
  spr.setColorDepth(32);
  tft.fillScreen(0x03EF); // Clear screen
  
  // Start serial for debugging
  Serial.begin(9600);
  // Connect to WIFI
  WiFi.begin(ssid, pass);
  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqttConnect();


  // Call first update
  
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
    mqttReconnect();
    updateScreen();
  }
}

void updateScreen() {
  Serial.println("Updatekw");
  char c = '\t';
  if (kw_ut > 0) {
    if (screen_flag == 0) {
      tft.fillScreen(0xC82F);
      screen_flag = 1;
    }
    spr.loadFont(AA_FONT_SMALL); // Must load the font first into the sprite class
    spr.setTextColor(0x0000, 0xC82F);
    tft.setCursor(xpos, ypos);          // Set the tft cursor position, yes tft position!
    sprintf(kwtext, "%.03f", kw_ut);
    spr.printToSprite("  " + String(kwtext) + "  ");  // Prints to tft cursor position, tft cursor NOT moved
    tft.setCursor(xpos + 30, ypos + 120);
    spr.printToSprite(" kw ut ");
    spr.unloadFont();

  } else {
    if (screen_flag == 1) {
      tft.fillScreen(0x03EF);
      screen_flag = 0;
    }
    spr.loadFont(AA_FONT_SMALL); // Must load the font first into the sprite class
    spr.setTextColor(0x0000, 0x03EF);
    tft.setCursor(xpos, ypos);          // Set the tft cursor position, yes tft position!
    sprintf(kwtext, "%.03f", kw_in);
    spr.printToSprite("  " + String(kwtext) + "  ");    // Prints to tft cursor position, tft cursor NOT moved
    tft.setCursor(xpos + 30, ypos + 120);
    spr.printToSprite(" kw in ");
    spr.unloadFont();
    //spr.loadFont(AA_FONT_SMALL);
    //spr.printToSprite(" kw in ");
    //spr.unloadFont();
  }
}
void updatepris() {
  Serial.print("Update_price ");
  tft.fillScreen(0xF800);
  spr.loadFont(AA_FONT_LARGE); // Must load the font first into the sprite class
  spr.setTextColor(0x0000, 0xF800);
  tft.setCursor(xpos, ypos + 50);        // Set the tft cursor position, yes tft position!
  //sprintf(spotprice, "%s%d%s", ' ', price, ' ');
  spr.printToSprite("SPOT");  // Prints to tft cursor position, tft cursor NOT moved
  spr.unloadFont();
}
void mqttReconnect() {
  //Make sure we stay connected to the mqtt broker
  if (!mqttClient.connected()) {
    mqttConnect();
  }
}

void mqttConnect() {
  Serial.println("Start connection to mqtt");
  // Connect to MQTT server
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  if (!mqttClient.connect(broker, port)) {
    while (1);
    Serial.print("connecting to mqtt");
  }
  // Set up MQTT
  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);
  // subscribe to topics
  // the second parameter sets the QoS of the subscription, the the library supports subscribing at QoS 0, 1, or 2
  int subscribeQos = 0;
  mqttClient.subscribe(inTopic, subscribeQos);
  mqttClient.subscribe(inTopic2, subscribeQos);
}


// Reads and classifies MQTT messages and update relevant variables.
void onMqttMessage(int messageSize) {
  // use the Stream interface to print the contents, break it at colon to get just number
  topic = mqttClient.messageTopic();
  kwvalue = "";
  while (mqttClient.available()) {
    char c = (char)mqttClient.read();
    // stop when receives a colon: number is over
    if (c == ':') {
      break;
    }
    kwvalue += c;
  }
  // Depending on topic update Positive or Negative or Spotprice
  if (topic == inTopic2) {
    kw_ut = std::stof(kwvalue);
    kwvalue_ut = kwvalue;
  } else if (topic == inTopic) {
    kw_in = std::stof(kwvalue);
    kwvalue_in = kwvalue;
  }
  Serial.println("Updated mqtt");
}
