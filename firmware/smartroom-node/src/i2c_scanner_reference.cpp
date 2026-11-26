/*
 * Referensi Program I2C Scanner (Tahap 1 Lab-2)
 * Jalankan kode ini terlebih dahulu jika ingin memverifikasi alamat perangkat I2C
 * sebelum menjalankan kode utama FreeRTOS + Deep Sleep.
 */
#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 21
#define I2C_SCL 22

void runI2CScanner() {
  Serial.println("\n=== I2C Scanner (SDA=21, SCL=22) ===");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.printf("Perangkat I2C terdeteksi di alamat: 0x%02X", addr);
      if (addr == 0x3C) Serial.print(" -> OLED SSD1306");
      if (addr == 0x76 || addr == 0x77) Serial.print(" -> Sensor BME280/BMP280");
      Serial.println();
      found++;
    }
  }
  if (found == 0) {
    Serial.println("Tidak ada perangkat I2C ditemukan!");
  } else {
    Serial.printf("Total %d perangkat I2C ditemukan tanpa tabrakan alamat.\n", found);
  }
}
