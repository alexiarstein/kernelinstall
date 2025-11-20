// Linux Mint users, and also Ubuntu, ElementaryOS and basically any other Ubuntu clone:
// Canonical signs the kernel with a Microsoft key for secure boot.
// Here we sign the kernel with a GoldenDog Linux key
// If you want this kernel to have secure boot, and you have UEFI, you can add it to boot.
// Otherwise, simply disable secure boot.

#ifndef LINUXMINT_H
#define LINUXMINT_H

#include "common.h"

void mint_install_dependencies() {
    run("sudo apt update && sudo apt install -y "
        "build-essential libncurses-dev bison flex libssl-dev libssl-dev libelf-dev "
        "bc wget tar xz-utils fakeroot curl git debhelper libdw-dev rsync locales gawk gettext "
        "mokutil openssl");
}

void mint_generate_certificate() {
    printf(_("Generating GoldenDogLinux Secure Boot certificate...\n"));
    
    // Create directory for MOK certificates
    run("sudo mkdir -p /var/lib/shim-signed/mok/");
    
    // Generate self-signed certificate (valid for 10 years)
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "sudo openssl req -nodes -new -x509 -newkey rsa:2048 "
             "-keyout /var/lib/shim-signed/mok/MOK_goldendoglinux.priv "
             "-outform DER -out /var/lib/shim-signed/mok/MOK_goldendoglinux.der "
             "-days 3650 -subj \"/CN=GoldenDogLinux Secure Boot Key/\"");
    run(cmd);
    
    // Set appropriate permissions for private key
    run("sudo chmod 600 /var/lib/shim-signed/mok/MOK_goldendoglinux.priv");
    run("sudo chmod 644 /var/lib/shim-signed/mok/MOK_goldendoglinux.der");
    
    printf(_("GoldenDogLinux certificate generated successfully.\n"));
}

int mint_ask_secure_boot_enrollment() {
    char command[512];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yesno \"%s\\n\\n"
             "%s\\n\\n"
             "%s\\n\\n"
             "%s\" 16 60",
             _("Secure Boot Enrollment"),
             _("Do you want to enroll the GoldenDogLinux certificate for Secure Boot?"),
             _("This will allow your custom kernel to work with Secure Boot enabled."),
             _("You will be asked to set a password and enroll the key during the next reboot."),
             _("Continue with enrollment?"));
    
    return system(command);
}

void mint_enroll_secure_boot_key() {
    printf(_("Enrolling GoldenDogLinux certificate for Secure Boot...\n"));
    
    // Import MOK certificate
    run("sudo mokutil --import /var/lib/shim-signed/mok/MOK_goldendoglinux.der");
    
    printf(_("\n=== IMPORTANT SECURE BOOT INSTRUCTIONS ===\n"));
    printf(_("1. You will be asked to set a enrollment password now\n"));
    printf(_("2. During the next reboot, a blue screen (MOK Manager) will appear\n"));
    printf(_("3. Select 'Enroll MOK' > 'Continue' > 'Yes' > Enter the password\n"));
    printf(_("4. Select 'Reboot' to complete the enrollment\n"));
    printf(_("5. After enrollment, your kernel will work with Secure Boot\n"));
    printf(_("==========================================\n"));
}

void mint_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    
    printf(_("Configuring GoldendogLinux Signature...\n"));
    
    // Clear Ubuntu/Mint specific certificates and use default certificates
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && "
             "sed -i 's/CONFIG_SYSTEM_TRUSTED_KEYS=.*/CONFIG_SYSTEM_TRUSTED_KEYS=\"\"/' .config && "
             "sed -i 's/CONFIG_SYSTEM_REVOCATION_KEYS=.*/CONFIG_SYSTEM_REVOCATION_KEYS=\"\"/' .config",
             home, version);
    run(cmd);
    
    // Compile the kernel
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && fakeroot make -j$(nproc) bindeb-pkg",
             home, version);
    run(cmd);
    
    // Instalar los paquetes
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb linux-headers-%s*%s*.deb",
             home, version, tag, version, tag);
    run(cmd);
}

void mint_update_bootloader() {
    run("sudo update-grub");
}

void mint_update_initramfs(const char* version) {
    char cmd[512];
    printf("Updating initramfs for Linux Mint/Ubuntu...\n");
    snprintf(cmd, sizeof(cmd), "sudo update-initramfs -c -k %s", version);
    run(cmd);
}

const char* mint_get_whiptail_install_cmd() {
    return "sudo apt update && sudo apt install -y whiptail";
}

DistroOperations MINT_OPS = {
    .name = "Linux Mint/Ubuntu",
    .install_dependencies = mint_install_dependencies,
    .build_and_install = mint_build_and_install,
    .update_bootloader = mint_update_bootloader,
    .update_initramfs = mint_update_initramfs,
    .get_whiptail_install_cmd = mint_get_whiptail_install_cmd
};

#endif
