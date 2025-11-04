#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>

// Pin definitions
#define DHT_PIN 4    // Pin untuk sensor DHT22
#define MOTION_PIN 5 // Pin untuk sensor motion PIR
#define SERVO_PIN 18 // Pin untuk servo motor
#define LED_PIN 2    // Pin untuk LED indikator

// Sensor configuration
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Servo configuration
Servo myServo;

// Variables
float temperature;
float humidity;
bool motionDetected = false;
bool alertStatus = false;
unsigned long previousMillis = 0;
const long interval = 2000;
int currentServoPos = 90; // Posisi awal servo (tengah)

// Thresholds
const float TEMP_THRESHOLD = 30.0;  // Suhu batas dalam Celsius
const float HUMID_THRESHOLD = 70.0; // Kelembaban batas dalam persen

void controlServo()
{
  int targetPosition = 90; // Posisi default (tengah)

  // Tentukan posisi servo berdasarkan kondisi sensor (hanya 3 posisi: 0°, 45°, 90°)
  if (motionDetected && temperature > TEMP_THRESHOLD)
  {
    // Kondisi KRITIS: Motion + Suhu tinggi -> 0 derajat
    targetPosition = 0;
    digitalWrite(LED_PIN, HIGH);
    if (!alertStatus)
    {
      Serial.println("KONDISI KRITIS: Motion + Suhu tinggi! Servo -> 0°");
      alertStatus = true;
    }
  }
  else if (motionDetected || temperature > TEMP_THRESHOLD || humidity > HUMID_THRESHOLD)
  {
    // Kondisi PERINGATAN: Salah satu sensor aktif -> 45 derajat
    targetPosition = 45;
    digitalWrite(LED_PIN, HIGH);
    if (!alertStatus)
    {
      Serial.println("PERINGATAN: Kondisi tidak normal! Servo -> 45°");
      alertStatus = true;
    }
  }
  else
  {
    // Kondisi NORMAL: Semua sensor aman -> 90 derajat
    targetPosition = 90;
    digitalWrite(LED_PIN, LOW);
    if (alertStatus)
    {
      Serial.println("NORMAL: Kondisi aman. Servo -> 90°");
      alertStatus = false;
    }
  }

  // Gerakkan servo secara smooth ke posisi target
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
    delay(20); // Delay untuk gerakan yang smooth
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize pins
  pinMode(MOTION_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initialize sensors
  dht.begin();

  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(90); // Posisi awal di tengah

  // Initial state
  digitalWrite(LED_PIN, LOW);

  Serial.println("=== Smart Monitoring System ===");
  Serial.println("DHT22 Sensor + Motion Sensor + Servo Alert");
  Serial.println("DHT22 pada pin 4, Motion PIR pada pin 5");
  Serial.println("Aktuator: LED pada pin 2, Servo pada pin 18");
  Serial.println("Threshold: 30°C, 70% RH");
  Serial.println("Servo Positions: 90°=Normal, 45°=Warning, 0°=Critical");
  Serial.println("=====================================");
}

void loop()
{
  unsigned long currentMillis = millis();

  // Baca sensor setiap 2 detik
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    // Baca DHT22 sensor
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // Validasi pembacaan DHT
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Error: Gagal membaca DHT sensor!");
      return;
    }

    // Baca motion sensor
    motionDetected = digitalRead(MOTION_PIN);

    // Tampilkan data sensor
    Serial.println("\n--- Data Sensor ---");
    Serial.printf("Suhu: %.2f°C\n", temperature);
    Serial.printf("Kelembaban: %.2f%%\n", humidity);
    Serial.printf("Motion: %s\n", motionDetected ? "TERDETEKSI" : "Tidak ada gerakan");
  }

  // Kontrol servo berdasarkan sensor
  controlServo();

  delay(100); // Delay kecil untuk stabilitas
}