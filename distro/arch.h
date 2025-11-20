#ifndef ARCH_H
#define ARCH_H

#include "common.h"

void arch_install_dependencies() {
    run("sudo pacman -Syu --noconfirm && sudo pacman -S --noconfirm "
        "base-devel ncurses bc kmod inetutils libelf pahole cpio perl tar xz "
        "gettext git wget curl dracut grub");
}

void arch_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    
    // Compile the kernel
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && make -j$(nproc)",
             home, version);
    run(cmd);
    
    // Install modules
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && sudo make modules_install",
             home, version);
    run(cmd);
    
    // Install kernel
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && sudo make install",
             home, version);
    run(cmd);
    
    // Generate initramfs with dracut
    printf(_("Generating initramfs with dracut for Arch Linux...\n"));
    char full_version[128];
    snprintf(full_version, sizeof(full_version), "%s%s", version, tag);
    
    snprintf(cmd, sizeof(cmd),
             "sudo dracut --force --hostonly /boot/initramfs-%s.img %s",
             full_version, full_version);
    run(cmd);
}

void arch_update_bootloader() {
    printf(_("Updating GRUB for Arch Linux...\n"));
    run("sudo grub-mkconfig -o /boot/grub/grub.cfg");
}

void arch_update_initramfs(const char* version) {
    char cmd[512];
    printf(_("Regenerating initramfs with dracut...\n"));
    snprintf(cmd, sizeof(cmd), 
             "sudo dracut --force --hostonly /boot/initramfs-%s.img %s", 
             version, version);
    run(cmd);
}

const char* arch_get_whiptail_install_cmd() {
    return "sudo pacman -S --noconfirm libnewt";
}

DistroOperations ARCH_OPS = {
    .name = "Arch Linux (Manjaro/EndeavourOS/CachyOS)",
    .install_dependencies = arch_install_dependencies,
    .build_and_install = arch_build_and_install,
    .update_bootloader = arch_update_bootloader,
    .update_initramfs = arch_update_initramfs,
    .get_whiptail_install_cmd = arch_get_whiptail_install_cmd
};

#endif
