#!/bin/bash
# Kernel Installer - Sandbox Test Mode
# Author: Alexia Michelle <alexia@goldendoglinux.org>
# This script simulates the installation without modifying the real system

set -e

SANDBOX_DIR="$HOME/kernel_install_sandbox"
TEST_LOG="$SANDBOX_DIR/test.log"

echo "==================================="
echo "  Kernel Installer - SANDBOX MODE"
echo "==================================="
echo ""

# Create sandbox directory
mkdir -p "$SANDBOX_DIR"
echo "[$(date)] Starting sandbox test" > "$TEST_LOG"

echo "✓ Sandbox directory created: $SANDBOX_DIR"

# Simulate distribution detection
echo ""
echo "📋 Detecting system..."
if [ -f /etc/os-release ]; then
    . /etc/os-release
    echo "   Distribution: $NAME"
    echo "   ID: $ID"
    echo "   Version: $VERSION_ID"
    echo "[$(date)] Distribution detected: $ID" >> "$TEST_LOG"
fi

# Detect initramfs
echo ""
echo "🔧 Detecting initramfs system..."
if command -v dracut &> /dev/null; then
    INITRAMFS="dracut"
    echo "   ✓ dracut found: $(dracut --version | head -1)"
elif command -v update-initramfs &> /dev/null; then
    INITRAMFS="initramfs-tools"
    echo "   ✓ initramfs-tools found"
elif command -v mkinitcpio &> /dev/null; then
    INITRAMFS="mkinitcpio"
    echo "   ✓ mkinitcpio found"
else
    INITRAMFS="none"
    echo "   ⚠ No initramfs system detected"
fi
echo "[$(date)] Initramfs system: $INITRAMFS" >> "$TEST_LOG"

# Verify current kernel
echo ""
echo "🐧 Current kernel: $(uname -r)"
echo "[$(date)] Current kernel: $(uname -r)" >> "$TEST_LOG"

# Verify C program dependencies
echo ""
echo "📦 Verifying compilation dependencies..."
DEPS_OK=true

check_dep() {
    if command -v "$1" &> /dev/null; then
        echo "   ✓ $1"
    else
        echo "   ✗ $1 (missing)"
        DEPS_OK=false
    fi
}

check_dep gcc
check_dep make
check_dep msgfmt
check_dep xgettext

# Verify GUI dependencies
echo ""
echo "🎨 Verifying GUI dependencies..."
GUI_OK=true

if command -v python3 &> /dev/null; then
    echo "   ✓ Python3: $(python3 --version)"
    
    # Verify GTK3
    if python3 -c "import gi; gi.require_version('Gtk', '3.0')" 2>/dev/null; then
        echo "   ✓ GTK3 bindings"
    else
        echo "   ✗ GTK3 bindings (install: sudo apt install python3-gi gir1.2-gtk-3.0)"
        GUI_OK=false
    fi
    
    # Verify VTE
    if python3 -c "import gi; gi.require_version('Vte', '2.91')" 2>/dev/null; then
        echo "   ✓ VTE terminal"
    else
        echo "   ✗ VTE terminal (install: sudo apt install gir1.2-vte-2.91)"
        GUI_OK=false
    fi
else
    echo "   ✗ Python3 not found"
    GUI_OK=false
fi

# Compile in sandbox
echo ""
echo "🔨 Compiling program in sandbox mode..."
cd "$(dirname "$0")"

if make clean > /dev/null 2>&1 && make > "$SANDBOX_DIR/compile.log" 2>&1; then
    echo "   ✓ Compilation successful"
    echo "[$(date)] Compilation successful" >> "$TEST_LOG"
else
    echo "   ✗ Compilation error (see $SANDBOX_DIR/compile.log)"
    echo "[$(date)] Compilation error" >> "$TEST_LOG"
    DEPS_OK=false
fi

# Verify binary was created
if [ -f "./kernel-installer" ]; then
    echo "   ✓ Executable created: ./kernel-installer"
    echo "   Size: $(du -h ./kernel-installer | cut -f1)"
else
    echo "   ✗ Could not create executable"
    DEPS_OK=false
fi

# Verify translation files
echo ""
echo "🌐 Verifying internationalization system..."
if [ -d "locale/es/LC_MESSAGES" ] && [ -f "locale/es/LC_MESSAGES/kernel-install.mo" ]; then
    echo "   ✓ Translations compiled (Spanish)"
else
    echo "   ⚠ Translations not found (run: make compile-mo)"
fi

# Simulate execution without root permissions
echo ""
echo "🧪 Execution test (dry-run mode)..."
echo ""
echo "   NOTE: To fully test the installer in sandbox,"
echo "   you would need a container or VM. This script only verifies"
echo "   that the program compiles and dependencies are present."
echo ""

# Summary
echo ""
echo "==================================="
echo "       SANDBOX SUMMARY"
echo "==================================="
echo ""

if [ "$DEPS_OK" = true ]; then
    echo "✓ C Program: READY"
else
    echo "✗ C Program: MISSING DEPENDENCIES"
fi

if [ "$GUI_OK" = true ]; then
    echo "✓ Python GUI: READY"
else
    echo "✗ Python GUI: MISSING DEPENDENCIES"
fi

echo ""
echo "Initramfs system detected: $INITRAMFS"
echo ""
echo "Sandbox files in: $SANDBOX_DIR"
echo "Test log: $TEST_LOG"
echo ""

# Offer to test GUI (without installing kernel)
if [ "$GUI_OK" = true ]; then
    echo "Do you want to test the GUI now? (interface only, won't install anything) [y/N]"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        echo ""
        echo "Starting GUI in demo mode..."
        cd gui
        python3 kernel_installer_gui.py &
        GUI_PID=$!
        echo "GUI started (PID: $GUI_PID)"
        echo ""
        echo "NOTE: The GUI will detect your system but will NOT execute the installation"
        echo "      unless you expressly start the process."
    fi
fi

echo ""
echo "Test completed. The sandbox did not modify your system."
echo ""
