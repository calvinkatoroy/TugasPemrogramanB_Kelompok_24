#include "network_analyzer.h"
#include <iostream>
#include <string>

void printHeader() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  PROGRAM ANALISIS LALU LINTAS JARINGAN" << std::endl;
    std::cout << "  Menggunakan Interpolasi Lagrange & Integrasi Simpson" << std::endl;
    std::cout << "  Analisis Dataset Universitas Cincinnati" << std::endl;
    std::cout << "  Kelompok 24: Calvin, Jonathan, Ekananda" << std::endl;
    std::cout << "==========================================" << std::endl;
}

void printMenu() {
    std::cout << "\n--- PILIHAN ANALISIS ---" << std::endl;
    std::cout << "1. Muat dan proses data mentah (output1.csv)" << std::endl;
    std::cout << "2. Muat data yang sudah diproses" << std::endl;
    std::cout << "3. Tampilkan statistik lalu lintas" << std::endl;
    std::cout << "4. Tampilkan tabel data per jam" << std::endl;
    std::cout << "5. Jalankan tes interpolasi Lagrange" << std::endl;
    std::cout << "6. Jalankan analisis integrasi Simpson" << std::endl;
    std::cout << "7. Ekspor hasil ke CSV" << std::endl;
    std::cout << "8. Mode prediksi interaktif" << std::endl;
    std::cout << "9. Laporan analisis lengkap" << std::endl;
    std::cout << "0. Keluar" << std::endl;
    std::cout << "Pilihan: ";
}

void interactivePrediction(NetworkAnalyzer& analyzer) {
    char continue_prediction = 'y';
    
    while (continue_prediction == 'y' || continue_prediction == 'Y') {
        double target_time;
        std::cout << "\nMasukkan waktu (0-24 jam) untuk prediksi bandwidth: ";
        std::cin >> target_time;
        
        if (target_time >= 0 && target_time <= 24) {
            double prediction = analyzer.lagrangeInterpolation(target_time);
            std::cout << "Prediksi bandwidth pada " << std::fixed << std::setprecision(2) 
                      << target_time << ":00 = " << std::setprecision(3) 
                      << prediction << " Mbps" << std::endl;
                      
            // Tampilkan nilai aktual terdekat untuk perbandingan
            int hour_before = static_cast<int>(target_time);
            int hour_after = hour_before + 1;
            
            if (hour_after <= 23) {
                double before = analyzer.lagrangeInterpolation(hour_before);
                double after = analyzer.lagrangeInterpolation(hour_after);
                std::cout << "  Referensi: " << hour_before << ":00 = " << before 
                          << " Mbps, " << hour_after << ":00 = " << after << " Mbps" << std::endl;
            }
        } else {
            std::cout << "Waktu tidak valid. Silakan masukkan nilai antara 0 dan 24." << std::endl;
        }
        
        std::cout << "Lanjutkan prediksi? (y/n): ";
        std::cin >> continue_prediction;
    }
}

void runCompleteAnalysis(NetworkAnalyzer& analyzer) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "           LAPORAN ANALISIS LENGKAP" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Tampilkan semua hasil
    analyzer.displayResults();
    
    std::cout << "\n=== ANALISIS INTERPOLASI DETAIL ===" << std::endl;
    
    // Tes interpolasi pada berbagai titik
    std::vector<double> test_points = {0.5, 2.5, 6.5, 9.5, 11.5, 13.5, 16.5, 18.5, 21.5, 23.5};
    
    std::cout << "Waktu\tPrediksi Bandwidth (Mbps)" << std::endl;
    std::cout << "----\t-------------------------" << std::endl;
    
    for (double t : test_points) {
        double pred = analyzer.lagrangeInterpolation(t);
        std::cout << std::fixed << std::setprecision(1) << t << ":30\t" 
                  << std::setprecision(3) << pred << std::endl;
    }
    
    std::cout << "\n=== ANALISIS INTEGRASI BERDASARKAN PERIODE WAKTU ===" << std::endl;
    
    struct TimePeriod {
        std::string name;
        double start, end;
    };
    
    std::vector<TimePeriod> periods = {
        {"Malam (0-6)", 0, 6},
        {"Pagi (6-12)", 6, 12},
        {"Siang (12-18)", 12, 18},
        {"Sore (18-24)", 18, 24},
        {"Jam Sibuk (8-10)", 8, 10},
        {"Jam Sepi (22-6)", 22, 24}  // Catatan: ini hanya sebagian
    };
    
    double total_24h = analyzer.simpsonIntegration(0, 24);
    
    for (const auto& period : periods) {
        double consumption = analyzer.simpsonIntegration(period.start, period.end);
        double percentage = (consumption / total_24h) * 100;
        double avg_in_period = consumption / (period.end - period.start);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << period.name << ":" << std::endl;
        std::cout << "  Total konsumsi: " << consumption << " Mbps×jam" << std::endl;
        std::cout << "  Persentase harian: " << percentage << "%" << std::endl;
        std::cout << "  Rata-rata dalam periode: " << avg_in_period << " Mbps" << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "=== STATISTIK RINGKASAN ===" << std::endl;
    std::cout << "Total konsumsi harian: " << total_24h << " Mbps×jam" << std::endl;
    std::cout << "Rata-rata bandwidth keseluruhan: " << analyzer.getAverageBandwidth() << " Mbps" << std::endl;
    std::cout << "Bandwidth puncak: " << analyzer.getMaxBandwidth() << " Mbps" << std::endl;
    std::cout << "Bandwidth minimum: " << analyzer.getMinBandwidth() << " Mbps" << std::endl;
    std::cout << "Faktor pemanfaatan puncak: " << std::setprecision(2) 
              << analyzer.getMaxBandwidth() / analyzer.getAverageBandwidth() << std::endl;
}

