#define BLYNK_TEMPLATE_ID "TMPL3-jIlndL6"
#define BLYNK_TEMPLATE_NAME "Smart IoT Energy Meter"
#define BLYNK_AUTH_TOKEN "wJiDjGjGtRU1f0zdiUsmniJ_l9UbLvzY"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi
char ssid[] = "Enjoy";
char pass[] = "Akash200";
// Telegram
const char* botToken = "8522252194:AAGWBCAbgF-RNA-xP8SOjq1iJKhL3Q5gMO8";
const char* chatID   = "7315998097";

// Variables
float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float cost = 0;

float tariff = 6.0;

unsigned long lastTime = 0;
BlynkTimer timer;

// -------- TELEGRAM (POST METHOD FIX) --------
void sendTelegram(String msg)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    String url = "https://api.telegram.org/bot" + String(botToken) + "/sendMessage";

    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "chat_id=" + String(chatID) + "&text=" + msg;

    int httpCode = http.POST(postData);

    Serial.print("HTTP Response: ");
    Serial.println(httpCode);

    http.end();
  }
}

// -------- SEND EVERY 1 MIN --------
void sendUpdate()
{
  if (power <= 0) return;

  String msg = "Electricity Update\n";
  msg += "V: " + String(voltage,1) + " V\n";
  msg += "I: " + String(current,2) + " A\n";
  msg += "P: " + String(power,1) + " W\n";
  msg += "E: " + String(energy,3) + " kWh\n";
  msg += "Cost: Rs " + String(cost,2);

  sendTelegram(msg);
}

// -------- READ SERIAL --------
void readSerialData()
{
  if (Serial2.available())
  {
    String data = Serial2.readStringUntil('\n');

    int i1 = data.indexOf(',');
    int i2 = data.indexOf(',', i1 + 1);

    if (i1 == -1 || i2 == -1) return;

    voltage = data.substring(0, i1).toFloat();
    current = data.substring(i1 + 1, i2).toFloat();
    power   = data.substring(i2 + 1).toFloat();

    unsigned long now = millis();

    if (power > 0)
    {
      energy += (power * (now - lastTime)) / 36000000.0;
    }

    lastTime = now;
    cost = energy * tariff*1000;

    // -------- BLYNK --------
    Blynk.virtualWrite(V0, voltage);
    Blynk.virtualWrite(V1, current);
    Blynk.virtualWrite(V2, power);
    Blynk.virtualWrite(V3, energy);
    Blynk.virtualWrite(V4, cost);

    Serial.println(data);
  }
}

// -------- LCD --------
void displayLCD()
{
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print((int)voltage);
  lcd.print(" I:");
  lcd.print(current,2);

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Rs:");
  lcd.print(cost,1);
  lcd.print(" P:");
  lcd.print((int)power);
}

// -------- SETUP --------
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX=16

  lcd.init();
  
  lcd.clear();

  Serial.println("ESP32 Started");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lastTime = millis();

  timer.setInterval(2000L, readSerialData);
  timer.setInterval(2000L, displayLCD);
  timer.setInterval(60000L, sendUpdate);   // ⭐ every 1 min
}

// -------- LOOP --------
void loop()
{
  Blynk.run();
  timer.run();
}