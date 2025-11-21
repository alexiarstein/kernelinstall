#!/bin/bash
# Kernel Installer by Alexia Michelle <alexia@goldendoglinux.org>
# License GNU GPL 3.0 (See License for more Information)

# Este es un script para quienes deseen compilar el programa. Si no querés compilarlo podes simplemente correr
# ./kernel-installer

compilar() {
sudo apt install -y gcc make gettext libncurses-dev
make clean
make
sudo make install
kernel-installer
}


echo -e "\nEste es un script para quienes deseen compilar el programa.\nSi no lo querés compilar, podés simplemente ejecutar ./kernel-installer.\nTambién requiere sudo. Si tu sistema no tiene sudo, correlo como root\n\n¿Deseas Compilar de todos modos? [S/N]"
read i

case $i in

Y|y|S|s)
compilar
;;
N|n)
echo -e "\nAbortando."
;;
*)
echo "Opcion incorrecta"
;;
esac
