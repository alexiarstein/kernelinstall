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
 * ~~Arch/Manjaro/EndeavourOS/CachyOS~~: (no explicit support, always up-to-date)
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
*Arch Linux and derivatives are rolling release and always up-to-date. Explicit support has been removed as of November 2025.*

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
 * 🎉 **GUI Interface** with Python/GTK3 - Modern graphical interface with real-time progress bar (shows real progress, not just animation)
 * 🎉 **Multi-Distribution Support** - Works on Debian, Ubuntu, Mint, Arch, Manjaro, Fedora, RHEL, Soplos and more
 * 🎉 **Automatic Package Manager Detection** - Uses apt, pacman, dnf, zypper or emerge as needed
 * 🎉 **Dracut Support** - Full support for dracut initramfs systems (Soplos, Arch, Fedora)
 * 🎉 **Automatic initramfs detection** - Detects and uses dracut, initramfs-tools, or mkinitcpio
 * 🎉 **Interactive Compilation** - Choose between CLI and GUI mode after compilation
 * 🎉 **Process Control** - Cancel button in GUI safely terminates the entire installation process group (no orphaned processes)
 * 🎉 **Dependency Robustness** - On Soplos, dependencies are always reinstalled to avoid missing headers
 * 🎉 **Internationalization** - Full i18n support (English and Spanish)
 * 🎉 **Security** - All process management and signals in the GUI are handled safely (no direct os.kill, only process groups)
 * 🎉 **Modular Architecture** - Each distribution has its own header and logic, easy to extend

## Testing


### Technical improvements (2025):
- Modular C code: each distro in its own header (see `distro/`)
- GUI progress bar now shows real progress (not just animation)
- GUI cancel button kills the full process group (safe, no zombies)
- All code and comments in English, only translations in .po/.mo
- Compile script for Soplos/apt now uses `--reinstall` to avoid missing headers
- Internationalization: 100% English/Spanish, easy to add more
- Security: no unsafe signal handling, no orphaned processes

This installer has been tested on multiple distributions. If you test it on Ubuntu, Mint, Elementary, Soplos or other distributions, please create an issue with your results to help improve compatibility.

**Note:** The script `test_sandbox.sh` has been removed as it is no longer necessary.

Thanks!

-- Alexia.

## DONATE / SUPPORT THE PROJECT

If you want to support these free and independent developments and buy me a coffee, you can make a private donation with the following QR.

![QR Binance](https://github.com/alexiarstein/kernelinstall.github.io/blob/main/IMG_3213.png)

Since donations are anonymous (I don't know who donates what or see your personal information), you can open an issue in the project above with the title "donation" and I'll add you to the donors list (Optional)


