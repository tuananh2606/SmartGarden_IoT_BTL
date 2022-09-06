#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#define BLYNK_PRINT Serial
#define APP_DEBUG
#include <BlynkSimpleEsp8266.h>


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <bits/stdc++.h>
#include <Wire.h>                 //Thư viện giao tiếp I2C
#include <LiquidCrystal_I2C.h>    //Thư viện LCD
LiquidCrystal_I2C lcd(0x27,16,2); //Thiết lập địa chỉ và loại LCD
Servo servo;
byte icon[] = {                   //Tạo ký tự riêng icon
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};


// Thông tin về wifi
char ssid[] = "";
char pass[] = "";

char auth[] = BLYNK_AUTH_TOKEN;
#define mqtt_server "broker.hivemq.com"
const uint16_t mqtt_port = 1883;

bool state = true;
bool state1 = true;
bool stateAuto = true;

#define RELAYPIN D1
BLYNK_WRITE(V5) {
  if (!stateAuto) {
    int control = param.asInt();
    if (control == 1)
    {
      state = false;
      // execute this code if the switch widget is now ON
      digitalWrite(RELAYPIN, HIGH); // Set digital pin D1 HIGH
    }
    else
    {
      state = false;
      // execute this code if the switch widget is now OFF
      digitalWrite(RELAYPIN, LOW); // Set digital pin D1 LOW
    }
  }
}

BLYNK_WRITE(V6) {
  int control1 = param.asInt();
  if (control1 == 0)
  {
    stateAuto = false;
  }
  else
  {
    stateAuto = true;
  }
}
BLYNK_WRITE(V7) {
  if (!stateAuto) {
    int control2 = param.asInt();
    if (control2 == 1)
    {
      state1 = false;
      // execute this code if the switch widget is now ON
      servo.write(180); // Set digital pin D1 HIGH
    }
    else
    {
      state1 = false;
      // execute this code if the switch widget is now OFF
      servo.write(0); // Set digital pin D1 LOW
    }
  }
}

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  pinMode(RELAYPIN, OUTPUT);
  servo.attach(13);
  pinMode(14,OUTPUT);
  Blynk.begin(auth, ssid, pass);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Wire.begin(D4,D3);               //Thiết lập chân kết nối I2C (SDA,SCL);
  lcd.init();                      //Khởi tạo LCD
  lcd.backlight();
  lcd.print("Nhiet do: ");          //Ghi đoạn text "Welcom to"
  lcd.setCursor(0,1);              //Đặt vị trí ở ô thứ 1 trên dòng 2
  lcd.print("Do am dat: ");
  lcd.createChar(1, icon);
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

float humi;
float temp;
float doamdat;
float rain;
float light;

// Hàm call back để nhận dữ liệu
void callback(char* topic, byte* payload, unsigned int length) {
  std::string topicData = "";
  //  Serial.print("Topic: ");
  //  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    topicData += (char)payload[i];
  }
  //  Serial.println(std::stof(topicData));



  if (strcmp(topic, "Humi_IoTnhom3") == 0) {
    humi = std::stof(topicData);
  }
  if (strcmp(topic, "Temp_IoTnhom3") == 0) {
    temp = std::stof(topicData);
  }
  if (strcmp(topic, "DoAmDat_IoTnhom3") == 0) {
    doamdat = std::stof(topicData);
  }
  if (strcmp(topic, "RainOrNot_IoTnhom3") == 0) {
    rain = std::stof(topicData);
  }
  if (strcmp(topic, "LightData_IoTnhom3") == 0) {
    light = std::stof(topicData);
  }
}

// Hàm reconnect thực hiện kết nối lại khi mất kết nối với MQTT Broker
void reconnect() {
  while (!client.connected()) // Chờ tới khi kết nối
  {
    // Thực hiện kết nối với mqtt user và pass
    if (client.connect("ESP8266_id2", "ESP_offline2", 0, 0, "ESP8266_id2_offline")) //kết nối vào broker
    {
      Serial.println("Đã kết nối!!!");
      client.subscribe("Humi_IoTnhom3");
      client.subscribe("Temp_IoTnhom3");
      client.subscribe("DoAmDat_IoTnhom3");
      client.subscribe("RainOrNot_IoTnhom3");
      client.subscribe("LightData_IoTnhom3");
    }
    else
    {
      Serial.print("Lỗi: rc= ");
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
  Serial.println(humi);
  Serial.println(temp);
  Serial.println(doamdat);
  Serial.println(rain);
  Serial.println(light);
  Serial.println("-----------------------------------");
  lcd.display();
  lcd.setCursor(9,0);
  lcd.print(round(temp));
  lcd.write(1);
  lcd.print("C");
  lcd.setCursor(10,1);
  lcd.print(round(doamdat));
  lcd.print("%");
  if (stateAuto == true) {
    if (temp < 30) {
      if (doamdat < 55) {
        relayControl(RELAYPIN, true);
        servo.write(0);
      }
      else {
        if (rain == 0) {
          servo.write(180);
        }
        else {
          servo.write(0);
        }
        relayControl(RELAYPIN, false);
      }
    }

    else {
      if (doamdat < 55 && rain == 0) {
        servo.write(0);
      }
      else {
        servo.write(180);
      }
      relayControl(RELAYPIN, false);
    }

    // Mở đèn
    if (light > 100) {
      led_light(14,false);
    }
    else {
      led_light(14,true);
    }
    }
  }

  void led_light(int led, bool check) {
    if(check)
      digitalWrite(led, HIGH);
    else
      digitalWrite(led, LOW);
  }

  void relayControl(int relay, bool check) {
    if (check == true) {
      digitalWrite(relay, HIGH);
    }
    else {
      digitalWrite(relay, LOW);
    }
  }
  void servoControl(bool check) {
    if (check == true) {
      servo.write(180);
    }
    else {
      servo.write(0);
    }
  }
