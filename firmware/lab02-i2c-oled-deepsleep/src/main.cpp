
#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_BMP085 bmp;

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

float suhu = 0;
float tekanan = 0;

SemaphoreHandle_t sensorMutex;

void TaskSensor(void *pvParameters) {
  while (true) {
    float suhuBaru = bmp.readTemperature();
    float tekananBaru = bmp.readPressure() / 100.0;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      suhu = suhuBaru;
      tekanan = tekananBaru;
      xSemaphoreGive(sensorMutex);
    }

    // Membuat objek JSON
    JsonDocument doc;

    doc["device"] = "Smart Campus UNIB";
    doc["suhu"] = suhuBaru;
    doc["tekanan"] = tekananBaru;

    // Mengubah JSON menjadi teks
    String jsonOutput;
    serializeJson(doc, jsonOutput);

    Serial.println("=== DATA SENSOR JSON ===");
    Serial.println(jsonOutput);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void TaskOLED(void *pvParameters) {
  while (true) {
    float suhuTampil;
    float tekananTampil;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      suhuTampil = suhu;
      tekananTampil = tekanan;
      xSemaphoreGive(sensorMutex);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SMART CAMPUS UNIB");
    display.println("----------------");

    display.setCursor(0, 22);
    display.print("Suhu: ");
    display.print(suhuTampil, 1);
    display.println(" C");

    display.setCursor(0, 42);
    display.print("Tekanan: ");
    display.print(tekananTampil, 1);
    display.println(" hPa");

    display.display();

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED tidak ditemukan!");
    while (true) {
      delay(100);
    }
  }

  if (!bmp.begin()) {
    Serial.println("BMP180 tidak ditemukan!");
    while (true) {
      delay(100);
    }
  }

  Serial.println("BMP180 dan OLED berhasil dimulai!");

  sensorMutex = xSemaphoreCreateMutex();

  if (sensorMutex == NULL) {
    Serial.println("Gagal membuat mutex!");
    while (true) {
      delay(100);
    }
  }

  xTaskCreatePinnedToCore(
    TaskSensor,
    "TaskSensor",
    4096,
    NULL,
    1,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    TaskOLED,
    "TaskOLED",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
