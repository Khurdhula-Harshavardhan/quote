#include<iostream>
#include<curl/curl.h>
#include<string>
#include<vector>
#include<sstream>
#include<iomanip>
#include<algorithm>
#include<cmath>
#include<chrono>

namespace quote {
// Stock data structure
struct StockData {
    std::string symbol;
    std::string name;
    std::string currency;
    std::string exchange;
    double currentPrice = 0.0;
    double previousClose = 0.0;
    double dayHigh = 0.0;
    double dayLow = 0.0;
    double fiftyTwoWeekHigh = 0.0;
    double fiftyTwoWeekLow = 0.0;
    long volume = 0;
    std::vector<double> prices;
    std::vector<long> timestamps;
    bool hasData = false;
    std::string lastFetchTime = "";
    double fetchDurationMs = 0.0;
};

// A simple class for a command-line tool that fetches stock quotes from Google Finance
class Quote {
private:
    std::string symbol;   // Stock symbol (e.g., "GOOGL")
    std::string exchange; // Exchange name (e.g., "NYSE")
    std::string fetchedData; // Store the raw fetched data
    StockData stockData; // Parsed stock data

    // Static callback function for curl
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // Private method to fetch data from Google Finance
    std::string fetchDataFromGoogleFinance(const std::string& symbol, const std::string& exchange) {
        // We'll use libcurl to make HTTP requests
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        // Initialize curl
        curl = curl_easy_init();
        if (curl) {
            // Let user specify the full symbol with exchange suffix
            // If exchange is provided and doesn't start with a dot, add it
            std::string fullSymbol = symbol;
            if (!exchange.empty() && exchange != "NASDAQ" && exchange != "NYSE") {
                if (exchange[0] != '.') {
                    fullSymbol += "." + exchange;
                } else {
                    fullSymbol += exchange;
                }
            }
            
            const std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + fullSymbol;
            
            // Set URL to fetch
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            // Set callback function to receive data
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            
            // Set data buffer to write to
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            
            // Set user agent to avoid being blocked
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36");
            
            // Follow redirects
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            
            // Set timeout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            
            // Perform the request
            res = curl_easy_perform(curl);
            
            // Get HTTP response code
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            
            // Check for errors
            if (res != CURLE_OK) {
                handleError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
                curl_easy_cleanup(curl);
                return "";
            }
            
            if (response_code != 200) {
                handleError("HTTP request failed with response code: " + std::to_string(response_code));
            }
            
            // Clean up
            curl_easy_cleanup(curl);
            
            return readBuffer;
        } else {
            handleError("Failed to initialize curl");
            return "";
        }
    }


    // Private method to parse the fetched data
    void parseData(const std::string& data) {
        // Check if the response contains valid data
        if (data.find("\"result\":null") != std::string::npos || 
            data.find("\"result\":[]") != std::string::npos) {
            fetchedData = "No data found for this symbol/exchange combination.";
            return;
        }
        
        // More specific error checking - look for actual error structure
        if (data.find("\"chart\":{\"error\"") != std::string::npos) {
            fetchedData = "API returned an error.";
            return;
        }

        // Parse JSON response
        parseStockData(data);
        fetchedData = data; // Keep raw data for debugging
    }

    // Parse JSON and extract stock data
    void parseStockData(const std::string& jsonData) {
        // Save timing info before resetting stockData
        std::string savedFetchTime = stockData.lastFetchTime;
        double savedFetchDuration = stockData.fetchDurationMs;
        
        stockData = StockData();
        
        // Restore timing info
        stockData.lastFetchTime = savedFetchTime;
        stockData.fetchDurationMs = savedFetchDuration;
        
        try {
            // Check if we have valid result data
            if (jsonData.find("\"result\":null") != std::string::npos) {
                return;
            }
            
            // Extract meta information from the first result
            size_t metaPos = jsonData.find("\"meta\":");
            if (metaPos == std::string::npos) {
                return;
            }
            
            // Extract from meta section
            stockData.symbol = extractStringFromMeta(jsonData, "symbol");
            stockData.name = extractStringFromMeta(jsonData, "longName");
            if (stockData.name.empty()) {
                stockData.name = extractStringFromMeta(jsonData, "shortName");
            }
            stockData.currency = extractStringFromMeta(jsonData, "currency");
            stockData.exchange = extractStringFromMeta(jsonData, "exchangeName");
            
            // Extract price data from meta section
            stockData.currentPrice = extractDoubleFromMeta(jsonData, "regularMarketPrice");
            stockData.previousClose = extractDoubleFromMeta(jsonData, "previousClose");
            stockData.dayHigh = extractDoubleFromMeta(jsonData, "regularMarketDayHigh");
            stockData.dayLow = extractDoubleFromMeta(jsonData, "regularMarketDayLow");
            stockData.fiftyTwoWeekHigh = extractDoubleFromMeta(jsonData, "fiftyTwoWeekHigh");
            stockData.fiftyTwoWeekLow = extractDoubleFromMeta(jsonData, "fiftyTwoWeekLow");
            stockData.volume = extractLongFromMeta(jsonData, "regularMarketVolume");
            
            // If regularMarketPrice is 0, try chartPreviousClose as current price
            if (stockData.currentPrice == 0.0) {
                stockData.currentPrice = extractDoubleFromMeta(jsonData, "chartPreviousClose");
            }
            
            // Extract price array for graph from indicators.quote[0].close
            extractPriceArrayFromIndicators(jsonData, stockData.prices, stockData.timestamps);
            
            stockData.hasData = (stockData.currentPrice > 0);
            
        } catch (const std::exception& e) {
            stockData.hasData = false;
        } catch (...) {
            stockData.hasData = false;
        }
    }

