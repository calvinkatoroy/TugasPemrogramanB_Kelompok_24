#ifndef NETWORK_ANALYZER_H
#define NETWORK_ANALYZER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <algorithm>

struct TrafficData {
    double timestamp;      // Cap waktu Unix
    double bandwidth_mbps; // Bandwidth dalam Mbps
    int packet_count;      // Jumlah paket dalam interval ini
};

class NetworkAnalyzer {
private:
    std::vector<TrafficData> data;
    std::vector<double> time_hours;    // Waktu dalam jam (0-24)
    std::vector<double> bandwidth;     // Nilai bandwidth yang sesuai
    
    // Fungsi pembantu
    double convertToMbps(double bytes, double time_interval);
    void aggregateData(const std::vector<std::pair<double, int>>& raw_packets);
    void generateHourlyPattern();
    
public:
    // Konstruktor
    NetworkAnalyzer();
    
    // Pemuatan dan pemrosesan data
    bool loadRawData(const std::string& filename);
    bool loadProcessedData(const std::string& filename);
    void saveProcessedData(const std::string& filename);
    
    // Metode numerik
    double lagrangeInterpolation(double target_time);
    double simpsonIntegration();
    double simpsonIntegration(double start_time, double end_time);
    
    // Fungsi analisis
    void calculateStatistics();
    double getMaxBandwidth();
    double getMinBandwidth();
    double getAverageBandwidth();
    
    // Fungsi keluaran
    void displayResults();
    void exportResults(const std::string& filename);
    void printInterpolationTable();
    
    // Getter
    size_t getDataSize() const { return data.size(); }
    const std::vector<TrafficData>& getData() const { return data; }
};

#endif // NETWORK_ANALYZER_H