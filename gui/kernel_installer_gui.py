#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Kernel Installer GUI - Python GTK3 Interface
Authors: 
    Sergi Perich <info@soploslinux.com> - GUI Development
    Alexia Michelle <alexia@goldendoglinux.org> - Original Kernel Installer
License: GNU GPL 3.0

GUI frontend in Python/GTK3 for the Kernel Installer (written in C)
"""

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
from gi.repository import Gtk, GLib, Vte, Pango
import subprocess
import threading
import os
import sys
import locale
import gettext

# Configure internationalization
locale.setlocale(locale.LC_ALL, '')
gettext.bindtextdomain('kernel-install', '/usr/local/share/locale')
gettext.textdomain('kernel-install')
_ = gettext.gettext

class KernelInstallerGUI(Gtk.Window):
    def __init__(self):
        super().__init__(title=_("Alexia Kernel Installer"))
        self.set_default_size(800, 600)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_border_width(10)
        
        # Process variable
        self.process = None
        self.installation_running = False
        
        # Create the interface
        self.create_ui()
        
    def create_ui(self):
        """Create the user interface"""
        # Main vertical box
        main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.add(main_box)
        
        # Header with title and version
        header_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        
        title_label = Gtk.Label()
        title_label.set_markup('<span size="x-large" weight="bold">🐕 Alexia Kernel Installer</span>')
        header_box.pack_start(title_label, False, False, 0)
        
        subtitle_label = Gtk.Label()
        subtitle_label.set_markup('<span size="small">'+_("Download, compile and install the latest stable kernel from kernel.org")+'</span>')
        subtitle_label.set_line_wrap(True)
        header_box.pack_start(subtitle_label, False, False, 0)
        
        separator = Gtk.Separator(orientation=Gtk.Orientation.HORIZONTAL)
        header_box.pack_start(separator, False, False, 5)
        
        main_box.pack_start(header_box, False, False, 0)
        
        # System information
        info_frame = Gtk.Frame(label=_("System Information"))
        info_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        info_box.set_border_width(10)
        
        self.distro_label = Gtk.Label(label=_("Detecting distribution..."))
        self.distro_label.set_halign(Gtk.Align.START)
        info_box.pack_start(self.distro_label, False, False, 0)
        
        self.initramfs_label = Gtk.Label(label=_("Detecting initramfs system..."))
        self.initramfs_label.set_halign(Gtk.Align.START)
        info_box.pack_start(self.initramfs_label, False, False, 0)
        
        self.kernel_label = Gtk.Label(label=_("Current kernel: ") + self.get_current_kernel())
        self.kernel_label.set_halign(Gtk.Align.START)
        info_box.pack_start(self.kernel_label, False, False, 0)
        
        info_frame.add(info_box)
        main_box.pack_start(info_frame, False, False, 0)
        
        # Terminal VTE para mostrar el progreso
        terminal_frame = Gtk.Frame(label=_("Installation Progress"))
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        
        self.terminal = Vte.Terminal()
        self.terminal.set_font(Pango.FontDescription("Monospace 10"))
        self.terminal.set_scroll_on_output(True)
        self.terminal.set_scrollback_lines(10000)
        
        scroll.add(self.terminal)
        terminal_frame.add(scroll)
        main_box.pack_start(terminal_frame, True, True, 0)
        
        # Barra de progreso
        self.progress_bar = Gtk.ProgressBar()
        self.progress_bar.set_show_text(True)
        self.progress_bar.set_text(_("Ready"))
        main_box.pack_start(self.progress_bar, False, False, 0)
        
        # Action buttons
        button_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        button_box.set_halign(Gtk.Align.END)
        
        self.start_button = Gtk.Button(label=_("Start Installation"))
        self.start_button.connect("clicked", self.on_start_clicked)
        button_box.pack_start(self.start_button, False, False, 0)
        
        self.cancel_button = Gtk.Button(label=_("Cancel"))
        self.cancel_button.set_sensitive(False)
        self.cancel_button.connect("clicked", self.on_cancel_clicked)
        button_box.pack_start(self.cancel_button, False, False, 0)
        
        self.close_button = Gtk.Button(label=_("Close"))
        self.close_button.connect("clicked", self.on_close_clicked)
        button_box.pack_start(self.close_button, False, False, 0)
        
        main_box.pack_start(button_box, False, False, 0)
        
        # Detect system on startup
        GLib.idle_add(self.detect_system)
        
    def get_current_kernel(self):
        """Get current kernel version"""
        try:
            result = subprocess.run(['uname', '-r'], capture_output=True, text=True)
            return result.stdout.strip()
        except:
            return _("Unknown")
    
    def detect_system(self):
        """Detect distribution and initramfs system"""
        # Detect distribution
        try:
            with open('/etc/os-release', 'r') as f:
                content = f.read()
                if 'ID=soplos' in content:
                    distro = "Soplos Linux"
                elif 'ID=debian' in content:
                    distro = "Debian"
                elif 'ID=linuxmint' in content:
                    distro = "Linux Mint"
                elif 'ID=ubuntu' in content:
                    distro = "Ubuntu"
                elif 'ID=arch' in content:
                    distro = "Arch Linux"
                elif 'ID=manjaro' in content:
                    distro = "Manjaro"
                elif 'ID=endeavouros' in content:
                    distro = "EndeavourOS"
                elif 'ID=cachyos' in content:
                    distro = "CachyOS"
                elif 'ID=garuda' in content:
                    distro = "Garuda Linux"
                elif 'ID=fedora' in content:
                    distro = "Fedora"
                elif 'ID=rocky' in content:
                    distro = "Rocky Linux"
                elif 'ID=almalinux' in content:
                    distro = "AlmaLinux"
                elif 'ID=centos' in content:
                    distro = "CentOS"
                elif 'ID=rhel' in content:
                    distro = "Red Hat Enterprise Linux"
                else:
                    distro = _("Unknown")
                
                self.distro_label.set_text(_("Distribution: ") + distro)
        except:
            self.distro_label.set_text(_("Distribution: Unknown"))
        
        # Detect initramfs
        if subprocess.run(['which', 'dracut'], capture_output=True).returncode == 0:
            initramfs = "dracut"
        elif subprocess.run(['which', 'update-initramfs'], capture_output=True).returncode == 0:
            initramfs = "initramfs-tools"
        elif subprocess.run(['which', 'mkinitcpio'], capture_output=True).returncode == 0:
            initramfs = "mkinitcpio"
        else:
            initramfs = _("Not detected")
        
        self.initramfs_label.set_text(_("Initramfs system: ") + initramfs)
        
        return False
    
    def on_start_clicked(self, button):
        """Start kernel installation"""
        # Confirm before starting
        dialog = Gtk.MessageDialog(
            transient_for=self,
            flags=0,
            message_type=Gtk.MessageType.WARNING,
            buttons=Gtk.ButtonsType.YES_NO,
            text=_("Start Kernel Installation?")
        )
        dialog.format_secondary_text(
            _("This process will download, compile and install the latest stable kernel.\n"
              "It may take several hours depending on your system.\n\n"
              "Do you want to continue?")
        )
        
        response = dialog.run()
        dialog.destroy()
        
        if response == Gtk.ResponseType.YES:
            self.start_installation()
    
    def start_installation(self):
        """Run the installer in the terminal"""
        self.installation_running = True
        self.start_button.set_sensitive(False)
        self.cancel_button.set_sensitive(True)
        self.progress_bar.set_text(_("Installing..."))
        self.progress_bar.pulse()
        
        # Start progress bar animation
        GLib.timeout_add(100, self.pulse_progress)
        
        # Run the C installer
        installer_path = self.find_installer()
        if installer_path:
            try:
                self.terminal.spawn_async(
                    Vte.PtyFlags.DEFAULT,
                    os.getcwd(),
                    [installer_path],
                    None,
                    GLib.SpawnFlags.DEFAULT,
                    None,
                    None,
                    -1,
                    None,
                    None,
                    None
                )
            except Exception as e:
                self.write_to_terminal(_("Error launching installer: {}\n").format(str(e)))
                self.installation_running = False
                self.start_button.set_sensitive(True)
                self.cancel_button.set_sensitive(False)
        else:
            self.write_to_terminal(_("Error: kernel-installer not found!\n"))
            self.installation_running = False
            self.start_button.set_sensitive(True)
            self.cancel_button.set_sensitive(False)
    
    def find_installer(self):
        """Find the installer executable"""
        paths = [
            './kernel-installer',
            '/usr/local/bin/kernel-installer',
            '/usr/bin/kernel-installer'
        ]
        
        for path in paths:
            if os.path.isfile(path) and os.access(path, os.X_OK):
                return path
        return None
    
    def write_to_terminal(self, text):
        """Write text to the terminal"""
        self.terminal.feed(text.encode('utf-8'))
    
    def pulse_progress(self):
        """Animate the progress bar"""
        if self.installation_running:
            self.progress_bar.pulse()
            return True
        return False
    
    def on_cancel_clicked(self, button):
        """Cancel the installation"""
        dialog = Gtk.MessageDialog(
            transient_for=self,
            flags=0,
            message_type=Gtk.MessageType.WARNING,
            buttons=Gtk.ButtonsType.YES_NO,
            text=_("Cancel Installation?")
        )
        dialog.format_secondary_text(_("Are you sure you want to cancel the installation?"))
        
        response = dialog.run()
        dialog.destroy()
        
        if response == Gtk.ResponseType.YES:
            # Try to terminate the process
            self.installation_running = False
            self.start_button.set_sensitive(True)
            self.cancel_button.set_sensitive(False)
            self.progress_bar.set_text(_("Cancelled"))
    
    def on_close_clicked(self, button):
        """Close the application"""
        if self.installation_running:
            dialog = Gtk.MessageDialog(
                transient_for=self,
                flags=0,
                message_type=Gtk.MessageType.WARNING,
                buttons=Gtk.ButtonsType.YES_NO,
                text=_("Installation in progress")
            )
            dialog.format_secondary_text(_("An installation is in progress. Are you sure you want to exit?"))
            
            response = dialog.run()
            dialog.destroy()
            
            if response == Gtk.ResponseType.NO:
                return
        
        Gtk.main_quit()

def main():
    """Main function"""
    # Check if running as root (not necessary in GUI)
    if os.geteuid() == 0:
        print(_("Warning: Running GUI as root is not recommended."))
    
    app = KernelInstallerGUI()
    app.connect("destroy", Gtk.main_quit)
    app.show_all()
    Gtk.main()

if __name__ == '__main__':
    main()
