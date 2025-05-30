# Tugas Pemrograman B -
Komputasi Numerik - Kelompok 24 :
- Calvin Wirathama Katoroy (2306242395)
- Jonathan Matius Weni Gerimu (2306161896)
- Ekananda Zhafif Dean (2306264420)
  
## Program Analisis Lalu Lintas Jaringan Menggunakan Interpolasi Lagrange & Integrasi Simpson
### Deskripsi Program
Program ini dirancang untuk melakukan analisis lalu lintas jaringan berdasarkan dataset dari Universitas Cincinnati. Program mengolah data dalam bentuk time series bandwidth (Mbps per jam) dan menerapkan Interpolasi Lagrange serta Integrasi Simpson untuk melakukan prediksi dan analisis konsumsi bandwidth harian.
Antarmuka berbasis teks ini memungkinkan pengguna menjalankan berbagai operasi analitik secara interaktif. 

Metode numerik yang digunakan dalam program ini meliputi:

- **Interpolasi Lagrange**, yang berfungsi untuk memperkirakan nilai bandwidth pada waktu tertentu berdasarkan titik data yang telah dikumpulkan sebelumnya.
- **Integrasi Numerik Metode Simpson**, yang digunakan untuk menghitung estimasi total konsumsi bandwidth dalam rentang waktu tertentu.

Bahasa pemrograman : 
- **C++**

### Pemrosesan Data
Program ini bekerja dengan cara memuat data mentah lalu lintas jaringan dari file CSV. Setelah data berhasil dimuat, program dapat menghitung statistik dasar seperti nilai maksimum, minimum, rata-rata bandwidth, serta faktor pemanfaatan puncak. Pengguna dapat melakukan prediksi bandwidth pada waktu tertentu menggunakan interpolasi Lagrange yang menghitung nilai estimasi berdasarkan data yang ada. Selain itu, program juga dapat menghitung total konsumsi bandwidth dalam periode tertentu menggunakan metode integrasi Simpson, yang memberikan gambaran penggunaan selama interval waktu yang diinginkan. Semua proses ini dikemas dalam menu interaktif yang memungkinkan pengguna memilih fungsi yang diinginkan, mulai dari memuat data, melihat statistik, dan menjalankan prediksi. Program ini memberikan alat analisis lengkap untuk memantau dan memprediksi lalu lintas jaringan berdasarkan data historis secara numerik.
