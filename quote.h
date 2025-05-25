#include<iostream>
#include<curl/curl.h>
#include<string>



namespace quote {
// A simple class for a command-line tool that fetches stock quotes from Google Finance
class Quote {
private:
    std::string symbol;   // Stock symbol (e.g., "GOOGL")
    std::string exchange; // Exchange name (e.g., "NYSE")
    std::string fetchedData; // Store the raw fetched data

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
            // Try a different API endpoint - Yahoo Finance alternative
            const std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol;
            
            std::cout << "Debug: Fetching URL: " << url << std::endl;
            
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
        // For now, just store the entire fetched result
        fetchedData = data;
    }
    
    // Private method to display the stock information
    void displayStockInfo() const {
        std::cout << "=== Stock Quote for " << symbol << " (" << exchange << ") ===" << std::endl;
        std::cout << "Raw data received:" << std::endl;
        std::cout << fetchedData << std::endl;
        std::cout << "================================================" << std::endl;
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



