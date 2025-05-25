#include<iostream>
#include<curl/curl.h>
#include<string>
#include<vector>
#include<sstream>
#include<iomanip>
#include<algorithm>
#include<cmath>

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
            
            std::cout << "Debug: Fetching URL: " << url << std::endl;
            std::cout << "Debug: Full symbol with exchange: " << fullSymbol << std::endl;
            
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
            
            std::cout << "Debug: HTTP Response Code: " << response_code << std::endl;
            std::cout << "Debug: Response size: " << readBuffer.size() << " bytes" << std::endl;
            
            // Check for errors
            if (res != CURLE_OK) {
                handleError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
                curl_easy_cleanup(curl);
                return "";
            }
            
            if (response_code != 200) {
                handleError("HTTP request failed with response code: " + std::to_string(response_code));
                std::cout << "Debug: Response content: " << readBuffer.substr(0, 500) << std::endl;
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
        } else if (data.find("\"error\"") != std::string::npos) {
            fetchedData = "API returned an error.";
            return;
        }

        // Parse JSON response
        parseStockData(data);
        fetchedData = data; // Keep raw data for debugging
    }

    // Parse JSON and extract stock data
    void parseStockData(const std::string& jsonData) {
        stockData = StockData();
        
        try {
            // Extract meta information
            extractValue(jsonData, "\"symbol\":\"", stockData.symbol);
            extractValue(jsonData, "\"longName\":\"", stockData.name);
            extractValue(jsonData, "\"currency\":\"", stockData.currency);
            extractValue(jsonData, "\"exchangeName\":\"", stockData.exchange);
            
            // Extract price data
            stockData.currentPrice = extractDoubleValue(jsonData, "\"regularMarketPrice\":");
            stockData.previousClose = extractDoubleValue(jsonData, "\"previousClose\":");
            stockData.dayHigh = extractDoubleValue(jsonData, "\"regularMarketDayHigh\":");
            stockData.dayLow = extractDoubleValue(jsonData, "\"regularMarketDayLow\":");
            stockData.fiftyTwoWeekHigh = extractDoubleValue(jsonData, "\"fiftyTwoWeekHigh\":");
            stockData.fiftyTwoWeekLow = extractDoubleValue(jsonData, "\"fiftyTwoWeekLow\":");
            stockData.volume = extractLongValue(jsonData, "\"regularMarketVolume\":");
            
            // Extract price array for graph
            extractPriceArray(jsonData, stockData.prices);
            
            stockData.hasData = (stockData.currentPrice > 0);
            
        } catch (...) {
            stockData.hasData = false;
        }
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

    void extractPriceArray(const std::string& json, std::vector<double>& prices) {
        size_t closePos = json.find("\"close\":[");
        if (closePos != std::string::npos) {
            closePos += 9; // length of "\"close\":["
            size_t endPos = json.find("]", closePos);
            if (endPos != std::string::npos) {
                std::string priceStr = json.substr(closePos, endPos - closePos);
                std::stringstream ss(priceStr);
                std::string price;
                
                while (std::getline(ss, price, ',')) {
                    try {
                        prices.push_back(std::stod(price));
                    } catch (...) {
                        // Skip invalid values
                    }
                }
            }
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
        
        // Add X-axis
        graph += "     +" + std::string(graphPrices.size(), '-') + "\n";
        
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
        
        // Price graph
        if (!stockData.prices.empty()) {
            std::cout << "\n" << bold << "Intraday Price Chart:" << reset << std::endl;
            std::cout << generateGraph(stockData.prices) << std::endl;
        }
        
        std::cout << std::string(80, '=') << std::endl;
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
        std::cout << "Debug: Starting fetchQuote for symbol: " << symbol << std::endl;
        
        if (!isValidSymbol(symbol)) {
            handleError("Invalid stock symbol: " + symbol);
            return;
        }
        
        std::cout << "Debug: Symbol validation passed" << std::endl;
        this->symbol = symbol;
        
        std::cout << "Debug: Calling fetchDataFromGoogleFinance" << std::endl;
        std::string data = fetchDataFromGoogleFinance(symbol, exchange);
        
        std::cout << "Debug: Received data length: " << data.length() << std::endl;
        
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



