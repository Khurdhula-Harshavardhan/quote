#include<iostream>

namespace quote {
// A simple class for a command-line tool that fetches stock quotes from Google Finance
class Quote {
public:
    // Creates a new Quote instance
    Quote();

    // Cleans up resources
    ~Quote();

    // Gets quote data for the specified stock symbol
    void fetchQuote(const std::string& symbol);

    // Shows the retrieved stock information
    void displayQuote();
};
} 



