IoT SmartRoom UNIB

Sistem Penghitung Jumlah Orang di Ruang Kelas Berbasis Internet of Things (IoT)

📌 Deskripsi Proyek

IoT SmartRoom UNIB merupakan proyek berbasis Internet of Things (IoT) yang dirancang untuk membantu memantau dan menghitung jumlah orang yang berada di dalam ruang kelas secara real-time.

Sistem ini menggunakan mikrokontroler ESP32 yang terintegrasi dengan sensor ultrasonik dan sensor suara. Sensor ultrasonik digunakan untuk mendeteksi keberadaan serta pergerakan orang pada area pintu masuk kelas, sedangkan sensor suara digunakan untuk mendeteksi aktivitas suara sebagai informasi pendukung kondisi ruangan.

Data hasil pembacaan sensor akan diproses oleh ESP32 dan dikirimkan ke server melalui jaringan Wi-Fi. Selanjutnya, data ditampilkan melalui dashboard sehingga jumlah orang di dalam kelas dapat dipantau secara langsung.

Proyek ini diharapkan dapat mendukung penerapan konsep Smart Campus di lingkungan Universitas Bengkulu.

🎯 Tujuan Proyek

1. Membuat sistem penghitung jumlah orang di dalam ruang kelas secara otomatis.
2. Memanfaatkan ESP32 sebagai mikrokontroler utama.
3. Mengintegrasikan sensor ultrasonik dan sensor suara.
4. Menampilkan jumlah orang di kelas secara real-time.
5. Menyediakan informasi aktivitas suara di dalam ruangan.
6. Mendukung efisiensi pemantauan dan pengelolaan ruang kelas.

🔧 Komponen Perangkat Keras

Komponen	Fungsi
ESP32	Mengolah data sensor dan mengirimkan data melalui Wi-Fi.
Sensor Ultrasonik	Mendeteksi orang yang melewati area pintu masuk atau keluar kelas.
Sensor Suara	Mendeteksi aktivitas suara di dalam kelas sebagai informasi pendukung.

⚙️ Teknologi yang Digunakan

* ESP32: Mikrokontroler utama.
* MQTT: Komunikasi pengiriman data sensor.
* HTTP/REST API: Pertukaran data antara server dan dashboard.
* OpenAPI 3.1: Dokumentasi REST API.
* Database: Penyimpanan data jumlah orang dan aktivitas sensor.
* Docker Compose: Pengelolaan layanan aplikasi.
* Dashboard: Visualisasi jumlah orang secara real-time.
* TLS/mTLS: Pengamanan komunikasi data.
* Authentication: Autentikasi pengguna dan perangkat.

🏗️ Alur Kerja Sistem

1. Sensor ultrasonik mendeteksi orang yang melewati pintu kelas.
2. ESP32 memproses hasil pembacaan sensor untuk memperbarui jumlah orang.
3. Sensor suara mendeteksi aktivitas suara di dalam kelas.
4. ESP32 mengirimkan data sensor ke server menggunakan MQTT.
5. Backend menerima dan mengolah data.
6. Data disimpan ke dalam database.
7. Dashboard menampilkan jumlah orang dan informasi aktivitas suara secara real-time.

📊 Fitur Utama

* Penghitungan jumlah orang di dalam kelas.
* Monitoring jumlah orang secara real-time.
* Deteksi aktivitas suara di dalam kelas.
* Visualisasi data melalui dashboard.
* Penyimpanan riwayat jumlah orang.
* Monitoring kondisi ruangan.
* Sistem peringatan berdasarkan batas kapasitas kelas.

📁 Struktur Repository

IoT-SmartRoom-UNIB/
│
├── backend/
│   ├── src/
│   ├── routes/
│   └── controllers/
│
├── frontend/
│   ├── src/
│   └── components/
│
├── firmware/
│   ├── ultrasonic/
│   ├── sound/
│   └── main/
│
├── database/
│
├── docs/
│   ├── architecture/
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
Integrasi ESP32 dan sensor	Belum dimulai
Implementasi penghitung orang	Belum dimulai
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
