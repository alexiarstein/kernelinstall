#ifndef SOPLOS_H
#define SOPLOS_H

#include "common.h"

void soplos_install_dependencies() {
    run("sudo apt update && sudo apt install -y "
        "build-essential libncurses-dev bison flex libssl-dev libelf-dev "
        "bc wget tar xz-utils gettext libc6-dev fakeroot curl git debhelper libdw-dev rsync locales "
        "dracut dracut-core dracut-network");
}

void soplos_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    
    // Build kernel using bindeb-pkg
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && fakeroot make -j$(nproc) bindeb-pkg",
             home, version);
    run(cmd);
    
    // Install generated .deb packages
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb linux-headers-%s*%s*.deb",
             home, version, tag, version, tag);
    run(cmd);
    
    // Generate initramfs with dracut specifically
    printf(_("Generating initramfs with dracut for Soplos Linux...\n"));
    char full_version[128];
    snprintf(full_version, sizeof(full_version), "%s%s", version, tag);
    
    snprintf(cmd, sizeof(cmd),
             "sudo dracut --force --hostonly --hostonly-cmdline "
             "/boot/initramfs-%s.img %s",
             full_version, full_version);
    run(cmd);
}

void soplos_update_bootloader() {
    printf(_("Updating GRUB for Soplos Linux...\n"));
    run("sudo update-grub");
}

void soplos_update_initramfs(const char* version) {
    char cmd[512];
    printf(_("Regenerating initramfs with dracut...\n"));
    snprintf(cmd, sizeof(cmd), 
             "sudo dracut --force --hostonly --hostonly-cmdline "
             "/boot/initramfs-%s.img %s", 
             version, version);
    run(cmd);
}

const char* soplos_get_whiptail_install_cmd() {
    return "sudo apt update && sudo apt install -y whiptail";
}

DistroOperations SOPLOS_OPS = {
    .name = "Soplos Linux",
    .install_dependencies = soplos_install_dependencies,
    .build_and_install = soplos_build_and_install,
    .update_bootloader = soplos_update_bootloader,
    .update_initramfs = soplos_update_initramfs,
    .get_whiptail_install_cmd = soplos_get_whiptail_install_cmd
};

#endif
