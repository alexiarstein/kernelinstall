#!/bin/bash
# Kernel Installer by Alexia Michelle <alexia@goldendoglinux.org>
# License GNU GPL 3.0 (See License for more Information)

# This is a script for those who wish to compile the program. If you don't want to compile it, you can simply run
# ./kernel-installer

compile() {
# Install compilation dependencies including kernel headers
echo "Installing compilation dependencies..."
sudo apt update
sudo apt install -y build-essential gcc make gettext linux-libc-dev libc6-dev
make clean
make
sudo make install
kernel-installer
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
