# Quote 📈

Fast, real-time stock quotes in your terminal. Built with C++ for speed and reliability.

## Installation

### Package Managers (Recommended)

**Homebrew (macOS):**
```bash
brew tap Khurdhula-Harshavardhan/quote
brew install quote
```

**Coming soon:**
- Debian/Ubuntu: `sudo apt install quote`
- Arch Linux: `yay -S quote` (AUR)
- Snap: `sudo snap install quote`

### Pre-built Binaries

Download from [GitHub Releases](https://github.com/Khurdhula-Harshavardhan/quote/releases):
```bash
# Linux
curl -L https://github.com/Khurdhula-Harshavardhan/quote/releases/latest/download/quote-linux-x86_64.tar.gz | tar xz
sudo mv quote-linux-x86_64/quote /usr/local/bin/

# macOS (Apple Silicon)
curl -L https://github.com/Khurdhula-Harshavardhan/quote/releases/latest/download/quote-macos-arm64.tar.gz | tar xz
sudo mv quote-macos-arm64/quote /usr/local/bin/

# macOS (Intel)
curl -L https://github.com/Khurdhula-Harshavardhan/quote/releases/latest/download/quote-macos-x86_64.tar.gz | tar xz
sudo mv quote-macos-x86_64/quote /usr/local/bin/
```

### Build from Source

**macOS:**
```bash
# Install dependencies
xcode-select --install

# Build and install
bash install.sh
```

**Linux (Ubuntu/Debian):**
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev

# Build and install
bash install.sh
```

**Linux (Fedora/RHEL):**
```bash
# Install dependencies
sudo dnf install gcc-c++ make libcurl-devel

# Build and install
bash install.sh
```

**Manual build:**
```bash
make              # Build the binary
make install      # Install to system
make check-deps   # Check if dependencies are installed
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
