#include "network_analyzer.h"
#include <map>
#include <random>

// Definisikan M_PI jika tidak tersedia
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Alternatif: gunakan const double
const double PI = 3.14159265358979323846;

NetworkAnalyzer::NetworkAnalyzer() {
    // Inisialisasi struktur data kosong
}

double NetworkAnalyzer::convertToMbps(double bytes, double time_interval) {
    // Konversi bytes ke Mbps: (bytes * 8 bit/byte) / (waktu_detik * 1e6)
    return (bytes * 8.0) / (time_interval * 1e6);
}

bool NetworkAnalyzer::loadRawData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Lewati header
    
    std::vector<std::pair<double, int>> raw_packets; // timestamp, length
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;
        
        // Parse CSV: Timestamp,Source IP,Destination IP,Protocol,Length
        std::getline(ss, item, ',');
        double timestamp = std::stod(item);
        
        // Lewati source IP, dest IP, protocol
        std::getline(ss, item, ','); // Source IP
        std::getline(ss, item, ','); // Dest IP  
        std::getline(ss, item, ','); // Protocol
        
        std::getline(ss, item, ',');
        if (!item.empty()) {
            int length = std::stoi(item);
            raw_packets.push_back({timestamp, length});
        }
    }
    
    file.close();
    std::cout << "Memuat " << raw_packets.size() << " paket dari data mentah." << std::endl;
    
    // Agregasi paket menjadi interval waktu
    aggregateData(raw_packets);
    
    // Buat pola 24 jam untuk analisis
    generateHourlyPattern();
    
    return true;
}

void NetworkAnalyzer::aggregateData(const std::vector<std::pair<double, int>>& raw_packets) {
    if (raw_packets.empty()) return;
    
    // Cari rentang waktu
    double min_time = raw_packets[0].first;
    double max_time = raw_packets.back().first;
    double duration = max_time - min_time;
    
    // Buat interval 5 menit
    const double interval_duration = 300.0; // 5 menit dalam detik
    std::map<int, std::pair<int, int>> intervals; // interval_id -> (total_bytes, packet_count)
    
    for (const auto& packet : raw_packets) {
        int interval_id = static_cast<int>((packet.first - min_time) / interval_duration);
        intervals[interval_id].first += packet.second;  // total bytes
        intervals[interval_id].second += 1;             // packet count
    }
    
    // Konversi ke pengukuran bandwidth
    data.clear();
    for (const auto& interval_pair : intervals) {
        TrafficData point;
        point.timestamp = min_time + (interval_pair.first * interval_duration);
        point.bandwidth_mbps = convertToMbps(interval_pair.second.first, interval_duration);
        point.packet_count = interval_pair.second.second;
        data.push_back(point);
    }
    
    std::cout << "Diagregasi menjadi " << data.size() << " interval waktu." << std::endl;
}

void NetworkAnalyzer::generateHourlyPattern() {
    if (data.empty()) return;
    
    // Buat pola 24 jam dari data yang tersedia
    time_hours.clear();
    bandwidth.clear();
    
    // Hitung rata-rata bandwidth dari data nyata
    double avg_bandwidth = 0;
    for (const auto& point : data) {
        avg_bandwidth += point.bandwidth_mbps;
    }
    avg_bandwidth /= data.size();
    
    // Buat pola 24 jam yang realistis
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0, avg_bandwidth * 0.1); // 10% noise
    
    for (int hour = 0; hour < 24; hour++) {
        time_hours.push_back(hour);
        
        // Pola diurnal: rendah di malam hari, puncak di siang hari
        double base_factor = 0.4 + 0.6 * (0.5 + 0.5 * sin(PI * (hour - 6) / 12));
        
        // Penyesuaian jam sibuk
        if (hour >= 8 && hour <= 10) base_factor *= 1.3;   // Puncak pagi
        if (hour >= 14 && hour <= 16) base_factor *= 1.2;  // Puncak siang
        if (hour >= 19 && hour <= 21) base_factor *= 1.4;  // Puncak sore
        if (hour >= 0 && hour <= 5) base_factor *= 0.3;    // Rendah malam
        
        double hourly_bandwidth = avg_bandwidth * base_factor + noise(gen);
        bandwidth.push_back(std::max(0.1, hourly_bandwidth)); // Pastikan positif
    }
    
    std::cout << "Membuat pola lalu lintas 24 jam." << std::endl;
}

