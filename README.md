# kernelinstall
Downloads, configures, compiles and installs the latest stable kernel from kernel.org

## Running it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```./kernel-installer```

## Running the GUI:

 * 1 Install dependencies: ```sudo apt install python3 python3-gi gir1.2-gtk-3.0 gir1.2-vte-2.91```
 * 2 ```cd gui```
 * 3 ```./run_gui.sh```

## Compiling it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```make && sudo make install```
 * 4 run ```kernel-installer```

When compiling from source, be sure to have the required packages installed (build-essential, gcc, make, gettext, linux-libc-dev, libc6-dev) as these are required. 

**Quick install of dependencies:**
```bash
sudo apt update
sudo apt install -y build-essential gcc make gettext linux-libc-dev libc6-dev
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


This is a simple tool that fetches the latest stable kernel release from [kernel.org](https://kernel.org)
configures it for debian, compiles and installs it.

For now it only works on debian or distros that use debian as their upstream.

**New features:**
 * 🎉 **GUI Interface** with Python/GTK3 (see `gui/` folder)
 * 🎉 **Dracut Support** for systems like Soplos Linux that don't use initramfs-tools
 * 🎉 **Automatic initramfs detection** (dracut, initramfs-tools, mkinitcpio)


It needs testing on ubuntu, mint, elementary or some of those. Please create an issue if you are willing to test with the results.

Thanks!

-- Alexia.

## DONATE / SUPPORT THE PROJECT

If you want to support these free and independent developments and buy me a coffee, you can make a private donation with the following QR.

![QR Binance](https://github.com/alexiarstein/kernelinstall.github.io/blob/main/IMG_3213.png)

Since donations are anonymous (I don't know who donates what or see your personal information), you can open an issue in the project above with the title "donation" and I'll add you to the donors list (Optional)


