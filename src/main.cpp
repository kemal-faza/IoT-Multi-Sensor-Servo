#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define DHT_PIN 4
#define MOTION_PIN 5
#define SERVO_PIN 18

#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

Servo myServo;

float temperature;
float humidity;
bool motionDetected = false;
bool alertStatus = false;
unsigned long previousMillis = 0;
const long interval = 2000; // Interval pembacaan sensor (2 detik)
int currentServoPos = 0;

// Thresholds
const float TEMP_THRESHOLD = 30.0;
const float HUMID_THRESHOLD = 70.0;

void controlServo()
{
  int targetPosition = 0;

  if (motionDetected && temperature > TEMP_THRESHOLD) // Suhu abnormal + motion trigger
  {
    targetPosition = 90;
    if (!alertStatus)
    {
      Serial.println("KONDISI KRITIS: Motion + Suhu tinggi! Servo -> 90°");
      alertStatus = true;
    }
  }
  else if (motionDetected || temperature > TEMP_THRESHOLD || humidity > HUMID_THRESHOLD) // Salah satu sensor ketrigger/abnormal
  {
    targetPosition = 45;
    if (!alertStatus)
    {
      Serial.println("PERINGATAN: Kondisi tidak normal! Servo -> 45°");
      alertStatus = true;
    }
  }
  else // Normal
  {
    targetPosition = 0;
    if (alertStatus)
    {
      Serial.println("NORMAL: Kondisi aman. Servo -> 0°");
      alertStatus = false;
    }
  }

  // Gerakkan servo sesuai posisi target
  if (currentServoPos != targetPosition)
  {
    if (currentServoPos < targetPosition)
    {
      currentServoPos += 2;
      if (currentServoPos > targetPosition)
        currentServoPos = targetPosition;
    }
    else
    {
      currentServoPos -= 2;
      if (currentServoPos < targetPosition)
        currentServoPos = targetPosition;
    }

    myServo.write(currentServoPos);
    delay(20);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(MOTION_PIN, INPUT);

  dht.begin();

  myServo.attach(SERVO_PIN);
  myServo.write(0);

  Serial.println("=== Smart Monitoring System ===");
  Serial.println("DHT22 Sensor + Motion Sensor + Servo Alert");
  Serial.println("DHT22 pada pin 4, Motion PIR pada pin 5");
  Serial.println("Aktuator: Servo pada pin 18");
  Serial.println("Threshold: 30°C, 70% RH");
  Serial.println("Servo Positions: 0°=Normal, 45°=Warning, 90°=Critical");
  Serial.println("=====================================");
}

void loop()
{
  unsigned long currentMillis = millis();

  // Baca sensor setiap 2 detik
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    // Baca sensor suhu
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Error: Gagal membaca DHT sensor!");
      return;
    }

    // Baca sensor motion
    motionDetected = digitalRead(MOTION_PIN);

    Serial.println("\n--- Data Sensor ---");
    Serial.printf("Suhu: %.2f°C\n", temperature);
    Serial.printf("Kelembaban: %.2f%%\n", humidity);
    Serial.printf("Motion: %s\n", motionDetected ? "TERDETEKSI" : "Tidak ada gerakan");
  }

  controlServo();

  delay(100);
}