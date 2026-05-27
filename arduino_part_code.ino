#include "EmonLib.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

EnergyMonitor emon;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float cost = 0;

float tariff = 6.0;   // ₹ per unit

unsigned long lastTime = 0;

void setup()
{
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  emon.voltage(A1, 230.0, 1.0);
  emon.current(A0, 2.0);

  lastTime = millis();
}

void loop()
{
  emon.calcVI(20, 2000);

  voltage = emon.Vrms;
  current = emon.Irms;
  power   = emon.realPower;

  if (isnan(voltage) || isnan(current) || isnan(power))
  {
    return;
  }

  unsigned long now = millis();

  // -------- ENERGY CALCULATION --------
  energy += (power * (now - lastTime)) / 3600000.0;
  lastTime = now;

  // -------- COST --------
  cost = energy * tariff * 100;

  // -------- SERIAL (to ESP32) --------
  Serial.print(voltage, 2);
  Serial.print(",");
  Serial.print(current, 3);
  Serial.print(",");
  Serial.print(power, 2);
  Serial.print(",");
  Serial.print(energy, 3);
  Serial.print(",");
  Serial.println(cost, 2);

  // -------- LCD DISPLAY --------

  // Line 1
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print((int)voltage);
  lcd.print(" I:");
  lcd.print(current,2);

  // Line 2
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Rs:");
  lcd.print(cost,1);   // cost
  lcd.print(" E:");
  lcd.print(energy,2);

  delay(2000);
}