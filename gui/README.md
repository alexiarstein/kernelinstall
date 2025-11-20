# Kernel Installer GUI

## Description

Graphical interface in Python/GTK3 for Alexia Michelle's Kernel Installer.

## Requirements

### System dependencies

```bash
sudo apt install python3 python3-gi gir1.2-gtk-3.0 gir1.2-vte-2.91
```

### Python packages

No additional Python packages are required, only system libraries.

## Usage

### Run the GUI

```bash
cd gui
./run_gui.sh
```

Or directly with Python:

```bash
python3 kernel_installer_gui.py
```

## Features

- ✅ Automatic distribution detection (Debian, Ubuntu, Mint, Soplos, Arch, Manjaro, EndeavourOS, CachyOS, Garuda, Fedora, Rocky, Alma, CentOS, RHEL)
- ✅ Automatic initramfs system detection (dracut, initramfs-tools, mkinitcpio)
- ✅ Integrated VTE terminal to view progress in real time
- ✅ Animated progress bar
- ✅ Multi-language support (English and Spanish)
- ✅ Intuitive and modern interface

## Screenshots

The GUI shows:
- Current system information
- Detected distribution
- Initramfs system in use
- Currently installed kernel
- Terminal with installation progress
- Controls to start/cancel/close

## Development

### Code structure

- `kernel_installer_gui.py` - Main GTK3 application
- `run_gui.sh` - Launcher script with dependency checking

### Extend functionality

The `KernelInstallerGUI` class can be extended to add:
- Selection of specific kernel version
- Compilation options configuration
- Custom configuration profiles
- Installation history

## Authors

- **Sergi Perich** <info@soploslinux.com> - GUI Development - Soplos Linux
- **Alexia Michelle** <alexia@goldendoglinux.org> - Original Kernel Installer

## License

GNU GPL 3.0 (same as the main project)
