#include "quote.h"
#include <iostream>
#include <string>
#include <cstring>

int main(int argc, char* argv[]) {
    std::string symbol = "";
    std::string exchange = "";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symbol") == 0) {
            if (i + 1 < argc) {
                symbol = argv[++i];
            }
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--exchange") == 0) {
            if (i + 1 < argc) {
                exchange = argv[++i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -s, --symbol SYMBOL      Specify the stock symbol\n"
                      << "  -e, --exchange EXCHANGE  Specify the exchange\n"
                      << "  -h, --help               Show this help message\n";
            return 0;
        }
    }
    
    std::cout << "Fetching quote for " << symbol << " on exchange " << exchange << std::endl;

    quote::Quote q(symbol, exchange);
    std::string data = q.fetchDataFromGoogleFinance(symbol, exchange);
    if (!data.empty()) {
        q.parseData(data);
        q.displayStockInfo();
    } else {
        std::cerr << "Failed to fetch data for symbol: " << symbol << " on exchange: " << exchange << std::endl;
    }
    q.displayQuote();
    std::cout << "Quote fetched successfully." << std::endl;
    return 0;
}