# Contributing to Quote

Thank you for your interest in contributing to Quote!

## Development Setup

### Prerequisites
- C++ compiler (g++ or clang)
- libcurl development libraries
- Make

### Building from Source

```bash
# Install dependencies
# macOS
xcode-select --install

# Linux (Ubuntu/Debian)
sudo apt-get install build-essential libcurl4-openssl-dev

# Build
make

# Test
./quote -s AAPL
```

## Making Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly on your platform
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Code Style

- Follow existing code formatting
- Keep functions focused and concise
- Add meaningful variable names
- Test on both Linux and macOS when possible

## Testing

Before submitting a PR:

```bash
# Test single fetch
./quote -s AAPL

# Test watch mode
./quote -s TSLA -w

# Test international exchanges
./quote -s SHOP -e TO

# Test error handling
./quote -s INVALID
```

## Reporting Issues

Please include:
- Operating system and version
- Steps to reproduce
- Expected vs actual behavior
- Error messages (if any)

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
