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

# Watch mode - continuous updates (like `watch nvidia-smi`)
quote -s AAPL -w        # Update every 2 seconds (default)
quote -s TSLA -w -i 1   # Update every 1 second
quote -s MSFT -w -i 5   # Update every 5 seconds
```

## Features
- ⚡ **Lightning Fast** - C++ performance beats Python/Node.js tools
- 📊 **Beautiful Dashboard** - Clean ASCII charts and formatted data
- 🌍 **Global Markets** - Support for major international exchanges
- 💰 **Real-time Data** - Live prices from Yahoo Finance
- 🖥️ **Terminal Native** - No GUI needed, works over SSH
- 📱 **Lightweight** - Single binary, minimal dependencies
- 👀 **Watch Mode** - Continuous monitoring like `watch nvidia-smi`

## Why C++?
- **10x faster** than interpreted languages
- **Low memory usage** - perfect for servers
- **Single binary** - no runtime dependencies
- **Always works** - compiled code is reliable

Perfect for traders, developers, and anyone who wants quick stock data without opening a browser.
