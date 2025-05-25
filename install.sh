#!/bin/bash

# Compile the program
g++ -std=c++17 -Wall -o quote quote.cpp

# Install to system
if [ "$EUID" -eq 0 ]; then
    # Running as root
    cp quote /usr/local/bin/
    chmod +x /usr/local/bin/quote
    echo "Installed quote to /usr/local/bin/"
else
    # Running as user
    mkdir -p ~/.local/bin
    cp quote ~/.local/bin/
    chmod +x ~/.local/bin/quote
    echo "Installed quote to ~/.local/bin/"
    echo "Make sure ~/.local/bin is in your PATH"
    echo "Now you can run 'quote' from anywhere using: quote -s 'symbol' -e 'exchange'"
fi