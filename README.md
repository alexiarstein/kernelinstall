# kernelinstall
Downloads, configures, compiles and installs the latest stable kernel from kernel.org

## Running it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```./kernel-installer```

## Running the GUI:

 * 1 After compilation with `./compile.sh`, simply run: ```kernel-installer-gui```
 
Or from source:
 * 1 Install dependencies (see compilation section below)
 * 2 ```cd gui```
 * 3 ```./run_gui.sh```

## Compiling it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```./compile.sh``` (automatically detects your distro and installs all dependencies)
 
The `compile.sh` script automatically detects your Linux distribution and installs the required dependencies using the appropriate package manager (apt, pacman, dnf, zypper, or emerge).

**Supported package managers:**
 * **Debian/Ubuntu/Mint/Soplos**: apt
 * **Arch/Manjaro/EndeavourOS/CachyOS**: pacman
 * **Fedora/RHEL/Rocky/Alma/CentOS**: dnf
 * **openSUSE**: zypper
 * **Gentoo**: emerge

Or manually:
 * 3 ```make && sudo make install```
 * 4 run ```kernel-installer``` (terminal) or ```kernel-installer-gui``` (GUI)

**Manual dependency installation by distro:**

Debian/Ubuntu/Mint:
```bash
sudo apt update
sudo apt install -y build-essential gcc make gettext linux-libc-dev libc6-dev \
    python3 python3-gi gir1.2-gtk-3.0 gir1.2-vte-2.91
```

Arch/Manjaro/EndeavourOS:
```bash
sudo pacman -Syu --noconfirm
sudo pacman -S --noconfirm base-devel gcc make gettext python python-gobject gtk3 vte3
```

Fedora/RHEL/Rocky/Alma:
```bash
sudo dnf update -y
sudo dnf install -y gcc make gettext-devel glibc-devel kernel-headers \
    python3 python3-gobject gtk3 vte291
``` 


## Supported Distros:

### Debian-based:
 * **Soplos Linux** (with dracut support)
 * Debian 13
 * Debian 12
 * MX Linux
 * Linux Mint LMDE
 * Linux Mint
 * Ubuntu
 * ElementaryOS
 * PopOS!

### Arch-based:
 * **Arch Linux** (with dracut support)
 * **Manjaro**
 * **EndeavourOS**
 * **CachyOS**
 * **Garuda Linux**

### RHEL-based:
 * **Fedora** (with dracut support)
 * **Rocky Linux**
 * **AlmaLinux**
 * **CentOS Stream**
 * **Red Hat Enterprise Linux**

## What does it change?

Read the changelog [https://cdn.kernel.org/pub/linux/kernel/v6.x/ChangeLog-6.17.8](https://cdn.kernel.org/pub/linux/kernel/v6.x/ChangeLog-6.17.8)

## Language Support

 * English
 * Spanish

## Features

This is a powerful tool that fetches the latest stable kernel release from [kernel.org](https://kernel.org), configures it, compiles and installs it.

**New features:**
 * 🎉 **GUI Interface** with Python/GTK3 - Modern graphical interface with real-time progress
 * 🎉 **Multi-Distribution Support** - Works on Debian, Ubuntu, Mint, Arch, Manjaro, Fedora, RHEL and more
 * 🎉 **Automatic Package Manager Detection** - Automatically uses apt, pacman, dnf, zypper or emerge
 * 🎉 **Dracut Support** - Full support for dracut initramfs systems (Soplos, Arch, Fedora)
 * 🎉 **Automatic initramfs detection** - Detects and uses dracut, initramfs-tools, or mkinitcpio
 * 🎉 **Interactive Compilation** - Choose between CLI and GUI mode after compilation
 * 🎉 **Process Control** - Cancel button in GUI properly terminates installation
 * 🎉 **Multi-language** - Full internationalization support (English and Spanish)

## Testing

This installer has been tested on multiple distributions. If you test it on Ubuntu, Mint, Elementary or other distributions, please create an issue with your results to help improve compatibility.

Thanks!

-- Alexia.

## DONATE / SUPPORT THE PROJECT

If you want to support these free and independent developments and buy me a coffee, you can make a private donation with the following QR.

![QR Binance](https://github.com/alexiarstein/kernelinstall.github.io/blob/main/IMG_3213.png)

Since donations are anonymous (I don't know who donates what or see your personal information), you can open an issue in the project above with the title "donation" and I'll add you to the donors list (Optional)


