# Laporan Praktikum Lab-0: Ekosistem & Pemrograman Dasar ESP32
- **Nama / NPM:** Reyhan Maulana Ibrahim / [G1A)@#)$^]
- **Kelompok / Topik:** IoT SmartRoom UNIB
- **Repositori:** https://github.com/ReyhanMaulanaI/Projek-IoT-Smart-Campus

## 1. Tabel Pengamatan 3.1: Komparasi Blocking vs Non-Blocking

| Parameter Pengamatan | Mode Blocking (`delay(2000)`) | Mode Non-Blocking (`millis()`) |
| :--- | :--- | :--- |
| Rata-rata jeda respons tombol | ~500 – 2000 ms (tertahan oleh `delay`) | < 10 ms (respons instan) |
| Deteksi tekanan cepat beruntun | Sebagian besar terlewat saat jeda `delay` | 100% terdeteksi (`#18` s/d `#23` tertangkap penuh) |
| Pergantian interval (`2000 ms` $\leftrightarrow$ `500 ms`) | Tertunda hingga siklus `delay` selesai | Berubah seketika saat tombol ditekan |
| Contoh Output JSON | `{"device_id":"esp32-unib-01","mode":"BLOCKING","interval_ms":2000,"temperature":24,"humidity":40,"button":"PRESSED","press_count":2,"status":"OK"}` | `{"device_id":"esp32-unib-01","mode":"NON_BLOCKING","interval_ms":500,"temperature":24,"humidity":40,"button":"RELEASED","press_count":23,"status":"OK"}` |

## 2. Jawaban Pertanyaan Analisis (C1–C6)

1. **[C1] Fungsi Berkas Konfigurasi:**
   - `platformio.ini`: Mengatur target *board* (`esp32doit-devkit-v1`), *framework* Arduino, kecepatan Serial Monitor (`115200`), dan pustaka eksternal (`DHTesp`, `ArduinoJson`).
   - `wokwi.toml`: Menghubungkan simulator Wokwi dengan hasil kompilasi firmware `.bin` dan `.elf` dari PlatformIO.
   - `diagram.json`: Mendefinisikan komponen virtual serta peta jalur kabel antarperangkat pada kanvas simulasi Wokwi.

2. **[C2] Alasan `.pio/` dan `.env` Masuk `.gitignore`:**
   - `.pio/` berisi ratusan file hasil kompilasi biner sementara berukuran besar yang dapat dibangun ulang secara otomatis oleh PlatformIO.
   - `.env` menyimpan kredensial sensitif (kata sandi Wi-Fi, token API, kunci *broker* MQTT) yang berbahaya jika terekspos di repositori publik.

3. **[C3] Kalkulasi Ukuran Payload & Kuota Bulanan:**
   - Panjang satu baris JSON hasil uji (`131 byte` + `2 byte` `\r\n`) = **133 byte**.
   - Pengiriman tiap 2 detik selama 30 hari: $(30 \times 24 \times 3600) / 2 = 1.296.000\text{ pesan}$.
   - Total kuota data: $1.296.000 \times 133\text{ byte} = 172.368.000\text{ byte} \approx \mathbf{164,38\text{ MB/bulan}}$.

4. **[C4] Penyebab Tekanan Tombol Terlewat pada Mode Blocking:**
   - Fungsi `delay()` menghentikan seluruh eksekusi CPU selama durasi jeda tersebut. Selama CPU berhenti, perintah `digitalRead(BUTTON_PIN)` tidak dijalankan sehingga perubahan sinyal tombol yang singkat terabaikan.

5. **[C5] Perbandingan `ArduinoJson` vs `Serial.printf()` Manual:**
   - `ArduinoJson` menjamin sintaks JSON selalu valid dan terstruktur meskipun jumlah kunci bertambah, dengan konsekuensi penggunaan memori SRAM/Flash yang sedikit lebih besar.
   - `Serial.printf()` lebih hemat memori, tetapi rawan kesalahan tanda baca (kurang kutip/koma) dan berisiko memicu *buffer overflow*.

6. **[C6] Modifikasi Kode Toggle Interval (2000 ms $\leftrightarrow$ 500 ms):**
   - Menggunakan deteksi transisi tombol (`buttonPressed && !lastButtonPressed`) untuk membalik variabel status `fastMode = !fastMode` dan memperbarui `sendInterval = fastMode ? 500 : 2000;`.