int main() {
    printHeader();
    
    NetworkAnalyzer analyzer;
    bool dataLoaded = false;
    int choice;
    
    while (true) {
        printMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                std::cout << "\nMemuat data mentah dari data/raw/output1.csv..." << std::endl;
                if (analyzer.loadRawData("data/raw/output1.csv")) {
                    std::cout << "Data mentah berhasil dimuat dan diproses!" << std::endl;
                    analyzer.saveProcessedData("data/processed/network_traffic_timeseries.csv");
                    dataLoaded = true;
                } else {
                    std::cout << "Gagal memuat data mentah. Periksa lokasi file." << std::endl;
                }
                break;
            }
            
            case 2: {
                std::cout << "\nMemuat data yang sudah diproses..." << std::endl;
                if (analyzer.loadProcessedData("data/processed/network_traffic_timeseries.csv")) {
                    std::cout << "Data terproses berhasil dimuat!" << std::endl;
                    dataLoaded = true;
                } else {
                    std::cout << "Gagal memuat data terproses." << std::endl;
                }
                break;
            }
            
            case 3: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                analyzer.calculateStatistics();
                break;
            }
            
            case 4: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                analyzer.printInterpolationTable();
                break;
            }
            
            case 5: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                std::cout << "\n=== TES INTERPOLASI LAGRANGE ===" << std::endl;
                std::vector<double> test_times = {8.5, 12.5, 15.5, 20.5};
                
                for (double t : test_times) {
                    double predicted = analyzer.lagrangeInterpolation(t);
                    std::cout << "Bandwidth pada " << t << ":30 = " 
                              << std::fixed << std::setprecision(3) << predicted << " Mbps" << std::endl;
                }
                break;
            }
            
            case 6: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                std::cout << "\n=== ANALISIS INTEGRASI SIMPSON ===" << std::endl;
                double total = analyzer.simpsonIntegration();
                std::cout << "Total konsumsi bandwidth (24 jam): " 
                          << std::fixed << std::setprecision(3) << total << " Mbps×jam" << std::endl;
                std::cout << "Rata-rata bandwidth: " << total/24.0 << " Mbps" << std::endl;
                break;
            }
            
            case 7: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                analyzer.exportResults("data/results/analysis_output.csv");
                break;
            }
            
            case 8: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                interactivePrediction(analyzer);
                break;
            }
            
            case 9: {
                if (!dataLoaded) {
                    std::cout << "Silakan muat data terlebih dahulu (pilihan 1 atau 2)." << std::endl;
                    break;
                }
                runCompleteAnalysis(analyzer);
                break;
            }
            
            case 0: {
                std::cout << "\nTerima kasih telah menggunakan Program Analisis Lalu Lintas Jaringan!" << std::endl;
                std::cout << "Analisis selesai." << std::endl;
                return 0;
            }
            
            default: {
                std::cout << "Pilihan tidak valid. Silakan coba lagi." << std::endl;
                break;
            }
        }
    }
    
    return 0;
}