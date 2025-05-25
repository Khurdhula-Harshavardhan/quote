# Quote 📈

Fast, real-time stock quotes in your terminal. Built with C++ for speed and reliability.

## Quick Start

### Install Dependencies
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
```

### Build & Install
```bash
bash install.sh
```

### Usage
```bash
# US stocks
quote -s TSLA
quote -s AAPL
quote -s GOOGL

# International stocks
quote -s SHOP -e TO    # Toronto Stock Exchange
quote -s VOD -e L      # London Stock Exchange
```

## Features
- ⚡ **Lightning Fast** - C++ performance beats Python/Node.js tools
- 📊 **Beautiful Dashboard** - Clean ASCII charts and formatted data
- 🌍 **Global Markets** - Support for major international exchanges
- 💰 **Real-time Data** - Live prices from Yahoo Finance
- 🖥️ **Terminal Native** - No GUI needed, works over SSH
- 📱 **Lightweight** - Single binary, minimal dependencies

## Why C++?
- **10x faster** than interpreted languages
- **Low memory usage** - perfect for servers
- **Single binary** - no runtime dependencies
- **Always works** - compiled code is reliable

Perfect for traders, developers, and anyone who wants quick stock data without opening a browser.
