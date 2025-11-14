#!/bin/bash
set -e

VERSION="0.0.3"
ARCH="amd64"
PKG_NAME="quote_${VERSION}_${ARCH}"

# Create package structure
mkdir -p ${PKG_NAME}/DEBIAN
mkdir -p ${PKG_NAME}/usr/local/bin
mkdir -p ${PKG_NAME}/usr/share/doc/quote

# Build the binary
make clean
make

# Copy files
cp quote ${PKG_NAME}/usr/local/bin/
cp README.md ${PKG_NAME}/usr/share/doc/quote/
cp LICENSE ${PKG_NAME}/usr/share/doc/quote/

# Create control file
cat > ${PKG_NAME}/DEBIAN/control <<EOF
Package: quote
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Depends: libcurl4
Maintainer: Harshavardhan Khurdhula <your-email@example.com>
Description: Fast, real-time stock quotes in your terminal
 Quote is a lightweight C++ tool for fetching real-time stock market data
 directly in your terminal. It supports multiple international exchanges
 and features a watch mode for continuous monitoring.
EOF

# Set permissions
chmod 755 ${PKG_NAME}/usr/local/bin/quote
chmod 644 ${PKG_NAME}/DEBIAN/control

# Build the package
dpkg-deb --build ${PKG_NAME}

echo "Package built: ${PKG_NAME}.deb"
echo "Install with: sudo dpkg -i ${PKG_NAME}.deb"
