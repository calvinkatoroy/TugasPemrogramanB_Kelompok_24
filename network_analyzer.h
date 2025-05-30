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
    double timestamp;      // Unix timestamp
    double bandwidth_mbps; // Bandwidth in Mbps
    int packet_count;      // Number of packets in this interval
};

class NetworkAnalyzer {
private:
    std::vector<TrafficData> data;
    std::vector<double> time_hours;    // Time in hours (0-24)
    std::vector<double> bandwidth;     // Corresponding bandwidth values
    
    // Helper functions
    double convertToMbps(double bytes, double time_interval);
    void aggregateData(const std::vector<std::pair<double, int>>& raw_packets);
    void generateHourlyPattern();
    
public:
    // Constructor
    NetworkAnalyzer();
    
    // Data loading and processing
    bool loadRawData(const std::string& filename);
    bool loadProcessedData(const std::string& filename);
    void saveProcessedData(const std::string& filename);
    
    // Numerical methods
    double lagrangeInterpolation(double target_time);
    double simpsonIntegration();
    double simpsonIntegration(double start_time, double end_time);
    
    // Analysis functions
    void calculateStatistics();
    double getMaxBandwidth();
    double getMinBandwidth();
    double getAverageBandwidth();
    
    // Output functions
    void displayResults();
    void exportResults(const std::string& filename);
    void printInterpolationTable();
    
    // Getters
    size_t getDataSize() const { return data.size(); }
    const std::vector<TrafficData>& getData() const { return data; }
};

#endif // NETWORK_ANALYZER_H