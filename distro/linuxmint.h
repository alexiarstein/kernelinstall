// Usuarios de linux mint, y también ubuntu, elementaryOS y basicamente cualquier otro clon de ubuntu:
// Canonical firma el kernel con una llave de Microsoft para secure boot.
// Aqui lo que hacemos es firmar el kernel con una de Goldendog Linux
// Si desean que este kernel tenga secure boot, y tienen uefi, pueden agregarlo al boot.
// sino, simplemente deshabilitar secure boot.

#ifndef LINUXMINT_H
#define LINUXMINT_H

#include "common.h"

void mint_install_dependencies() {
    run("sudo apt update && sudo apt install -y "
        "build-essential libncurses-dev pkg-config libncursesw5-dev bison flex libssl-dev libssl-dev libelf-dev "
        "bc wget tar xz-utils fakeroot curl git debhelper libdw-dev rsync locales gawk gettext "
        "mokutil openssl");
}

void mint_generate_certificate() {
    printf(_("Generating GoldenDogLinux Secure Boot certificate...\n"));
    
    // Crear directorio para los certificados MOK
    run("sudo mkdir -p /var/lib/shim-signed/mok/");
    
    // Generar certificado autofirmado (válido por 10 años)
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "sudo openssl req -nodes -new -x509 -newkey rsa:2048 "
             "-keyout /var/lib/shim-signed/mok/MOK_goldendoglinux.priv "
             "-outform DER -out /var/lib/shim-signed/mok/MOK_goldendoglinux.der "
             "-days 3650 -subj \"/CN=GoldenDogLinux Secure Boot Key/\"");
    run(cmd);
    
    // Establecer permisos adecuados para la clave privada
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
    
    // Importar el certificado MOK
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
    
    // Limpiar certificados específicos de Ubuntu/Mint y usar certificados por defecto
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && "
             "sed -i 's/CONFIG_SYSTEM_TRUSTED_KEYS=.*/CONFIG_SYSTEM_TRUSTED_KEYS=\"\"/' .config && "
             "sed -i 's/CONFIG_SYSTEM_REVOCATION_KEYS=.*/CONFIG_SYSTEM_REVOCATION_KEYS=\"\"/' .config",
             home, version);
    run(cmd);
    
    // Compilar el kernel
    char source_dir[512];
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);

    snprintf(cmd, sizeof(cmd),
             "cd %s && stdbuf -oL -eL LC_ALL=C fakeroot make -j$(nproc) bindeb-pkg",
             source_dir);
    run_build_with_progress(cmd, source_dir);
    
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

const char* mint_get_whiptail_install_cmd() {
    return "sudo apt update && sudo apt install -y whiptail";
}

DistroOperations MINT_OPS = {
    .name = "Linux Mint/Ubuntu",
    .install_dependencies = mint_install_dependencies,
    .build_and_install = mint_build_and_install,
    .update_bootloader = mint_update_bootloader,
    .get_whiptail_install_cmd = mint_get_whiptail_install_cmd
};

#endif