    // Helper function to extract string values from meta section
    std::string extractStringFromMeta(const std::string& json, const std::string& key) {
        size_t metaPos = json.find("\"meta\":");
        if (metaPos == std::string::npos) return "";
        
        size_t metaEnd = json.find("\"timestamp\":", metaPos);
        if (metaEnd == std::string::npos) metaEnd = json.size();
        
        std::string metaSection = json.substr(metaPos, metaEnd - metaPos);
        
        std::string searchKey = "\"" + key + "\":\"";
        size_t pos = metaSection.find(searchKey);
        if (pos != std::string::npos) {
            pos += searchKey.length();
            size_t end = metaSection.find("\"", pos);
            if (end != std::string::npos) {
                return metaSection.substr(pos, end - pos);
            }
        }
        return "";
    }

    // Helper function to extract double values from meta section
    double extractDoubleFromMeta(const std::string& json, const std::string& key) {
        size_t metaPos = json.find("\"meta\":");
        if (metaPos == std::string::npos) return 0.0;
        
        size_t metaEnd = json.find("\"timestamp\":", metaPos);
        if (metaEnd == std::string::npos) metaEnd = json.size();
        
        std::string metaSection = json.substr(metaPos, metaEnd - metaPos);
        
        std::string searchKey = "\"" + key + "\":";
        size_t pos = metaSection.find(searchKey);
        if (pos != std::string::npos) {
            pos += searchKey.length();
            size_t end = metaSection.find_first_of(",}", pos);
            if (end != std::string::npos) {
                std::string valueStr = metaSection.substr(pos, end - pos);
                try {
                    return std::stod(valueStr);
                } catch (...) {
                    return 0.0;
                }
            }
        }
        return 0.0;
    }

    // Helper function to extract long values from meta section
    long extractLongFromMeta(const std::string& json, const std::string& key) {
        size_t metaPos = json.find("\"meta\":");
        if (metaPos == std::string::npos) return 0;
        
        size_t metaEnd = json.find("\"timestamp\":", metaPos);
        if (metaEnd == std::string::npos) metaEnd = json.size();
        
        std::string metaSection = json.substr(metaPos, metaEnd - metaPos);
        
        std::string searchKey = "\"" + key + "\":";
        size_t pos = metaSection.find(searchKey);
        if (pos != std::string::npos) {
            pos += searchKey.length();
            size_t end = metaSection.find_first_of(",}", pos);
            if (end != std::string::npos) {
                std::string valueStr = metaSection.substr(pos, end - pos);
                try {
                    return std::stol(valueStr);
                } catch (...) {
                    return 0;
                }
            }
        }
        return 0;
    }

    // Helper functions for JSON parsing
    void extractValue(const std::string& json, const std::string& key, std::string& value) {
        size_t pos = json.find(key);
        if (pos != std::string::npos) {
            pos += key.length();
            size_t end = json.find("\"", pos);
            if (end != std::string::npos) {
                value = json.substr(pos, end - pos);
            }
        }
    }

    double extractDoubleValue(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos != std::string::npos) {
            pos += key.length();
            size_t end = json.find_first_of(",}", pos);
            if (end != std::string::npos) {
                std::string valueStr = json.substr(pos, end - pos);
                return std::stod(valueStr);
            }
        }
        return 0.0;
    }