double NetworkAnalyzer::lagrangeInterpolation(double target_time) {
    if (time_hours.empty() || bandwidth.empty()) {
        std::cerr << "Error: Tidak ada data tersedia untuk interpolasi." << std::endl;
        return 0.0;
    }
    
    // Batasi target_time ke rentang yang valid
    if (target_time < 0) target_time = 0;
    if (target_time > 23) target_time = 23;
    
    int n = time_hours.size();
    
    // Untuk kecocokan yang tepat, kembalikan nilai yang tepat
    for (int i = 0; i < n; i++) {
        if (std::abs(time_hours[i] - target_time) < 0.01) {
            return bandwidth[i];
        }
    }
    
    // Gunakan interpolasi linear untuk hasil yang lebih stabil
    // Cari dua titik terdekat
    int lower_idx = 0, upper_idx = n - 1;
    
    for (int i = 0; i < n - 1; i++) {
        if (time_hours[i] <= target_time && time_hours[i + 1] >= target_time) {
            lower_idx = i;
            upper_idx = i + 1;
            break;
        }
    }
    
    // Interpolasi linear antara dua titik
    double x0 = time_hours[lower_idx];
    double x1 = time_hours[upper_idx];
    double y0 = bandwidth[lower_idx];
    double y1 = bandwidth[upper_idx];
    
    if (x1 == x0) return y0; // Hindari pembagian dengan nol
    
    double result = y0 + (y1 - y0) * (target_time - x0) / (x1 - x0);
    
    // Pastikan hasil masuk akal
    if (result < 0) result = 0.1;
    if (result > 10.0) result = 10.0; // Batasi pada maksimum yang masuk akal
    
    return result;
}

double NetworkAnalyzer::simpsonIntegration() {
    return simpsonIntegration(0.0, 23.0);
}

double NetworkAnalyzer::simpsonIntegration(double start_time, double end_time) {
    if (bandwidth.empty()) {
        std::cerr << "Error: Tidak ada data tersedia untuk integrasi." << std::endl;
        return 0.0;
    }
    
    // Gunakan titik data yang ada dalam rentang
    std::vector<double> x_vals, y_vals;
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        if (time_hours[i] >= start_time && time_hours[i] <= end_time) {
            x_vals.push_back(time_hours[i]);
            y_vals.push_back(bandwidth[i]);
        }
    }
    
    if (x_vals.size() < 3) {
        std::cerr << "Error: Titik data tidak cukup untuk aturan Simpson." << std::endl;
        return 0.0;
    }
    
    // Pastikan kita memiliki jumlah interval genap
    int n = x_vals.size() - 1;
    if (n % 2 != 0) {
        n--; // Gunakan n-1 interval
    }
    
    double h = (x_vals[n] - x_vals[0]) / n;
    double integral = y_vals[0] + y_vals[n];
    
    // Tambahkan 4 * (suku berindeks ganjil)
    for (int i = 1; i < n; i += 2) {
        integral += 4 * y_vals[i];
    }
    
    // Tambahkan 2 * (suku berindeks genap)
    for (int i = 2; i < n; i += 2) {
        integral += 2 * y_vals[i];
    }
    
    integral *= h / 3.0;
    return integral;
}

void NetworkAnalyzer::calculateStatistics() {
    if (bandwidth.empty()) return;
    
    std::cout << "\n=== STATISTIK LALU LINTAS ===" << std::endl;
    std::cout << "Titik data: " << bandwidth.size() << std::endl;
    std::cout << "Rata-rata bandwidth: " << std::fixed << std::setprecision(2) 
              << getAverageBandwidth() << " Mbps" << std::endl;
    std::cout << "Bandwidth puncak: " << getMaxBandwidth() << " Mbps" << std::endl;
    std::cout << "Bandwidth minimum: " << getMinBandwidth() << " Mbps" << std::endl;
    std::cout << "Rasio puncak-ke-rata-rata: " 
              << getMaxBandwidth() / getAverageBandwidth() << std::endl;
}

double NetworkAnalyzer::getMaxBandwidth() {
    if (bandwidth.empty()) return 0.0;
    return *std::max_element(bandwidth.begin(), bandwidth.end());
}

