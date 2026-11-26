#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define I2C_SDA 21
#define I2C_SCL 22
#define OLED_ADDR 0x3C
#define SENSOR_ADDR 0x76

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define WAKEUP_GPIO GPIO_NUM_15
#define SLEEP_DURATION_US (10ULL * 1000000ULL) // 10 detik
#define HISTORY_SIZE 5

struct SensorRecord {
  uint32_t bootCount;
  float temperature;
  float humidity;
  float pressure;
  uint8_t wakeupReason;
};

RTC_DATA_ATTR uint32_t rtcBootCount = 0;
RTC_DATA_ATTR uint8_t rtcHistoryIndex = 0;
RTC_DATA_ATTR uint8_t rtcHistoryCount = 0;
RTC_DATA_ATTR SensorRecord rtcHistory[HISTORY_SIZE];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;
Adafruit_BMP280 bmp;

bool isBME280 = false;
bool isBMP280 = false;

SemaphoreHandle_t i2cMutex = nullptr;
SemaphoreHandle_t dataMutex = nullptr;
SemaphoreHandle_t sensorDoneSem = nullptr;
SemaphoreHandle_t readyToSleepSem = nullptr;

float latestTemp = 0.0f;
float latestHum = 0.0f;
float latestPres = 0.0f;

const char* getWakeupReasonText(esp_sleep_wakeup_cause_t reason) {
  switch (reason) {
    case ESP_SLEEP_WAKEUP_TIMER: return "TIMER (10s)";
    case ESP_SLEEP_WAKEUP_EXT0:  return "EXT0 (GPIO15)";
    default:                     return "POWER_ON/RESET";
  }
}

void scanI2CBus() {
  Serial.println("\n[I2C SCANNER] Memindai bus I2C (SDA=21, SCL=22)...");
  int count = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  -> Ditemukan perangkat di 0x%02X", addr);
      if (addr == OLED_ADDR) Serial.print(" (OLED SSD1306)");
      else if (addr == SENSOR_ADDR) Serial.print(" (BME280/BMP280)");
      Serial.println();
      count++;
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  Serial.printf("[I2C SCANNER] Total perangkat aktif: %d\n", count);
}

void printRTCHistory() {
  Serial.println("--- Riwayat Data di RTC_DATA_ATTR ---");
  for (uint8_t i = 0; i < rtcHistoryCount; i++) {
    Serial.printf("  [%u] Boot #%lu | Temp: %.2f C | Hum: %.1f %% | Pres: %.2f hPa | Wake: %s\n",
                  i + 1,
                  (unsigned long)rtcHistory[i].bootCount,
                  rtcHistory[i].temperature,
                  rtcHistory[i].humidity,
                  rtcHistory[i].pressure,
                  getWakeupReasonText((esp_sleep_wakeup_cause_t)rtcHistory[i].wakeupReason));
  }
}

// Task Core 0: Baca sensor & simpan ke RTC_DATA_ATTR
void taskSensor(void *pvParameters) {
  (void)pvParameters;
  float temp = NAN;
  float hum = 0.0f;
  float pres = NAN;

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    if (bme.begin(SENSOR_ADDR)) {
      isBME280 = true;
      temp = bme.readTemperature();
      hum = bme.readHumidity();
      pres = bme.readPressure() / 100.0F;
    } else if (bmp.begin(SENSOR_ADDR)) {
      isBMP280 = true;
      temp = bmp.readTemperature();
      hum = 0.0f;
      pres = bmp.readPressure() / 100.0F;
    }
    xSemaphoreGive(i2cMutex);
  }

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    rtcBootCount++;
    latestTemp = temp;
    latestHum = hum;
    latestPres = pres;

    rtcHistory[rtcHistoryIndex].bootCount = rtcBootCount;
    rtcHistory[rtcHistoryIndex].temperature = temp;
    rtcHistory[rtcHistoryIndex].humidity = hum;
    rtcHistory[rtcHistoryIndex].pressure = pres;
    rtcHistory[rtcHistoryIndex].wakeupReason = (uint8_t)reason;

    rtcHistoryIndex = (rtcHistoryIndex + 1) % HISTORY_SIZE;
    if (rtcHistoryCount < HISTORY_SIZE) {
      rtcHistoryCount++;
    }
    xSemaphoreGive(dataMutex);
  }

  Serial.printf("[Core %d - TaskSensor] Boot #%lu | T=%.2f C | H=%.1f%% | P=%.2f hPa\n",
                xPortGetCoreID(), (unsigned long)rtcBootCount, temp, hum, pres);

  xSemaphoreGive(sensorDoneSem);
  vTaskDelete(nullptr);
}

// Task Core 1: Perbarui OLED
void taskOLED(void *pvParameters) {
  (void)pvParameters;

  xSemaphoreTake(sensorDoneSem, portMAX_DELAY);

  float tempCopy = NAN;
  float humCopy = 0.0f;
  float presCopy = NAN;
  uint32_t bootCopy = 0;
  uint8_t histCountCopy = 0;

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    tempCopy = latestTemp;
    humCopy = latestHum;
    presCopy = latestPres;
    bootCopy = rtcBootCount;
    histCountCopy = rtcHistoryCount;
    xSemaphoreGive(dataMutex);
  }

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.printf("Boot #%lu (Core %d)\n", (unsigned long)bootCopy, xPortGetCoreID());
    display.printf("Wake: %s\n", getWakeupReasonText(reason));
    display.println("---------------------");
    display.printf("Temp: %.2f C\n", tempCopy);
    if (isBME280) {
      display.printf("Hum : %.1f %%\n", humCopy);
    }
    display.printf("Pres: %.2f hPa\n", presCopy);
    display.printf("RTC Log: %u/%u data\n", histCountCopy, HISTORY_SIZE);
    display.display();
    xSemaphoreGive(i2cMutex);
  }

  Serial.printf("[Core %d - TaskOLED] OLED diperbarui.\n", xPortGetCoreID());
  printRTCHistory();

  vTaskDelay(pdMS_TO_TICKS(1500));

  xSemaphoreGive(readyToSleepSem);
  vTaskDelete(nullptr);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  Serial.println("==========================================");
  Serial.printf("ESP32 Bangun! Penyebab: %s\n", getWakeupReasonText(reason));

  Wire.begin(I2C_SDA, I2C_SCL);

  if (reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
    scanI2CBus();
  }

  i2cMutex = xSemaphoreCreateMutex();
  dataMutex = xSemaphoreCreateMutex();
  sensorDoneSem = xSemaphoreCreateBinary();
  readyToSleepSem = xSemaphoreCreateBinary();

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Serial.println("Gagal inisialisasi OLED SSD1306 di 0x3C!");
    }
    xSemaphoreGive(i2cMutex);
  }

  xTaskCreatePinnedToCore(taskSensor, "TaskSensor", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskOLED, "TaskOLED", 4096, nullptr, 2, nullptr, 1);
}

void loop() {
  if (xSemaphoreTake(readyToSleepSem, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      xSemaphoreGive(i2cMutex);
    }

    Serial.println("Memasuki Deep Sleep 10 detik (Timer + EXT0 GPIO 15)...\n");
    Serial.flush();

    rtc_gpio_pullup_dis(WAKEUP_GPIO);
    rtc_gpio_pulldown_en(WAKEUP_GPIO);
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1);
    esp_deep_sleep_start();
  }
}