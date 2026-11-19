#include <Arduino.h>
#include <DHTesp.h>
#include <ArduinoJson.h>

// Ubah ke 0 untuk Uji Blocking (delay 2000 ms), ubah ke 1 untuk Uji Non-Blocking (millis)
#define MODE_NON_BLOCKING 1

// Definisi Pin (sesuai skema rangkaianmu)
const int DHT_PIN = 15;
const int LED_PIN = 2;
const int BUTTON_PIN = 4;

DHTesp dht;

unsigned long prevSendMillis = 0;
unsigned long sendInterval = 2000; // Default 2000 ms, toggle ke 500 ms saat tombol ditekan
bool fastMode = false;
bool lastButtonPressed = false;
int buttonPressCount = 0;

void sendTelemetry(float temp, float hum, bool buttonPressed) {
  JsonDocument doc;
  doc["device_id"] = "esp32-unib-01";
  doc["mode"] = MODE_NON_BLOCKING ? "NON_BLOCKING" : "BLOCKING";
  doc["interval_ms"] = sendInterval;
  doc["temperature"] = isnan(temp) ? 0.0 : round(temp * 10) / 10.0;
  doc["humidity"] = isnan(hum) ? 0.0 : round(hum * 10) / 10.0;
  doc["button"] = buttonPressed ? "PRESSED" : "RELEASED";
  doc["press_count"] = buttonPressCount;
  doc["status"] = "OK";

  serializeJson(doc, Serial);
  Serial.println();
}

void handleButton() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  digitalWrite(LED_PIN, buttonPressed ? HIGH : LOW);

  // Deteksi transisi tombol baru ditekan (rising edge dari state lepas ke tekan)
  if (buttonPressed && !lastButtonPressed) {
    buttonPressCount++;
    fastMode = !fastMode;
    sendInterval = fastMode ? 500 : 2000;

    Serial.print(">> [EVENT] Tombol ditekan (#");
    Serial.print(buttonPressCount);
    Serial.print(") | Interval kirim: ");
    Serial.print(sendInterval);
    Serial.println(" ms");
  }

  lastButtonPressed = buttonPressed;
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi Sensor dan Pin I/O
  dht.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("{\"status\":\"Sistem IoT ESP32 Siap...\"}");
}

void loop() {
#if MODE_NON_BLOCKING == 0
  // UJI 1: BLOCKING (delay menghentikan loop selama sendInterval)
  handleButton();
  bool currentBtn = (digitalRead(BUTTON_PIN) == LOW);
  TempAndHumidity data = dht.getTempAndHumidity();
  sendTelemetry(data.temperature, data.humidity, currentBtn);
  delay(sendInterval);

#else
  // UJI 2: NON-BLOCKING (loop tetap berjalan cepat tanpa memblokir pembacaan tombol)
  handleButton();
  unsigned long now = millis();
  if (now - prevSendMillis >= sendInterval) {
    prevSendMillis = now;
    bool currentBtn = (digitalRead(BUTTON_PIN) == LOW);
    TempAndHumidity data = dht.getTempAndHumidity();
    sendTelemetry(data.temperature, data.humidity, currentBtn);
  }
#endif
}