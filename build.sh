#!/bin/bash
set -e

echo "=== premflow Developer Build ==="
echo

echo "→ Cleaning previous build..."
make clean

echo
echo "→ Running all tests..."
make test

echo
echo "→ Building premflow..."
make

echo
echo "✅ Build successful!"
echo
echo "You can now run:"
echo "  ./premflow"
echo "  make install          # Install to ~/.local/bin"
echo "  make install PREFIX=/usr/local   # System-wide (requires sudo)"