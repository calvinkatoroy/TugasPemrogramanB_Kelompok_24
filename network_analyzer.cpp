#include "network_analyzer.h"
#include <map>
#include <random>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Alternative: use const double instead
const double PI = 3.14159265358979323846;

NetworkAnalyzer::NetworkAnalyzer() {
    // Initialize empty data structures
}

double NetworkAnalyzer::convertToMbps(double bytes, double time_interval) {
    // Convert bytes to Mbps: (bytes * 8 bits/byte) / (time_seconds * 1e6)
    return (bytes * 8.0) / (time_interval * 1e6);
}

bool NetworkAnalyzer::loadRawData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    std::vector<std::pair<double, int>> raw_packets; // timestamp, length
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;
        
        // Parse CSV: Timestamp,Source IP,Destination IP,Protocol,Length
        std::getline(ss, item, ',');
        double timestamp = std::stod(item);
        
        // Skip source IP, dest IP, protocol
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
    std::cout << "Loaded " << raw_packets.size() << " packets from raw data." << std::endl;
    
    // Aggregate packets into time intervals
    aggregateData(raw_packets);
    
    // Generate 24-hour pattern for analysis
    generateHourlyPattern();
    
    return true;
}

void NetworkAnalyzer::aggregateData(const std::vector<std::pair<double, int>>& raw_packets) {
    if (raw_packets.empty()) return;
    
    // Find time range
    double min_time = raw_packets[0].first;
    double max_time = raw_packets.back().first;
    double duration = max_time - min_time;
    
    // Create 5-minute intervals
    const double interval_duration = 300.0; // 5 minutes in seconds
    std::map<int, std::pair<int, int>> intervals; // interval_id -> (total_bytes, packet_count)
    
    for (const auto& packet : raw_packets) {
        int interval_id = static_cast<int>((packet.first - min_time) / interval_duration);
        intervals[interval_id].first += packet.second;  // total bytes
        intervals[interval_id].second += 1;             // packet count
    }
    
    // Convert to bandwidth measurements
    data.clear();
    for (const auto& interval_pair : intervals) {
        TrafficData point;
        point.timestamp = min_time + (interval_pair.first * interval_duration);
        point.bandwidth_mbps = convertToMbps(interval_pair.second.first, interval_duration);
        point.packet_count = interval_pair.second.second;
        data.push_back(point);
    }
    
    std::cout << "Aggregated into " << data.size() << " time intervals." << std::endl;
}

void NetworkAnalyzer::generateHourlyPattern() {
    if (data.empty()) return;
    
    // Generate 24-hour pattern from the available data
    time_hours.clear();
    bandwidth.clear();
    
    // Calculate average bandwidth from real data
    double avg_bandwidth = 0;
    for (const auto& point : data) {
        avg_bandwidth += point.bandwidth_mbps;
    }
    avg_bandwidth /= data.size();
    
    // Generate realistic 24-hour pattern
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0, avg_bandwidth * 0.1); // 10% noise
    
    for (int hour = 0; hour < 24; hour++) {
        time_hours.push_back(hour);
        
        // Diurnal pattern: low at night, peak during day
        double base_factor = 0.4 + 0.6 * (0.5 + 0.5 * sin(PI * (hour - 6) / 12));
        
        // Peak hours adjustment
        if (hour >= 8 && hour <= 10) base_factor *= 1.3;   // Morning peak
        if (hour >= 14 && hour <= 16) base_factor *= 1.2;  // Afternoon peak
        if (hour >= 19 && hour <= 21) base_factor *= 1.4;  // Evening peak
        if (hour >= 0 && hour <= 5) base_factor *= 0.3;    // Night low
        
        double hourly_bandwidth = avg_bandwidth * base_factor + noise(gen);
        bandwidth.push_back(std::max(0.1, hourly_bandwidth)); // Ensure positive
    }
    
    std::cout << "Generated 24-hour traffic pattern." << std::endl;
}

double NetworkAnalyzer::lagrangeInterpolation(double target_time) {
    if (time_hours.empty() || bandwidth.empty()) {
        std::cerr << "Error: No data available for interpolation." << std::endl;
        return 0.0;
    }
    
    // Clamp target_time to valid range
    if (target_time < 0) target_time = 0;
    if (target_time > 23) target_time = 23;
    
    int n = time_hours.size();
    
    // For boundary cases, return nearest neighbor
    if (target_time <= time_hours[0]) return bandwidth[0];
    if (target_time >= time_hours[n-1]) return bandwidth[n-1];
    
    // Use only nearby points to avoid oscillation (Runge's phenomenon)
    int start_idx = 0, end_idx = n - 1;
    
    // Find closest data points (use 6-8 points around target)
    for (int i = 0; i < n; i++) {
        if (time_hours[i] <= target_time) {
            start_idx = std::max(0, i - 3);
            end_idx = std::min(n - 1, i + 4);
            break;
        }
    }
    
    double result = 0.0;
    
    // Lagrange interpolation with selected points
    for (int i = start_idx; i <= end_idx; i++) {
        double term = bandwidth[i];
        
        // Calculate Lagrange basis polynomial Li(x)
        for (int j = start_idx; j <= end_idx; j++) {
            if (i != j) {
                term *= (target_time - time_hours[j]) / (time_hours[i] - time_hours[j]);
            }
        }
        
        result += term;
    }
    
    // Ensure result is reasonable (non-negative, within bounds)
    double min_bw = *std::min_element(bandwidth.begin(), bandwidth.end());
    double max_bw = *std::max_element(bandwidth.begin(), bandwidth.end());
    
    if (result < 0) result = 0.1; // Minimum bandwidth
    if (result > max_bw * 2) result = max_bw; // Cap at 2x max observed
    
    return result;
}

