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

    // Private method to fetch data from Google Finance
    std::string fetchDataFromGoogleFinance(const std::string& symbol, const std::string& exchange) {
        // We'll use libcurl to make HTTP requests
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        // Initialize curl
        curl = curl_easy_init();
        if (curl) {
            const std::string url = "https://finance.google.com/finance?q=" + symbol + "&output=json";
            
            // Set URL to fetch
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            // Set callback function to receive data
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
                [](void* contents, size_t size, size_t nmemb, std::string* userp) -> size_t {
                    userp->append((char*)contents, size * nmemb);
                    return size * nmemb;
                });
            
            // Set data buffer to write to
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            
            // Set user agent to avoid being blocked
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
            
            // Perform the request
            res = curl_easy_perform(curl);
            
            // Check for errors
            if (res != CURLE_OK) {
                handleError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
                return "";
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
    void handleError(const std::string& errorMessage);
    // Private method to validate the stock symbol
    bool isValidSymbol(const std::string& symbol) const;
    // Private method to validate the exchange name
    bool isValidExchange(const std::string& exchange) const;
public:
    // Creates a new Quote instance
    Quote(std::string symbol = "", std::string exchange = "NYSE") {
        this->symbol = symbol;
        this->exchange = exchange;
    };

    // Cleans up resources
    ~Quote();

    // Gets quote data for the specified stock symbol
    void fetchQuote(const std::string& symbol)
    {
        if (!isValidSymbol(symbol)) {
            handleError("Invalid stock symbol: " + symbol);
            return;
        }
        this->symbol = symbol;
        std::string data = fetchDataFromGoogleFinance(symbol, exchange);
        if (!data.empty()) {
            parseData(data);
            displayStockInfo();
        } else {
            handleError("Failed to fetch data for symbol: " + symbol + " on exchange: " + exchange);
        }
    }

    // Shows the retrieved stock information
    void displayQuote();
};
}