    long extractLongValue(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos != std::string::npos) {
            pos += key.length();
            size_t end = json.find_first_of(",}", pos);
            if (end != std::string::npos) {
                std::string valueStr = json.substr(pos, end - pos);
                return std::stol(valueStr);
            }
        }
        return 0;
    }

    void extractPriceArrayFromIndicators(const std::string& json, std::vector<double>& prices, std::vector<long>& timestamps) {
        // First extract timestamps from the main data section
        size_t timestampPos = json.find("\"timestamp\":[");
        if (timestampPos != std::string::npos) {
            timestampPos += 13; // length of "\"timestamp\":["
            size_t endPos = json.find("]", timestampPos);
            if (endPos != std::string::npos) {
                std::string timestampStr = json.substr(timestampPos, endPos - timestampPos);
                std::stringstream ss(timestampStr);
                std::string timestamp;
                
                while (std::getline(ss, timestamp, ',')) {
                    try {
                        // Remove any whitespace
                        timestamp.erase(0, timestamp.find_first_not_of(" \t\n\r"));
                        timestamp.erase(timestamp.find_last_not_of(" \t\n\r") + 1);
                        
                        if (!timestamp.empty() && timestamp != "null") {
                            timestamps.push_back(std::stol(timestamp));
                        }
                    } catch (...) {
                        // Skip invalid values
                    }
                }
            }
        }
        
        // Look for the indicators section with quote data
        size_t indicatorsPos = json.find("\"indicators\":");
        if (indicatorsPos == std::string::npos) return;
        
        size_t quotePos = json.find("\"quote\":[", indicatorsPos);
        if (quotePos == std::string::npos) return;
        
        size_t closePos = json.find("\"close\":[", quotePos);
        if (closePos != std::string::npos) {
            closePos += 9; // length of "\"close\":["
            size_t endPos = json.find("]", closePos);
            if (endPos != std::string::npos) {
                std::string priceStr = json.substr(closePos, endPos - closePos);
                std::stringstream ss(priceStr);
                std::string price;
                
                while (std::getline(ss, price, ',')) {
                    try {
                        // Remove any whitespace
                        price.erase(0, price.find_first_not_of(" \t\n\r"));
                        price.erase(price.find_last_not_of(" \t\n\r") + 1);
                        
                        if (!price.empty() && price != "null") {
                            prices.push_back(std::stod(price));
                        }
                    } catch (...) {
                        // Skip invalid values - keep arrays aligned
                        if (!timestamps.empty() && timestamps.size() > prices.size()) {
                            timestamps.pop_back();
                        }
                    }
                }
            }
        }
        
        // Ensure arrays are the same size - trim timestamps if needed
        while (timestamps.size() > prices.size() && !timestamps.empty()) {
            timestamps.pop_back();
        }
    }

    // Generate ASCII graph
    std::string generateGraph(const std::vector<double>& prices, int width = 60, int height = 10) const {
        if (prices.empty()) return "No data available for graph";
        
        // Take last 'width' prices or all if less
        std::vector<double> graphPrices;
        int startIndex = std::max(0, (int)prices.size() - width);
        for (int i = startIndex; i < (int)prices.size(); i++) {
            graphPrices.push_back(prices[i]);
        }
        
        if (graphPrices.empty()) return "No data available for graph";
        
        double minPrice = *std::min_element(graphPrices.begin(), graphPrices.end());
        double maxPrice = *std::max_element(graphPrices.begin(), graphPrices.end());
        double range = maxPrice - minPrice;
        
        if (range == 0) range = 1; // Avoid division by zero
        
        std::string graph;
        
        // Generate graph from top to bottom
        for (int row = height - 1; row >= 0; row--) {
            double threshold = minPrice + (range * (row + 0.5)) / height;
            std::string line;
            
            for (double price : graphPrices) {
                if (price >= threshold) {
                    line += "█";
                } else {
                    line += " ";
                }
            }
            
            // Add Y-axis labels
            double yValue = minPrice + (range * (row + 1)) / height;
            graph += std::string(8 - std::to_string((int)yValue).length(), ' ') + 
                     std::to_string((int)yValue) + " |" + line + "\n";
        }
        
        // Add X-axis base line
        graph += "     +" + std::string(graphPrices.size(), '-') + "\n";
        
        // Add X-axis time labels - use actual timestamps if available
        std::string xAxisLabels = "      ";
        int numLabels = std::min(5, (int)graphPrices.size()); // Show up to 5 time labels
        
        if (numLabels > 1 && !stockData.timestamps.empty()) {
            // Use actual timestamps from the data
            int startIndex = std::max(0, (int)stockData.timestamps.size() - (int)graphPrices.size());
            
            for (int labelIdx = 0; labelIdx < numLabels; labelIdx++) {
                int pointIndex = (labelIdx * (graphPrices.size() - 1)) / (numLabels - 1);
                int timestampIndex = startIndex + pointIndex;
                
                if (timestampIndex < (int)stockData.timestamps.size()) {
                    // Convert Unix timestamp to local time
                    time_t timestamp = stockData.timestamps[timestampIndex];
                    auto* tm = std::localtime(&timestamp);
                    
                    // Format time as HH:MM
                    std::string timeLabel;
                    if (tm->tm_hour < 10) timeLabel += "0";
                    timeLabel += std::to_string(tm->tm_hour) + ":";
                    if (tm->tm_min < 10) timeLabel += "0";
                    timeLabel += std::to_string(tm->tm_min);
                    
                    // Calculate spacing for alignment
                    int targetPos = (pointIndex * 60) / graphPrices.size(); // Approximate position in 60-char width
                    int currentPos = xAxisLabels.length() - 6; // Subtract initial spacing
                    int spacingNeeded = std::max(0, targetPos - currentPos);
                    
                    // Add appropriate spacing
                    if (labelIdx > 0) {
                        spacingNeeded = std::max(1, spacingNeeded); // At least 1 space between labels
                    }
                    
                    xAxisLabels += std::string(spacingNeeded, ' ') + timeLabel;
                }
            }
        } else if (numLabels > 1) {
            // Fallback to current time calculation if no timestamps available
            auto now = std::chrono::system_clock::now();
            auto currentTime = std::chrono::system_clock::to_time_t(now);
            
            // Calculate time span covered by the graph (assume 1-5 minute intervals based on data size)
            int minutesPerPoint = 1;
            if (graphPrices.size() > 200) minutesPerPoint = 5;      // 5-min intervals for larger datasets
            else if (graphPrices.size() > 78) minutesPerPoint = 2;  // 2-min intervals for medium datasets
            
            for (int labelIdx = 0; labelIdx < numLabels; labelIdx++) {
                int pointIndex = (labelIdx * (graphPrices.size() - 1)) / (numLabels - 1);
                int minutesBack = (graphPrices.size() - 1 - pointIndex) * minutesPerPoint;
                
                // Calculate the time for this point
                auto pointTime = currentTime - minutesBack * 60;
                auto* tm = std::localtime(&pointTime);
                
                // Format time as HH:MM
                std::string timeLabel;
                if (tm->tm_hour < 10) timeLabel += "0";
                timeLabel += std::to_string(tm->tm_hour) + ":";
                if (tm->tm_min < 10) timeLabel += "0";
                timeLabel += std::to_string(tm->tm_min);
                
                // Calculate spacing for alignment
                int targetPos = (pointIndex * 60) / graphPrices.size(); // Approximate position in 60-char width
                int currentPos = xAxisLabels.length() - 6; // Subtract initial spacing
                int spacingNeeded = std::max(0, targetPos - currentPos);
                
                // Add appropriate spacing
                if (labelIdx > 0) {
                    spacingNeeded = std::max(1, spacingNeeded); // At least 1 space between labels
                }
                
                xAxisLabels += std::string(spacingNeeded, ' ') + timeLabel;
            }
        }
        
        graph += xAxisLabels + "\n";
        
        // Dynamic interval description based on actual data
        std::string intervalDesc = "      (Last " + std::to_string(graphPrices.size()) + " data points";
        
        if (!stockData.timestamps.empty() && stockData.timestamps.size() >= 2) {
            // Calculate actual interval from timestamps
            int startIndex = std::max(0, (int)stockData.timestamps.size() - (int)graphPrices.size());
            if (startIndex + 1 < (int)stockData.timestamps.size()) {
                long interval = stockData.timestamps[startIndex + 1] - stockData.timestamps[startIndex];
                int minutes = interval / 60;
                if (minutes > 0) {
                    intervalDesc += " - " + std::to_string(minutes) + "min intervals";
                } else {
                    intervalDesc += " - real-time data";
                }
            }
        } else {
            // Fallback to estimated intervals
            int minutesPerPoint = 1;
            if (graphPrices.size() > 200) minutesPerPoint = 5;
            else if (graphPrices.size() > 78) minutesPerPoint = 2;
            intervalDesc += " - " + std::to_string(minutesPerPoint) + "min intervals";
        }
        
        intervalDesc += ")\n";
        graph += intervalDesc;
        
        return graph;
    }

    // Create dashboard display
    void displayStockInfo() const {
        if (!stockData.hasData) {
            std::cout << "No valid stock data available." << std::endl;
            return;
        }

        // Calculate change
        double change = stockData.currentPrice - stockData.previousClose;
        double changePercent = (change / stockData.previousClose) * 100;
        
        // Color codes
        std::string green = "\033[32m";
        std::string red = "\033[31m";
        std::string blue = "\033[34m";
        std::string yellow = "\033[33m";
        std::string reset = "\033[0m";
        std::string bold = "\033[1m";
        
        std::string changeColor = (change >= 0) ? green : red;
        std::string changeSymbol = (change >= 0) ? "▲" : "▼";
        
        // Header
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << bold << "  STOCK QUOTE DASHBOARD" << reset << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        // Stock info
        std::cout << bold << blue << stockData.symbol << reset;
        if (!stockData.name.empty()) {
            std::cout << " - " << stockData.name;
        }
        std::cout << " (" << stockData.exchange << ")" << std::endl;
        
        // Current price and change
        std::cout << "\n" << bold << "Current Price: " << reset 
                  << yellow << std::fixed << std::setprecision(2) 
                  << stockData.currentPrice << " " << stockData.currency << reset;
        
        std::cout << "  " << changeColor << changeSymbol << " " 
                  << std::abs(change) << " (" 
                  << std::showpos << std::setprecision(2) << changePercent 
                  << "%)" << std::noshowpos << reset << std::endl;
        
        // Price ranges
        std::cout << "\n" << bold << "Day Range:    " << reset 
                  << stockData.dayLow << " - " << stockData.dayHigh << " " << stockData.currency << std::endl;
        
        std::cout << bold << "52W Range:    " << reset 
                  << stockData.fiftyTwoWeekLow << " - " << stockData.fiftyTwoWeekHigh 
                  << " " << stockData.currency << std::endl;
        
        std::cout << bold << "Volume:       " << reset << stockData.volume << std::endl;
        std::cout << bold << "Prev Close:   " << reset << stockData.previousClose 
                  << " " << stockData.currency << std::endl;
        
        // Fetch information
        if (!stockData.lastFetchTime.empty()) {
            std::cout << "\n" << bold << "Last Updated: " << reset << stockData.lastFetchTime;
            if (stockData.fetchDurationMs > 0) {
                std::cout << " (" << std::fixed << std::setprecision(1) 
                          << stockData.fetchDurationMs << "ms)";
            }
            std::cout << std::endl;
        }
        
        // Price graph
        if (!stockData.prices.empty()) {
            std::cout << "\n" << bold << "Intraday Price Chart:" << reset << std::endl;
            std::cout << generateGraph(stockData.prices) << std::endl;
        }
    }

    // Private method to handle errors
    void handleError(const std::string& errorMessage) {
        std::cerr << "Error: " << errorMessage << std::endl;
    }
    
    // Private method to validate the stock symbol
    bool isValidSymbol(const std::string& symbol) const {
        // Basic validation - not empty and contains only alphanumeric characters
        if (symbol.empty()) return false;
        for (char c : symbol) {
            if (!std::isalnum(c) && c != '.') return false;
        }
        return true;
    }
    
    // Private method to validate the exchange name
    bool isValidExchange(const std::string& exchange) const {
        // Basic validation - not empty
        return !exchange.empty();
    }