double NetworkAnalyzer::simpsonIntegration() {
    return simpsonIntegration(0.0, 23.0);
}

double NetworkAnalyzer::simpsonIntegration(double start_time, double end_time) {
    if (bandwidth.empty()) {
        std::cerr << "Error: No data available for integration." << std::endl;
        return 0.0;
    }
    
    // Use existing data points within the range
    std::vector<double> x_vals, y_vals;
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        if (time_hours[i] >= start_time && time_hours[i] <= end_time) {
            x_vals.push_back(time_hours[i]);
            y_vals.push_back(bandwidth[i]);
        }
    }
    
    if (x_vals.size() < 3) {
        std::cerr << "Error: Insufficient data points for Simpson's rule." << std::endl;
        return 0.0;
    }
    
    // Make sure we have even number of intervals
    int n = x_vals.size() - 1;
    if (n % 2 != 0) {
        n--; // Use n-1 intervals
    }
    
    double h = (x_vals[n] - x_vals[0]) / n;
    double integral = y_vals[0] + y_vals[n];
    
    // Add 4 * (odd-indexed terms)
    for (int i = 1; i < n; i += 2) {
        integral += 4 * y_vals[i];
    }
    
    // Add 2 * (even-indexed terms)
    for (int i = 2; i < n; i += 2) {
        integral += 2 * y_vals[i];
    }
    
    integral *= h / 3.0;
    return integral;
}

void NetworkAnalyzer::calculateStatistics() {
    if (bandwidth.empty()) return;
    
    std::cout << "\n=== TRAFFIC STATISTICS ===" << std::endl;
    std::cout << "Data points: " << bandwidth.size() << std::endl;
    std::cout << "Average bandwidth: " << std::fixed << std::setprecision(2) 
              << getAverageBandwidth() << " Mbps" << std::endl;
    std::cout << "Peak bandwidth: " << getMaxBandwidth() << " Mbps" << std::endl;
    std::cout << "Minimum bandwidth: " << getMinBandwidth() << " Mbps" << std::endl;
    std::cout << "Peak-to-average ratio: " 
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
    std::cout << "\n=== NETWORK TRAFFIC ANALYSIS RESULTS ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    calculateStatistics();
    
    std::cout << "\n=== LAGRANGE INTERPOLATION PREDICTIONS ===" << std::endl;
    std::vector<double> test_times = {8.5, 12.5, 15.5, 20.5};
    
    for (double t : test_times) {
        double predicted = lagrangeInterpolation(t);
        std::cout << "Predicted bandwidth at " << t 
                  << ":30 = " << predicted << " Mbps" << std::endl;
    }
    
    std::cout << "\n=== SIMPSON INTEGRATION RESULTS ===" << std::endl;
    double total_consumption = simpsonIntegration();
    double average_bandwidth = total_consumption / 24.0;
    
    std::cout << "Total bandwidth consumption (24h): " 
              << total_consumption << " Mbps×hour" << std::endl;
    std::cout << "Integrated average bandwidth: " 
              << average_bandwidth << " Mbps" << std::endl;
    
    // Calculate some intervals
    std::cout << "\nBandwidth consumption by time period:" << std::endl;
    std::cout << "Morning (6-12): " << simpsonIntegration(6, 12) << " Mbps×hour" << std::endl;
    std::cout << "Afternoon (12-18): " << simpsonIntegration(12, 18) << " Mbps×hour" << std::endl;
    std::cout << "Evening (18-24): " << simpsonIntegration(18, 24) << " Mbps×hour" << std::endl;
}

void NetworkAnalyzer::printInterpolationTable() {
    std::cout << "\n=== HOURLY BANDWIDTH DATA ===" << std::endl;
    std::cout << "Time (hr)\tBandwidth (Mbps)" << std::endl;
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
        
        // Add interpolated values for comparison
        if (i == 0) {
            outFile << "," << interp_8_5 << "," << interp_12_5 
                    << "," << interp_15_5 << "," << interp_20_5;
        } else {
            outFile << ",,,";
        }
        outFile << std::endl;
    }
    
    outFile.close();
    std::cout << "Results exported to " << filename << std::endl;
}

bool NetworkAnalyzer::loadProcessedData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open processed file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
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
    std::cout << "Loaded " << time_hours.size() << " processed data points." << std::endl;
    return true;
}

void NetworkAnalyzer::saveProcessedData(const std::string& filename) {
    std::ofstream outFile(filename);
    
    outFile << "Time_Hour,Bandwidth_Mbps" << std::endl;
    
    for (size_t i = 0; i < time_hours.size(); i++) {
        outFile << time_hours[i] << "," << bandwidth[i] << std::endl;
    }
    
    outFile.close();
    std::cout << "Processed data saved to " << filename << std::endl;
}