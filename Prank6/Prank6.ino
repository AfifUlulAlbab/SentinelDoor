#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#define BUZZER 5
#define LED 2

int ldrStatus;
MPU6050 mpu6050(Wire);
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void tone(uint8_t pin, uint32_t frequency, uint32_t duration = 0) {
  ledcSetup(0, frequency, 8);
  ledcAttachPin(pin, 0);
  ledcWriteTone(0, frequency);

  if (duration) {
    delay(duration);
    noTone(pin);
  }
}

void noTone(uint8_t pin) {
  ledcDetachPin(pin);
}

void danger() {
  ledcWrite(ledChannel, 255);
  delay(10);
  ledcWrite(ledChannel, 0);
}

const char ssid[] = "huhuy";            //ganti dengan nama wifi
const char pass[] = "12345678";         //ganti dengan password
const char server[] = "192.168.90.8";  //ganti dengan webserver check di cmd dengan perintah ipconfig pada bagian IPV4
const int port = 80;

WiFiClient wClient;
HTTPClient http;
String url;

bool manualMode = false;
int valueTombol;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LED, ledChannel);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  pinMode(LED, OUTPUT);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("*");
    delay(1000);
  }
}

void loop() {
  float sumbu_y = mpu6050.getAngleY();
  float sumbu_x = mpu6050.getAccAngleX();

  url = "http://" + String(server) + "/pirdasuas/public/api/save/" + String(sumbu_x) + "/" + String(sumbu_y);
  String uri;

  if (!wClient.connect(server, port)) {
    Serial.print("Menghubungkan ke server ...");
    return;
  }

  mpu6050.update();


  if (mpu6050.getAngleY() > 10) {
    Serial.println("awas maling");
    Serial.println(mpu6050.getAngleY());
    tone(BUZZER, 1000, 50);
    danger();
    uri = url + "/Maling";
    http.begin(wClient, uri);
  }

  else if (mpu6050.getAngleY() < -10) {
    Serial.println("awas ya tiati");
    Serial.println(mpu6050.getAngleY());
    tone(BUZZER, 500, 50);
    danger();
    uri = url + "/Maling";
    http.begin(wClient, uri);
  }

  else if (-9 > mpu6050.getAngleY() < 9 && -9 > mpu6050.getAngleX() < 9) {
    Serial.println("aman cuy");
    Serial.println(mpu6050.getAngleY());
    noTone(BUZZER);
    ledcWrite(ledChannel, 0);
    ledcWrite(ledChannel, 0);
    uri = url + "/Aman";
    http.begin(wClient, uri);
  }

  http.GET();
  http.end();
}
