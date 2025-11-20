#ifndef FEDORA_H
#define FEDORA_H

#include "common.h"

void fedora_install_dependencies() {
    run("sudo dnf update -y && sudo dnf install -y "
        "gcc make ncurses-devel bc openssl-devel elfutils-libelf-devel "
        "bison flex perl wget tar xz git gettext dwarves rpm-build "
        "dracut grub2-tools kernel-devel");
}

void fedora_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    
    // Compile the kernel using binrpm-pkg
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && make -j$(nproc) binrpm-pkg",
             home, version);
    run(cmd);
    
    // Install generated RPM packages
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo rpm -ivh --force kernel-%s*.rpm",
             home, version);
    run(cmd);
    
    // Generate initramfs with dracut
    printf(_("Generating initramfs with dracut for Fedora...\n"));
    char full_version[128];
    snprintf(full_version, sizeof(full_version), "%s%s", version, tag);
    
    snprintf(cmd, sizeof(cmd),
             "sudo dracut --force --hostonly /boot/initramfs-%s.img %s",
             full_version, full_version);
    run(cmd);
}

void fedora_update_bootloader() {
    printf(_("Updating GRUB2 for Fedora...\n"));
    run("sudo grub2-mkconfig -o /boot/grub2/grub.cfg");
}

void fedora_update_initramfs(const char* version) {
    char cmd[512];
    printf(_("Regenerating initramfs with dracut...\n"));
    snprintf(cmd, sizeof(cmd), 
             "sudo dracut --force --hostonly /boot/initramfs-%s.img %s", 
             version, version);
    run(cmd);
}

const char* fedora_get_whiptail_install_cmd() {
    return "sudo dnf install -y newt";
}

DistroOperations FEDORA_OPS = {
    .name = "Fedora/RHEL (Rocky/Alma/CentOS)",
    .install_dependencies = fedora_install_dependencies,
    .build_and_install = fedora_build_and_install,
    .update_bootloader = fedora_update_bootloader,
    .update_initramfs = fedora_update_initramfs,
    .get_whiptail_install_cmd = fedora_get_whiptail_install_cmd
};

#endif
