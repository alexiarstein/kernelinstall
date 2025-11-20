#ifndef DEBIAN_H
#define DEBIAN_H

#include "common.h"

void debian_install_dependencies() {
    run("sudo apt update && sudo apt install -y "
        "build-essential libncurses-dev bison flex libssl-dev libelf-dev "
        "bc wget tar xz-utils gettext libc6-dev fakeroot curl git debhelper libdw-dev rsync locales");
}

void debian_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    char source_dir[512];
    
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);
    
    snprintf(cmd, sizeof(cmd),
             "cd %s && fakeroot make -j$(nproc) bindeb-pkg",
             source_dir);
    run_build_with_progress(cmd, source_dir);
    
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb linux-headers-%s*%s*.deb",
             home, version, tag, version, tag);
    run(cmd);
}

void debian_update_bootloader() {
    run("sudo update-grub");
}

const char* debian_get_whiptail_install_cmd() {
    return "sudo apt update && sudo apt install -y whiptail";
}

DistroOperations DEBIAN_OPS = {
    .name = "Debian",
    .install_dependencies = debian_install_dependencies,
    .build_and_install = debian_build_and_install,
    .update_bootloader = debian_update_bootloader,
    .get_whiptail_install_cmd = debian_get_whiptail_install_cmd
};

#endif