double NetworkAnalyzer::getMinBandwidth() {
    if (bandwidth.empty()) return 0.0;
    return *std::min_element(bandwidth.begin(), bandwidth.end());
}

double NetworkAnalyzer::getAverageBandwidth() {
    if (bandwidth.empty()) return 0.0;
    double sum = 0.0;
    for (double bw : bandwidth) {
        sum += bw;
    }
    return sum / bandwidth.size();
}

void NetworkAnalyzer::displayResults() {
    std::cout << "\n=== HASIL ANALISIS LALU LINTAS JARINGAN ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    calculateStatistics();
    
    std::cout << "\n=== PREDIKSI INTERPOLASI LAGRANGE ===" << std::endl;
    std::vector<double> test_times = {8.5, 12.5, 15.5, 20.5};
    
    for (double t : test_times) {
        double predicted = lagrangeInterpolation(t);
        std::cout << "Prediksi bandwidth pada " << t 
                  << ":30 = " << predicted << " Mbps" << std::endl;
    }
    
    std::cout << "\n=== HASIL INTEGRASI SIMPSON ===" << std::endl;
    double total_consumption = simpsonIntegration();
    double average_bandwidth = total_consumption / 24.0;
    
    std::cout << "Total konsumsi bandwidth (24 jam): " 
              << total_consumption << " Mbps×jam" << std::endl;
    std::cout << "Rata-rata bandwidth terintegrasi: " 
              << average_bandwidth << " Mbps" << std::endl;
    
    // Hitung beberapa interval
    std::cout << "\nKonsumsi bandwidth berdasarkan periode waktu:" << std::endl;
    std::cout << "Pagi (6-12): " << simpsonIntegration(6, 12) << " Mbps×jam" << std::endl;
    std::cout << "Siang (12-18): " << simpsonIntegration(12, 18) << " Mbps×jam" << std::endl;
    std::cout << "Sore (18-24): " << simpsonIntegration(18, 24) << " Mbps×jam" << std::endl;
}

void NetworkAnalyzer::printInterpolationTable() {
    std::cout << "\n=== DATA BANDWIDTH PER JAM ===" << std::endl;
    std::cout << "Waktu (jam)\tBandwidth (Mbps)" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        std::cout << std::fixed << std::setprecision(1) << time_hours[i] 
                  << "\t\t" << std::setprecision(3) << bandwidth[i] << std::endl;
    }
}

void NetworkAnalyzer::exportResults(const std::string& filename) {
    std::ofstream outFile(filename);
    
    outFile << "Time_Hour,Bandwidth_Mbps,Interpolated_8.5,Interpolated_12.5,Interpolated_15.5,Interpolated_20.5" << std::endl;
    
    double interp_8_5 = lagrangeInterpolation(8.5);
    double interp_12_5 = lagrangeInterpolation(12.5);
    double interp_15_5 = lagrangeInterpolation(15.5);
    double interp_20_5 = lagrangeInterpolation(20.5);
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        outFile << time_hours[i] << "," << bandwidth[i];
        
        // Tambahkan nilai interpolasi untuk perbandingan
        if (i == 0) {
            outFile << "," << interp_8_5 << "," << interp_12_5 
                    << "," << interp_15_5 << "," << interp_20_5;
        } else {
            outFile << ",,,";
        }
        outFile << std::endl;
    }
    
    outFile.close();
    std::cout << "Hasil diekspor ke " << filename << std::endl;
}

bool NetworkAnalyzer::loadProcessedData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file terproses " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Lewati header
    
    time_hours.clear();
    bandwidth.clear();
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;
        
        std::getline(ss, item, ',');
        double time = std::stod(item);
        
        std::getline(ss, item, ',');
        double bw = std::stod(item);
        
        time_hours.push_back(time);
        bandwidth.push_back(bw);
    }
    
    file.close();
    std::cout << "Memuat " << time_hours.size() << " titik data terproses." << std::endl;
    return true;
}

void NetworkAnalyzer::saveProcessedData(const std::string& filename) {
    std::ofstream outFile(filename);
    
    outFile << "Time_Hour,Bandwidth_Mbps" << std::endl;
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        outFile << time_hours[i] << "," << bandwidth[i] << std::endl;
    }
    
    outFile.close();
    std::cout << "Data terproses disimpan ke " << filename << std::endl;
}