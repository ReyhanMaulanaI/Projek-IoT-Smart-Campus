#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// --- KONFIGURASI WI-FI (WAJIB DIISI) ---
const char* WIFI_SSID     = "Bebas";
const char* WIFI_PASSWORD = "Dio2025r";

// --- DEFINISI PIN NODEMCU ESP8266MOD ---
#define TRIG_OUTER    D1   // GPIO 5  - Ultrasonik 1 (Pintu Luar)
#define ECHO_OUTER    D2   // GPIO 4
#define TRIG_INNER    D5   // GPIO 14 - Ultrasonik 2 (Pintu Dalam)
#define ECHO_INNER    D6   // GPIO 12
#define SOUND_PIN     A0   // ADC0    - Sensor Suara (OUT)
#define LED_LOAD_PIN  D7   // GPIO 13 - LED 1 (Beban Ruangan)
#define LED_ALERT_PIN D0   // GPIO 16 - LED 2 (Peringatan Bising)

// --- PARAMETER KALIBRASI ---
const float DOOR_TRIGGER_CM          = 35.0;
const unsigned long CROSS_TIMEOUT_MS = 2000;
const int NOISE_THRESHOLD_ADC        = 180;
const unsigned long ALERT_HOLD_MS    = 2000;
const unsigned long TELEMETRY_MS     = 1500;

// --- VARIABEL STATUS ---
ESP8266WebServer server(80);
int peopleCount = 0;
int entryState  = 0;
float lastDistOuter = 999.0;
float lastDistInner = 999.0;
int lastNoiseAmp    = 0;
bool lastIsNoisy    = false;

unsigned long stateTimerMillis    = 0;
unsigned long prevTelemetryMillis = 0;
unsigned long noiseHoldUntil      = 0;
int peakNoiseInWindow             = 0;
unsigned long seqCounter          = 0;