public:
    // Creates a new Quote instance
    Quote(std::string symbol = "", std::string exchange = "NYSE") {
        this->symbol = symbol;
        this->exchange = exchange;
    };

    // Cleans up resources
    ~Quote() {
        // Destructor implementation
    }

    // Gets quote data for the specified stock symbol
    void fetchQuote(const std::string& symbol)
    {
        // Start timing
        auto startTime = std::chrono::high_resolution_clock::now();
        
        if (!isValidSymbol(symbol)) {
            handleError("Invalid stock symbol: " + symbol);
            return;
        }
        
        this->symbol = symbol;
        
        std::string data = fetchDataFromGoogleFinance(symbol, exchange);
        
        // End timing
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        stockData.fetchDurationMs = duration.count() / 1000.0; // Convert to milliseconds
        
        // Set timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        stockData.lastFetchTime = ss.str();
        
        if (!data.empty()) {
            parseData(data);
            displayStockInfo();
        } else {
            handleError("Failed to fetch data for symbol: " + symbol + " on exchange: " + exchange);
        }
    }

    // Shows the retrieved stock information
    void displayQuote() {
        if (!fetchedData.empty()) {
            displayStockInfo();
        } else {
            std::cout << "No quote data available. Please fetch a quote first." << std::endl;
        }
    }
};
}



