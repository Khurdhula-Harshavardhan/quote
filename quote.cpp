#include "quote.h"
#include <iostream>
#include <string>
#include <cstring>

int main(int argc, char* argv[]) {
    std::string symbol = "";
    std::string exchange = "NASDAQ"; // Set a default exchange
    
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
                      << "  -s, --symbol SYMBOL      Specify the stock symbol (required)\n"
                      << "  -e, --exchange EXCHANGE  Specify the exchange (default: NASDAQ)\n"
                      << "  -h, --help               Show this help message\n";
            return 0;
        }
    }
    
    // If no symbol provided, show help
    if (symbol.empty()) {
        std::cout << "Usage: " << argv[0] << " [options]\n"
                  << "Options:\n"
                  << "  -s, --symbol SYMBOL      Specify the stock symbol (required)\n"
                  << "  -e, --exchange EXCHANGE  Specify the exchange (default: NASDAQ)\n"
                  << "  -h, --help               Show this help message\n";
        return 1;
    }
    
    std::cout << "Fetching quote for " << symbol << " on exchange " << exchange << std::endl;

    quote::Quote q(symbol, exchange);
    q.fetchQuote(symbol);
    
    std::cout << "Quote operation completed." << std::endl;
    return 0;
}