// --- HALAMAN WEB DASHBOARD ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>SmartRoom UNIB Monitor</title>
  <style>
    body { font-family: system-ui, -apple-system, sans-serif; background: #0f172a; color: #f8fafc; margin: 0; padding: 20px; }
    .container { max-width: 680px; margin: 0 auto; }
    h1 { font-size: 1.4rem; margin-bottom: 4px; color: #38bdf8; }
    p.sub { color: #94a3b8; font-size: 0.9rem; margin-top: 0; margin-bottom: 20px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 14px; }
    .card { background: #1e293b; border-radius: 12px; padding: 18px; border: 1px solid #334155; }
    .label { font-size: 0.85rem; color: #94a3b8; margin-bottom: 6px; }
    .val { font-size: 2rem; font-weight: 700; }
    .badge { display: inline-block; padding: 4px 10px; border-radius: 999px; font-size: 0.8rem; font-weight: 600; margin-top: 8px; }
    .ok { background: #065f46; color: #6ee7b7; }
    .off { background: #334155; color: #cbd5e1; }
    .warn { background: #991b1b; color: #fca5a5; }
    button { margin-top: 18px; background: #0284c7; color: #fff; border: none; padding: 10px 16px; border-radius: 8px; font-weight: 600; cursor: pointer; }
    button:hover { background: #0369a1; }
  </style>
</head>
<body>
  <div class="container">
    <h1>SmartRoom Monitoring Node</h1>
    <p class="sub">ESP8266MOD Real-Time Telemetry</p>
    <div class="grid">
      <div class="card">
        <div class="label">Jumlah Orang di Ruangan</div>
        <div class="val" id="count">0</div>
        <span class="badge off" id="occ-badge">Kosong (LED D7 MATI)</span>
      </div>
      <div class="card">
        <div class="label">Tingkat Kebisingan (ADC)</div>
        <div class="val" id="noise">0</div>
        <span class="badge ok" id="noise-badge">Tenang (LED D0 MATI)</span>
      </div>
      <div class="card">
        <div class="label">Jarak Pintu Luar / Dalam</div>
        <div class="val" style="font-size:1.3rem;" id="dist">- / - cm</div>
        <span class="badge off" id="seq">Seq: 0</span>
      </div>
    </div>
    <button onclick="resetCount()">Reset Hitungan Orang (0)</button>
  </div>
  <script>
    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        const d = await res.json();
        document.getElementById('count').textContent = d.people_count;
        document.getElementById('noise').textContent = d.noise_amp;
        document.getElementById('dist').textContent = d.dist_outer + ' / ' + d.dist_inner + ' cm';
        document.getElementById('seq').textContent = 'Seq: ' + d.seq;

        const occBadge = document.getElementById('occ-badge');
        occBadge.textContent = d.occupied ? 'Terisi (LED D7 NYALA)' : 'Kosong (LED D7 MATI)';
        occBadge.className = 'badge ' + (d.occupied ? 'ok' : 'off');

        const noiseBadge = document.getElementById('noise-badge');
        noiseBadge.textContent = d.noisy_alert ? 'Bising! (LED D0 NYALA)' : 'Tenang (LED D0 MATI)';
        noiseBadge.className = 'badge ' + (d.noisy_alert ? 'warn' : 'ok');
      } catch (e) {}
    }
    async function resetCount() {
      await fetch('/api/reset');
      fetchStatus();
    }
    setInterval(fetchStatus, 1000);
    fetchStatus();
  </script>
</body>
</html>
)rawliteral";

float readDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999.0;
  return (duration * 0.0343) / 2.0;
}

int readSoundAmplitude() {
  unsigned long startWindow = millis();
  int signalMax = 0;
  int signalMin = 1023;

  while (millis() - startWindow < 30) {
    int sample = analogRead(SOUND_PIN);
    if (sample > signalMax) signalMax = sample;
    if (sample < signalMin) signalMin = sample;
    yield();
  }
  return (signalMax - signalMin);
}

void buildJsonPayload(String &output, int noiseVal) {
  JsonDocument doc;
  doc["device_id"]    = "smartroom-unib-8266";
  doc["seq"]          = seqCounter;
  doc["dist_outer"]   = round(lastDistOuter * 10) / 10.0;
  doc["dist_inner"]   = round(lastDistInner * 10) / 10.0;
  doc["people_count"] = peopleCount;
  doc["occupied"]     = (peopleCount > 0);
  doc["noise_amp"]    = noiseVal;
  doc["noisy_alert"]  = lastIsNoisy;
  doc["load_led_on"]  = (peopleCount > 0);
  serializeJson(doc, output);
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus() {
  String json;
  buildJsonPayload(json, lastNoiseAmp);
  server.send(200, "application/json", json);
}

void handleReset() {
  peopleCount = 0;
  entryState = 0;
  server.send(200, "application/json", "{\"status\":\"reset_ok\"}");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_OUTER, OUTPUT);
  pinMode(ECHO_OUTER, INPUT);
  pinMode(TRIG_INNER, OUTPUT);
  pinMode(ECHO_INNER, INPUT);

  pinMode(LED_LOAD_PIN, OUTPUT);
  pinMode(LED_ALERT_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_LOAD_PIN, LOW);
  digitalWrite(LED_ALERT_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\nMenghubungkan ke Wi-Fi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n{\"event\":\"wifi_connected\",\"ip\":\"%s\"}\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n{\"event\":\"wifi_timeout\",\"mode\":\"offline_serial\"}");
  }

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/reset", handleReset);
  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long now = millis();

  // 1. Baca Jarak Kedua Sensor Ultrasonik
  lastDistOuter = readDistanceCm(TRIG_OUTER, ECHO_OUTER);
  delay(15);
  lastDistInner = readDistanceCm(TRIG_INNER, ECHO_INNER);

  bool outerTriggered = (lastDistOuter < DOOR_TRIGGER_CM);
  bool innerTriggered = (lastDistInner < DOOR_TRIGGER_CM);

  // 2. Logika Penghitung Orang Masuk & Keluar
  if (entryState == 0) {
    if (outerTriggered && !innerTriggered) {
      entryState = 1;
      stateTimerMillis = now;
    } else if (innerTriggered && !outerTriggered) {
      entryState = 2;
      stateTimerMillis = now;
    }
  } else if (entryState == 1) {
    if (innerTriggered) {
      peopleCount++;
      entryState = 0;
      Serial.printf("{\"event\":\"person_enter\",\"people_count\":%d}\n", peopleCount);
      delay(400);
    } else if (now - stateTimerMillis > CROSS_TIMEOUT_MS) {
      entryState = 0;
    }
  } else if (entryState == 2) {
    if (outerTriggered) {
      if (peopleCount > 0) peopleCount--;
      entryState = 0;
      Serial.printf("{\"event\":\"person_exit\",\"people_count\":%d}\n", peopleCount);
      delay(400);
    } else if (now - stateTimerMillis > CROSS_TIMEOUT_MS) {
      entryState = 0;
    }
  }

  // 3. Baca Tingkat Kebisingan & Tahan Peringatan 2 Detik
  int noiseLevel = readSoundAmplitude();
  if (noiseLevel > peakNoiseInWindow) {
    peakNoiseInWindow = noiseLevel;
  }
  if (noiseLevel > NOISE_THRESHOLD_ADC) {
    noiseHoldUntil = now + ALERT_HOLD_MS;
  }
  lastIsNoisy = (now < noiseHoldUntil);
  lastNoiseAmp = lastIsNoisy ? max(noiseLevel, peakNoiseInWindow) : noiseLevel;

  // 4. Kendali LED Otomatis
  bool roomOccupied = (peopleCount > 0);
  digitalWrite(LED_LOAD_PIN, roomOccupied ? HIGH : LOW);
  digitalWrite(LED_BUILTIN, roomOccupied ? LOW : HIGH);
  digitalWrite(LED_ALERT_PIN, lastIsNoisy ? HIGH : LOW);

  // 5. Kirim Telemetri JSON ke Serial
  if (now - prevTelemetryMillis >= TELEMETRY_MS) {
    prevTelemetryMillis = now;
    seqCounter++;

    String jsonOut;
    buildJsonPayload(jsonOut, peakNoiseInWindow);
    Serial.println(jsonOut);
    peakNoiseInWindow = 0;
  }
}