#include <DHTesp.h>
#include <ArduinoJson.h>

// Definisi Pin
const int DHT_PIN = 15;
const int LED_PIN = 2;
const int BUTTON_PIN = 4;

DHTesp dht;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Sensor dan Pin I/O
  dht.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("Sistem IoT ESP32 Siap...");
}

void loop() {
  // Baca status tombol (active low karena menggunakan INPUT_PULLUP)
  bool buttonState = (digitalRead(BUTTON_PIN) == LOW);
  
  // Kontrol LED berdasarkan status tombol
  digitalWrite(LED_PIN, buttonState ? HIGH : LOW);

  // Baca suhu dan kelembapan dari DHT22
  TempAndHumidity data = dht.getTempAndHumidity();

  // Buat objek JSON menggunakan ArduinoJson (kapasitas 200 byte)
  JsonDocument doc;
  doc["temperature"] = data.temperature;
  doc["humidity"]    = data.humidity;
  doc["button"]      = buttonState ? "PRESSED" : "RELEASED";
  doc["status"]      = "OK";

  // Cetak string JSON valid ke Serial Monitor
  serializeJson(doc, Serial);
  Serial.println(); // Baris baru agar format JSON terbaca per baris

  delay(2000); // Kirim data setiap 2 detik
}