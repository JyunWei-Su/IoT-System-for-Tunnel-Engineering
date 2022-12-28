#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// @ref: https://www.emqx.com/en/blog/esp8266-connects-to-the-public-mqtt-broker
// @Ref: https://sites.google.com/site/wenyumaker/03-pm2-5灰塵傳感器/01-g3-uno?pli=1

SoftwareSerial mySerial(D6, D7);  // RX, TX
// WiFi
const char *ssid = "IoTDemo";       // Enter your WiFi name
const char *password = "12345678";  // Enter WiFi password

long pmcf10 = 0;
long pmcf25 = 0;
long pmcf100 = 0;

// MQTT Broker
const char *mqtt_broker = "homeassistant.local";
const char *topic = "/SensorBOX";
const char *mqtt_username = "admin";
const char *mqtt_password = "admin";
const int mqtt_port = 1883;

unsigned long long pre_time5000 = 0;
unsigned long long pre_time2000 = 0;
unsigned long long now_time = 0;

char buf[50];


WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument doc(96);
char temp[96];


//CF=1 根据美国TSI公司的仪器校准
//大气环境下 根据中国气象局的数据校准

void sendToMQTT() {
  if (now_time - pre_time2000 > 2000) {
    //String temp = String(millis());
    
    pre_time2000 = now_time;
    doc["PM1.0"] = pmcf10;
    doc["PM2.5"] = pmcf25;
    doc["PM10"] = pmcf100;
    serializeJson(doc, temp);
    client.publish(topic, temp);
  }
}

void getPMvalue() {
  if (now_time - pre_time5000 > 5000) {
    int count = 0;
    unsigned char c;
    unsigned char high;
    while (mySerial.available()) {
      c = mySerial.read();
      if ((count == 0 && c != 0x42) || (count == 1 && c != 0x4d)) {
        Serial.println("check failed");
        break;
      }
      if (count > 15) {
        Serial.println("complete");
        break;
      } else if (count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) high = c;
      else if (count == 5) {
        pmcf10 = 256 * high + c;
        Serial.print("CF=1, PM1.0=");
        Serial.print(pmcf10);
        Serial.println(" ug/m3");
      } else if (count == 7) {
        pmcf25 = 256 * high + c;
        Serial.print("CF=1, PM2.5=");
        Serial.print(pmcf25);
        Serial.println(" ug/m3");
      } else if (count == 9) {
        pmcf100 = 256 * high + c;
        Serial.print("CF=1, PM10=");
        Serial.print(pmcf100);
        Serial.println(" ug/m3");
      }
      count++;
    }

    while (mySerial.available()) mySerial.read();
    Serial.println();
    pre_time5000 = now_time;
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
  getPMvalue();
  sendToMQTT();
}
