#include "quote.h"
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

// Global flag for graceful shutdown
volatile bool running = true;

// Signal handler for Ctrl+C
void signalHandler(int signal) {
    running = false;
    std::cout << "\n\nShutting down gracefully...\n" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string symbol = "";
    std::string exchange = "NASDAQ"; // Set a default exchange
    bool watchMode = false;
    double refreshInterval = 0.7; // Default 700ms for watch command

    // Set up signal handler
    signal(SIGINT, signalHandler);
    
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
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--watch") == 0) {
            watchMode = true;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                refreshInterval = std::stod(argv[++i]);
                if (refreshInterval < 0.1) refreshInterval = 0.1; // Minimum 100ms
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -s, --symbol SYMBOL      Specify the stock symbol (required)\n"
                      << "  -e, --exchange EXCHANGE  Specify the exchange suffix (default: none for US stocks)\n"
                      << "                           Examples: TO (TSX), L (LSE), DE (XETRA),\n"
                      << "                           PA (Euronext), AX (ASX), HK (Hong Kong), NS (NSE)\n"
                      << "  -w, --watch              Enable continuous monitoring mode\n"
                      << "  -i, --interval SECONDS   Refresh interval for watch mode (default: 2.0 seconds, min: 0.1)\n"
                      << "  -h, --help               Show this help message\n"
                      << "\nExamples:\n"
                      << "  " << argv[0] << " -s TSLA           # Single quote fetch (US stock)\n"
                      << "  " << argv[0] << " -s SHOP -e TO     # Toronto Stock Exchange\n"
                      << "  " << argv[0] << " -s VOD -e L       # London Stock Exchange\n"
                      << "  " << argv[0] << " -s AAPL -w        # Watch mode with 2.0s real-time updates\n"
                      << "  " << argv[0] << " -s MSFT -w -i 0.1 # Ultra-fast 100ms updates\n"
                      << "  " << argv[0] << " -s TSLA -w -i 5   # Watch mode with 5s intervals\n";
            return 0;
        }
    }
    
    // If no symbol provided, show help
    if (symbol.empty()) {
        std::cout << "Usage: " << argv[0] << " [options]\n"
                  << "Options:\n"
                  << "  -s, --symbol SYMBOL      Specify the stock symbol (required)\n"
                  << "  -e, --exchange EXCHANGE  Specify the exchange suffix (default: none for US stocks)\n"
                  << "                           Examples: TO (TSX), L (LSE), DE (XETRA),\n"
                  << "                           PA (Euronext), AX (ASX), HK (Hong Kong), NS (NSE)\n"
                  << "  -w, --watch              Enable continuous monitoring mode\n"
                  << "  -i, --interval SECONDS   Refresh interval for watch mode (default: 2.0 seconds, min: 0.1)\n"
                  << "  -h, --help               Show this help message\n";
        return 1;
    }
    
    
    quote::Quote q(symbol, exchange);
    
    if (watchMode) {
        // Hide cursor for clean display
        std::cout << "\033[?25l" << std::flush;
        
        while (running) {
            // Try multiple clearing methods for maximum compatibility
            // Method 1: Use tput (most reliable)
            system("tput clear 2>/dev/null || clear 2>/dev/null || printf '\\033[2J\\033[H'");
            
            // Method 2: ANSI escape sequences as additional backup
            std::cout << "\033[2J\033[H" << std::flush;
            
            // Get current time for header
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::cout << "Every " << refreshInterval << "s: quote -s " << symbol;
            if (exchange != "NASDAQ") std::cout << " -e " << exchange;
            std::cout << "    " << std::put_time(std::localtime(&time_t), "%a %b %d %H:%M:%S %Y") << "\n";
            
            // Fetch data without displaying
            if (q.fetchQuoteData(symbol)) {
                q.displayStockInfo();
            }
            
            // Force flush all output before sleeping
            std::cout << std::flush;
            
            if (running) { // Check if we should continue before sleeping
                std::this_thread::sleep_for(std::chrono::duration<double>(refreshInterval));
            }
        }
        
        // Restore cursor and clean exit
        std::cout << "\033[?25h" << std::flush;
        std::cout << "\nMonitoring stopped.\n" << std::endl;
    } else {
        // Single fetch mode
        std::cout << "Fetching quote for " << symbol;
        if (!exchange.empty()) {
            std::cout << " on exchange " << exchange;
        }
        std::cout << std::endl;
        
        q.fetchQuote(symbol);
        
        std::cout << "Quote operation completed." << std::endl;
    }
    
    return 0;
}