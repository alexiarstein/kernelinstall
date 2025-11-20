#!/bin/bash
# Kernel Installer by Alexia Michelle <alexia@goldendoglinux.org>
# License GNU GPL 3.0 (See License for more Information)

# This is a script for those who wish to compile the program. If you don't want to compile it, you can simply run
# ./kernel-installer

detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    else
        echo "unknown"
    fi
}

install_dependencies() {
    local distro=$(detect_distro)
    echo "Detected distribution: $distro"
    echo "Installing compilation dependencies..."
    
    case $distro in
        debian|ubuntu|linuxmint|pop|elementary|zorin|kali|raspbian|soplos|mx)
            # Debian-based distributions
            sudo apt update
            sudo apt install -y build-essential gcc make gettext linux-libc-dev libc6-dev \
                python3 python3-gi gir1.2-gtk-3.0 gir1.2-vte-2.91
            ;;
        arch|manjaro|endeavouros|cachyos|garuda|artix)
            # Arch-based distributions
            sudo pacman -Syu --noconfirm
            sudo pacman -S --noconfirm base-devel gcc make gettext python python-gobject gtk3 vte3
            ;;
        fedora|rhel|centos|rocky|almalinux|ol)
            # RHEL-based distributions
            sudo dnf update -y
            sudo dnf install -y gcc make gettext-devel glibc-devel kernel-headers \
                python3 python3-gobject gtk3 vte291
            ;;
        opensuse*|sles)
            # openSUSE
            sudo zypper refresh
            sudo zypper install -y gcc make gettext-tools glibc-devel linux-glibc-devel \
                python3 python3-gobject gtk3 vte
            ;;
        gentoo)
            # Gentoo
            sudo emerge --ask=n sys-devel/gcc sys-devel/make sys-devel/gettext \
                dev-python/pygobject x11-libs/gtk+ x11-libs/vte
            ;;
        *)
            echo "⚠ Warning: Distribution not automatically recognized: $distro"
            echo "Please install manually:"
            echo "  - GCC, Make, Gettext"
            echo "  - Linux headers (kernel-headers or linux-libc-dev)"
            echo "  - Python3, PyGObject, GTK3, VTE"
            echo ""
            read -p "Do you want to continue anyway? [y/N] " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                exit 1
            fi
            ;;
    esac
}

compile() {
    install_dependencies
    
    echo ""
    echo "Compiling..."
    make clean
    make
    
    echo ""
    echo "Installing..."
    sudo make install
    
    echo ""
    echo "✓ Installation complete!"
    echo ""
    echo "How do you want to run the Kernel Installer?"
    echo "1) Terminal (CLI)"
    echo "2) Graphical Interface (GUI)"
    echo -n "Choose an option [1/2]: "
    read mode
    
    case $mode in
        1)
            echo ""
            echo "Starting Kernel Installer in terminal mode..."
            kernel-installer
            ;;
        2)
            echo ""
            echo "Starting Kernel Installer in GUI mode..."
            kernel-installer-gui &
            echo ""
            echo "GUI launched. You can close this terminal."
            sleep 2
            exit 0
            ;;
        *)
            echo ""
            echo "Invalid option. You can run manually:"
            echo "  - Terminal: kernel-installer"
            echo "  - GUI:      kernel-installer-gui"
            ;;
    esac
}


echo -e "\nThis is a script for those who wish to compile the program.\nIf you don't want to compile it, you can simply run ./kernel-installer.\nIt also requires sudo. If your system doesn't have sudo, run it as root\n\nDo you want to compile anyway? [Y/N]"
read i

case $i in

Y|y|S|s)
compile
;;
N|n)
echo -e "\nAborting."
;;
*)
echo "Incorrect option"
;;
esac
