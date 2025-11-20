#ifndef DEBIAN_H
#define DEBIAN_H

#include "common.h"

void debian_install_dependencies() {
    run("sudo apt update && sudo apt install -y "
        "build-essential libncurses-dev bison flex libssl-dev libelf-dev "
        "bc wget tar xz-utils gettext libc6-dev fakeroot curl git debhelper libdw-dev rsync locales dracut");
}

void debian_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && fakeroot make -j$(nproc) bindeb-pkg",
             home, version);
    run(cmd);
    
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb linux-headers-%s*%s*.deb",
             home, version, tag, version, tag);
    run(cmd);
}

void debian_update_bootloader() {
    run("sudo update-grub");
}

void debian_update_initramfs(const char* version) {
    char cmd[512];
    // Detect which system is available
    if (system("which dracut > /dev/null 2>&1") == 0) {
        printf("Using dracut to generate initramfs...\n");
        snprintf(cmd, sizeof(cmd), "sudo dracut --force /boot/initramfs-%s.img %s", version, version);
    } else if (system("which update-initramfs > /dev/null 2>&1") == 0) {
        printf("Using initramfs-tools to generate initramfs...\n");
        snprintf(cmd, sizeof(cmd), "sudo update-initramfs -c -k %s", version);
    } else {
        printf("Warning: No initramfs tool found. Skipping initramfs generation.\n");
        return;
    }
    run(cmd);
}

const char* debian_get_whiptail_install_cmd() {
    return "sudo apt update && sudo apt install -y whiptail";
}

DistroOperations DEBIAN_OPS = {
    .name = "Debian",
    .install_dependencies = debian_install_dependencies,
    .build_and_install = debian_build_and_install,
    .update_bootloader = debian_update_bootloader,
    .update_initramfs = debian_update_initramfs,
    .get_whiptail_install_cmd = debian_get_whiptail_install_cmd
};

#endif
