#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#define BLYNK_PRINT Serial
#define APP_DEBUG
#include <BlynkSimpleEsp8266.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <BH1750.h>
#include <Wire.h>
#include <string>
#include <bits/stdc++.h>

// Thông tin về wifi
char ssid[] = "";
char pass[] = "";

char auth[] = BLYNK_AUTH_TOKEN;
#define mqtt_server "broker.hivemq.com"
const uint16_t mqtt_port = 1883;

#define DHTTYPE DHT11
#define DHTPIN D5
#define RAINPIN D4

WiFiClient espClient;
PubSubClient client(espClient);

//Cảm biến nhiệt độ, độ ẩm của không khí
DHT dht(DHTPIN, DHTTYPE);
float readHumi() {
  return dht.readHumidity();
}

float readTemp(bool check) {
  if (!check)
    return dht.readTemperature();
  // F
  return dht.readTemperature(true);
}

float computeHeatIndex(bool check) {
  float h = dht.readHumidity();
  delay(10);
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (!check)
    // Compute heat index in Celsius (isFahreheit = false)
    // C
    return dht.computeHeatIndex(t, h, false);
  // F
  return dht.computeHeatIndex(f, h);
}

//Cảm biến ánh sáng
BH1750 lightMeter;
float computeLightSensor() {
  float lux = lightMeter.readLightLevel();
  return lux;
}

//Cảm biến độ ẩm đất
float computeSolidSensor() {
  int value = analogRead(A0);
  int percent = map(value, 350, 1023, 0, 100); // convert to %
  int realpercent = 100 - percent;
  return realpercent;
}

//Cảm biến mưa
int computeRaindropsSensor() {
  int value = digitalRead(RAINPIN);
  return value;
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Wire.begin();
  lightMeter.begin();
  Blynk.begin(auth, ssid, pass);
}

// Hàm kết nối wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Hàm call back để nhận dữ liệu
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++)
    Serial.print((char)payload[i]);
  Serial.println();
}

// Hàm reconnect thực hiện kết nối lại khi mất kết nối với MQTT Broker
void reconnect() {
  while (!client.connected()) // Chờ tới khi kết nối
  {   
    if (client.connect("ESP8266_id1", "ESP_offline1", 0, 0, "ESP8266_id1_offline")) { //kết nối vào broker
      Serial.println("Đã kết nối!!!");
    }
    else
    {
      Serial.print("Lỗi: rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected())// Kiểm tra kết nối
    reconnect();
  client.loop();
  Blynk.run();
  Blynk.virtualWrite(V3, computeLightSensor());
  Blynk.virtualWrite(V1, readHumi());
  Blynk.virtualWrite(V0, readTemp(false));
  Blynk.virtualWrite(V4, computeSolidSensor());
  Blynk.virtualWrite(V2, computeRaindropsSensor());
  
  Serial.println(readHumi());
  Serial.println(readTemp(false));
  delay(2000);
  //  Serial.println("Đưa dữ liệu đo được lên các topic");
  client.publish("Humi_IoTnhom3", std::to_string(readHumi()).c_str());
  client.publish("Temp_IoTnhom3", std::to_string(readTemp(false)).c_str());
  //  client.publish("HeatIndex_IoTnhom3", std::to_string(computeHeatIndex(false)).c_str());
  client.publish("DoAmDat_IoTnhom3", std::to_string(computeSolidSensor()).c_str());
  client.publish("RainOrNot_IoTnhom3", std::to_string(computeRaindropsSensor()).c_str());
  client.publish("LightData_IoTnhom3", std::to_string(computeLightSensor()).c_str());
}
