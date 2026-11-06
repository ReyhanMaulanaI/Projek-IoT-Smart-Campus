// Konfigurasi Pin
const int LED_HIJAU = 25;
const int LED_BIRU = 26;
const int LED_MERAH = 27;

const int TRIG_LUAR = 5;
const int ECHO_LUAR = 18;
const int TRIG_DALAM = 19;
const int ECHO_DALAM = 21;
const int PIN_SUARA = 34; // Pin untuk simulasi sensor suara (Potensiometer)

// Variabel Sistem
int jumlahOrang = 0;
const int BATAS_JARAK = 50; // Jarak deteksi (cm)
int statusLangkah = 0;      

void setup() {
  Serial.begin(115200);

  // Inisialisasi Pin
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_BIRU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  
  pinMode(TRIG_LUAR, OUTPUT);
  pinMode(ECHO_LUAR, INPUT);
  pinMode(TRIG_DALAM, OUTPUT);
  pinMode(ECHO_DALAM, INPUT);
  
  // Karena potensiometer adalah sensor analog, tidak wajib dideklarasikan, tapi disarankan
  pinMode(PIN_SUARA, INPUT); 

  Serial.println("Sistem Smartroom Aktif!");
  Serial.println("Membaca data sensor...");
}

void loop() {
  // 1. Membaca nilai dari Potensiometer (Simulasi Suara)
  int nilaiKebisingan = analogRead(PIN_SUARA);

  // 2. Membaca jarak dari kedua sensor ultrasonik
  int jarakLuar = ukurJarak(TRIG_LUAR, ECHO_LUAR);
  int jarakDalam = ukurJarak(TRIG_DALAM, ECHO_DALAM);

  // 3. Menampilkan data secara real-time ke Serial Monitor
  Serial.print("Jarak Luar: "); Serial.print(jarakLuar); Serial.print(" cm | ");
  Serial.print("Jarak Dalam: "); Serial.print(jarakDalam); Serial.print(" cm | ");
  Serial.print("Kebisingan: "); Serial.println(nilaiKebisingan);

  // --- LOGIKA SENSOR SUARA ---
  // Jika nilai potensiometer diputar melewati batas (misal 2500)
  if (nilaiKebisingan > 2500) { 
    Serial.println("Peringatan: KEBISINGAN TINGGI!");
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_BIRU, LOW);
    delay(300);
    digitalWrite(LED_MERAH, LOW);
    delay(300);
    return; // Lewati logika di bawahnya sejenak sampai kondisi tenang
  } else {
    digitalWrite(LED_MERAH, LOW);
  }

  // --- LOGIKA PENGHITUNG ORANG (SENSOR ULTRASONIK) ---
  // Seseorang masuk: Terdeteksi di Luar dulu, lalu Dalam
  if (jarakLuar < BATAS_JARAK && statusLangkah == 0) {
    statusLangkah = 1; 
  } else if (jarakDalam < BATAS_JARAK && statusLangkah == 1) {
    jumlahOrang++;
    statusLangkah = 0; 
    Serial.print(">>> ORANG MASUK! Total di ruangan: "); Serial.println(jumlahOrang);
    delay(1000); 
  }

  // Seseorang keluar: Terdeteksi di Dalam dulu, lalu Luar
  if (jarakDalam < BATAS_JARAK && statusLangkah == 0) {
    statusLangkah = 2;
  } else if (jarakLuar < BATAS_JARAK && statusLangkah == 2) {
    if (jumlahOrang > 0) jumlahOrang--;
    statusLangkah = 0; 
    Serial.print("<<< ORANG KELUAR! Total di ruangan: "); Serial.println(jumlahOrang);
    delay(1000); 
  }

  // Reset jika orang cuma lewat di depan pintu dan tidak masuk/keluar
  if (jarakLuar >= BATAS_JARAK && jarakDalam >= BATAS_JARAK) {
    delay(50); 
  }

  // --- LOGIKA INDIKATOR RUANGAN ---
  if (jumlahOrang == 0) {
    digitalWrite(LED_HIJAU, HIGH); // Ruangan kosong
    digitalWrite(LED_BIRU, LOW);
  } else {
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_BIRU, HIGH);  // Ruangan terisi
  }

  delay(200); // Jeda pendek agar Serial Monitor tidak terlalu cepat bergeser
}

// Fungsi Bantuan untuk membaca jarak Ultrasonik
int ukurJarak(int pinTrig, int pinEcho) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  
  long durasi = pulseIn(pinEcho, HIGH);
  int jarak = durasi * 0.034 / 2;
  
  if (jarak == 0 || jarak > 400) return 400; // Kembalikan nilai maksimal jika tidak ada objek
  return jarak;
}