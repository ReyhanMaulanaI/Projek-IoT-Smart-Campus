IoT SmartRoom UNIB

Sistem Monitoring dan Pengelolaan Ruangan Cerdas Berbasis Internet of Things

📌 Deskripsi Proyek

IoT SmartRoom UNIB merupakan proyek berbasis Internet of Things (IoT) yang dirancang untuk mendukung penerapan konsep Smart Campus di lingkungan Universitas Bengkulu.

Proyek ini bertujuan untuk melakukan monitoring kondisi ruangan secara real-time dengan memanfaatkan beberapa sensor yang terintegrasi melalui mikrokontroler ESP32. Sistem memungkinkan pengguna memantau aktivitas dan kondisi lingkungan ruangan melalui dashboard secara terpusat.

Data sensor akan dikirimkan ke server menggunakan protokol komunikasi IoT, kemudian disimpan dan ditampilkan dalam bentuk informasi yang mudah dipahami.

🎯 Tujuan Proyek

1. Membangun sistem monitoring ruangan berbasis IoT.
2. Memantau aktivitas dan kondisi lingkungan ruangan secara real-time.
3. Mengintegrasikan beberapa sensor menggunakan ESP32.
4. Menyediakan dashboard untuk visualisasi data sensor.
5. Menerapkan komunikasi MQTT dan HTTP/REST API.
6. Mendukung efisiensi pengelolaan fasilitas dan ruangan kampus.

🔧 Komponen Perangkat Keras

Komponen	Fungsi
ESP32	Mikrokontroler utama untuk membaca sensor dan mengirimkan data melalui Wi-Fi.
Sensor Suara	Mendeteksi aktivitas suara dalam ruangan.
Sensor Ultrasonik	Mendeteksi keberadaan objek berdasarkan jarak.
Sensor Kelembapan	Memantau tingkat kelembapan lingkungan.

⚙️ Teknologi yang Digunakan

* ESP32: Mikrokontroler utama.
* MQTT: Protokol komunikasi data sensor.
* HTTP/REST API: Pertukaran data antara server dan aplikasi.
* OpenAPI 3.1: Dokumentasi REST API.
* Database: Penyimpanan data sensor.
* Docker Compose: Pengelolaan layanan aplikasi.
* Dashboard: Visualisasi data dan monitoring ruangan.
* TLS/mTLS: Pengamanan komunikasi data.
* Authentication: Autentikasi pengguna dan perangkat.

🏗️ Arsitektur Sistem

Alur kerja sistem:

1. Sensor membaca kondisi lingkungan dan aktivitas ruangan.
2. ESP32 menerima dan memproses data sensor.
3. ESP32 mengirimkan data melalui jaringan Wi-Fi menggunakan MQTT.
4. Backend menerima dan mengolah data.
5. Data disimpan ke dalam database.
6. Dashboard mengambil dan menampilkan data melalui REST API.
7. Sistem memberikan notifikasi apabila kondisi tertentu memenuhi aturan peringatan.

📊 Fitur Utama

* Monitoring data sensor secara real-time.
* Pemantauan aktivitas suara.
* Deteksi keberadaan objek.
* Monitoring kelembapan ruangan.
* Visualisasi data melalui dashboard.
* Penyimpanan riwayat data sensor.
* Sistem peringatan atau alert.
* Analisis data untuk mendukung pengelolaan ruangan.

📁 Struktur Repository

IoT-SmartRoom-UNIB/
│
├── backend/
├── frontend/
├── firmware/
│   ├── sensors/
│   └── main/
├── database/
├── docs/
│   └── openapi.yaml
│
├── docker-compose.yml
├── .env.example
├── .gitignore
└── README.md

👥 Anggota Kelompok

No.	Nama	NPM
1	Ammar Siraj Ananda	G1A023021
2	Reyhan Maulana Ibrahim	G1A023046
3	Yovanza Villareal	G1A023054
4	Arrafi Andersont	G1A023090

🚀 Status Pengembangan

Tahapan	Status
Perencanaan sistem	Dalam proses
Perancangan arsitektur	Dalam proses
Integrasi sensor dengan ESP32	Belum dimulai
Implementasi MQTT	Belum dimulai
Pengembangan backend	Belum dimulai
Pengembangan dashboard	Belum dimulai
Integrasi database	Belum dimulai
Implementasi keamanan	Belum dimulai
Pengujian sistem	Belum dimulai
Deployment	Belum dimulai

📌 Dokumentasi dan Kontribusi

Repository ini digunakan sebagai pusat pengembangan, dokumentasi, dan pemantauan progres proyek IoT SmartRoom UNIB.

Setiap anggota kelompok diharapkan:

* Mengunggah kode sesuai bagian pekerjaan masing-masing.
* Menggunakan commit message yang jelas.
* Memperbarui progres pekerjaan secara berkala.
* Mendokumentasikan perubahan dan hasil pengujian.
* Melakukan koordinasi melalui GitHub.

📄 Lisensi

Proyek ini dikembangkan untuk memenuhi tugas proyek mata kuliah Internet of Things (IoT), Program Studi Informatika, Universitas Bengkulu.

⸻

IoT SmartRoom UNIB

Towards a Smarter and More Efficient Campus.
