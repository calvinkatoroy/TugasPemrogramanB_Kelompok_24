#include "network_analyzer.h"
#include <iostream>
#include <string>

void printHeader() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  NETWORK TRAFFIC ANALYSIS PROGRAM" << std::endl;
    std::cout << "  Using Lagrange Interpolation & Simpson Integration" << std::endl;
    std::cout << "  University of Cincinnati Dataset Analysis" << std::endl;
    std::cout << "  Kelompok 24: Calvin, Jonathan, Ekananda" << std::endl;
    std::cout << "==========================================" << std::endl;
}

void printMenu() {
    std::cout << "\n--- ANALYSIS OPTIONS ---" << std::endl;
    std::cout << "1. Load and process raw data (output1.csv)" << std::endl;
    std::cout << "2. Load processed data" << std::endl;
    std::cout << "3. Display traffic statistics" << std::endl;
    std::cout << "4. Show hourly data table" << std::endl;
    std::cout << "5. Run Lagrange interpolation test" << std::endl;
    std::cout << "6. Run Simpson integration analysis" << std::endl;
    std::cout << "7. Export results to CSV" << std::endl;
    std::cout << "8. Interactive prediction mode" << std::endl;
    std::cout << "9. Complete analysis report" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

void interactivePrediction(NetworkAnalyzer& analyzer) {
    char continue_prediction = 'y';
    
    while (continue_prediction == 'y' || continue_prediction == 'Y') {
        double target_time;
        std::cout << "\nEnter time (0-24 hours) for bandwidth prediction: ";
        std::cin >> target_time;
        
        if (target_time >= 0 && target_time <= 24) {
            double prediction = analyzer.lagrangeInterpolation(target_time);
            std::cout << "Predicted bandwidth at " << std::fixed << std::setprecision(2) 
                      << target_time << ":00 = " << std::setprecision(3) 
                      << prediction << " Mbps" << std::endl;
                      
            // Show nearby actual values for comparison
            int hour_before = static_cast<int>(target_time);
            int hour_after = hour_before + 1;
            
            if (hour_after <= 23) {
                double before = analyzer.lagrangeInterpolation(hour_before);
                double after = analyzer.lagrangeInterpolation(hour_after);
                std::cout << "  Reference: " << hour_before << ":00 = " << before 
                          << " Mbps, " << hour_after << ":00 = " << after << " Mbps" << std::endl;
            }
        } else {
            std::cout << "Invalid time. Please enter a value between 0 and 24." << std::endl;
        }
        
        std::cout << "Continue prediction? (y/n): ";
        std::cin >> continue_prediction;
    }
}

void runCompleteAnalysis(NetworkAnalyzer& analyzer) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "           COMPLETE ANALYSIS REPORT" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Display all results
    analyzer.displayResults();
    
    std::cout << "\n=== DETAILED INTERPOLATION ANALYSIS ===" << std::endl;
    
    // Test interpolation at various points
    std::vector<double> test_points = {0.5, 2.5, 6.5, 9.5, 11.5, 13.5, 16.5, 18.5, 21.5, 23.5};
    
    std::cout << "Time\tPredicted Bandwidth (Mbps)" << std::endl;
    std::cout << "----\t-------------------------" << std::endl;
    
    for (double t : test_points) {
        double pred = analyzer.lagrangeInterpolation(t);
        std::cout << std::fixed << std::setprecision(1) << t << ":30\t" 
                  << std::setprecision(3) << pred << std::endl;
    }
    
    std::cout << "\n=== INTEGRATION ANALYSIS BY TIME PERIODS ===" << std::endl;
    
    struct TimePeriod {
        std::string name;
        double start, end;
    };
    
    std::vector<TimePeriod> periods = {
        {"Night (0-6)", 0, 6},
        {"Morning (6-12)", 6, 12},
        {"Afternoon (12-18)", 12, 18},
        {"Evening (18-24)", 18, 24},
        {"Peak Hours (8-10)", 8, 10},
        {"Off-Peak (22-6)", 22, 24}  // Note: this is partial
    };
    
    double total_24h = analyzer.simpsonIntegration(0, 24);
    
    for (const auto& period : periods) {
        double consumption = analyzer.simpsonIntegration(period.start, period.end);
        double percentage = (consumption / total_24h) * 100;
        double avg_in_period = consumption / (period.end - period.start);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << period.name << ":" << std::endl;
        std::cout << "  Total consumption: " << consumption << " Mbps×hour" << std::endl;
        std::cout << "  Percentage of daily: " << percentage << "%" << std::endl;
        std::cout << "  Average in period: " << avg_in_period << " Mbps" << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "=== SUMMARY STATISTICS ===" << std::endl;
    std::cout << "Total daily consumption: " << total_24h << " Mbps×hour" << std::endl;
    std::cout << "Overall average bandwidth: " << analyzer.getAverageBandwidth() << " Mbps" << std::endl;
    std::cout << "Peak bandwidth: " << analyzer.getMaxBandwidth() << " Mbps" << std::endl;
    std::cout << "Minimum bandwidth: " << analyzer.getMinBandwidth() << " Mbps" << std::endl;
    std::cout << "Peak utilization factor: " << std::setprecision(2) 
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
                std::cout << "\nLoading raw data from data/raw/output1.csv..." << std::endl;
                if (analyzer.loadRawData("data/raw/output1.csv")) {
                    std::cout << "Raw data loaded and processed successfully!" << std::endl;
                    analyzer.saveProcessedData("data/processed/network_traffic_timeseries.csv");
                    dataLoaded = true;
                } else {
                    std::cout << "Failed to load raw data. Check file path." << std::endl;
                }
                break;
            }
            
            case 2: {
                std::cout << "\nLoading processed data..." << std::endl;
                if (analyzer.loadProcessedData("data/processed/network_traffic_timeseries.csv")) {
                    std::cout << "Processed data loaded successfully!" << std::endl;
                    dataLoaded = true;
                } else {
                    std::cout << "Failed to load processed data." << std::endl;
                }
                break;
            }
            
            case 3: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                analyzer.calculateStatistics();
                break;
            }
            
            case 4: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                analyzer.printInterpolationTable();
                break;
            }
            
            case 5: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                std::cout << "\n=== LAGRANGE INTERPOLATION TEST ===" << std::endl;
                std::vector<double> test_times = {8.5, 12.5, 15.5, 20.5};
                
                for (double t : test_times) {
                    double predicted = analyzer.lagrangeInterpolation(t);
                    std::cout << "Bandwidth at " << t << ":30 = " 
                              << std::fixed << std::setprecision(3) << predicted << " Mbps" << std::endl;
                }
                break;
            }
            
            case 6: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                std::cout << "\n=== SIMPSON INTEGRATION ANALYSIS ===" << std::endl;
                double total = analyzer.simpsonIntegration();
                std::cout << "Total bandwidth consumption (24h): " 
                          << std::fixed << std::setprecision(3) << total << " Mbps×hour" << std::endl;
                std::cout << "Average bandwidth: " << total/24.0 << " Mbps" << std::endl;
                break;
            }
            
            case 7: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                analyzer.exportResults("data/results/analysis_output.csv");
                break;
            }
            
            case 8: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                interactivePrediction(analyzer);
                break;
            }
            
            case 9: {
                if (!dataLoaded) {
                    std::cout << "Please load data first (option 1 or 2)." << std::endl;
                    break;
                }
                runCompleteAnalysis(analyzer);
                break;
            }
            
            case 0: {
                std::cout << "\nThank you for using Network Traffic Analysis Program!" << std::endl;
                std::cout << "Analysis complete." << std::endl;
                return 0;
            }
            
            default: {
                std::cout << "Invalid choice. Please try again." << std::endl;
                break;
            }
        }
    }
    
    return 0;
}