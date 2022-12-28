#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define ARDUINOJSON_USE_DOUBLE 0
// @ref: https://www.emqx.com/en/blog/esp8266-connects-to-the-public-mqtt-broker
// @Ref: https://sites.google.com/site/wenyumaker/03-pm2-5灰塵傳感器/01-g3-uno?pli=1

SoftwareSerial mySerial(D6, D7);  // RX, TX
// WiFi
const char *ssid = "IoTDemo";       // Enter your WiFi name
const char *password = "12345678";  // Enter WiFi password

int orp   = 0;   //氧化還原電位 real mV
int ss    = 0;   //懸浮固體
double oxy = 0.0; //溶氧
double ph  = 0.0; //酸鹼
int tds   = 0;   //可溶性
double nh3 = 0.0; //氨氮  ~1 ppm
int hard  = 0;   //硬度  100~150 nu


// MQTT Broker
const char *mqtt_broker = "homeassistant.local";
const char *topic = "/AquaBOX";
const char *mqtt_username = "admin";
const char *mqtt_password = "admin";
const int mqtt_port = 1883;

unsigned long long pre_time2000 = 0;
unsigned long long now_time = 0;

char buf[50];

WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument doc(1024);
DynamicJsonDocument orp_json(256);
char temp[1024];

//CF=1 根据美国TSI公司的仪器校准
//大气环境下 根据中国气象局的数据校准

void sendToMQTT() {
  if (now_time - pre_time2000 > 2000) {
    //String temp = String(millis());
    ss   = random(700, 900);      //懸浮固體 700~900
    oxy  = (double)random(70, 150) / 10.0; //溶氧 7~15 ppm
    ph   = (double)random(10, 140) / 10.0; //酸鹼 1~14
    tds  = random(1000, 1200);    //可溶性 1000~1200 ppm
    nh3  = (double)random(0, 20) / 10.0;   //氨氮  <2 ppm
    hard = random(100, 150);      //硬度  100~150
    
    doc["orp"] = orp;
    doc["ss"]  = ss;
    doc["oxy"] = oxy;
    doc["ph"]  = ph;
    doc["tds"] = tds;
    doc["nh3"] = nh3;
    doc["hard"]= hard;
    serializeJson(doc, temp);
    client.publish(topic, temp);
    pre_time2000 = now_time;
  }
}

void getORPvalue() {
  String temp;
  if(mySerial.available())
  {
    temp = mySerial.readStringUntil('\n');
    deserializeJson(orp_json, temp.c_str());
    orp = orp_json["ORP"];
    //Serial.println(temp.c_str());
    //Serial.println(orp);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);

  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  client.loop();
  now_time = millis();
  getORPvalue();
  sendToMQTT();
}
