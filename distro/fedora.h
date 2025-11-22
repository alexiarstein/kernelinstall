// esto está re experimental por ahora. Probar bajo su propio riesgo.
// - Alexia.
#ifndef FEDORA_H
#define FEDORA_H

#include "common.h"

static void replace_char(const char *src, char *dst, char c, char r, size_t length) {
    for (size_t i = 0; i < length - 1; i++) {
        dst[i] = src[i];
        if (src[i] == c)
            dst[i] = r;
        if (src[i] == '\0')
            return;
    }

    dst[length - 1] = '\0';
}

void fedora_install_dependencies() {
    run("sudo dnf install -y "
        "gcc make ncurses-devel bison flex openssl-devel elfutils-libelf-devel "
        "rpm-build newt curl git wget tar xz");
}

void fedora_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    char source_dir[512];
    
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);

    // Cambia algunas configuraciones que tiene el kernel de Fedora que no es
    // posible replicar directamente, relacionadas a Secure Boot.
    snprintf(cmd, sizeof(cmd),
             "cd %s && "
             "sed -i 's/CONFIG_SYSTEM_TRUSTED_KEYS=.*/CONFIG_SYSTEM_TRUSTED_KEYS=\"\"/' .config && "
             "sed -i 's/CONFIG_EFI_SBAT=.*/CONFIG_EFI_SBAT=\"\"/' .config && "
             "sed -i 's/CONFIG_EFI_SBAT_FILE=.*/CONFIG_EFI_SBAT_FILE=\"\"/' .config",
             source_dir);
    run(cmd);

    // Compilar generando RPMs
    snprintf(cmd, sizeof(cmd),
             "cd %s && LC_ALL=C stdbuf -oL -eL make -j$(nproc) binrpm-pkg",
             source_dir);
    run_build_with_progress(cmd, source_dir);
    
    // Instalar los RPMs generados
    // A partir de la version 6.6 del kernel, todos los paquetes generados por make *rpm-pkg
    // se localizan dentro de la raiz del codigo fuente del kernel, especificamente
    // en el directorio ./rpmbuild/RPMS/$(uname -m).
    
    // La version en el rpm es cambiaba, reemplazando los '-' por '_' en la parte del tag.
    char tag_rpm[64];
    replace_char(tag, tag_rpm, '-', '_', 64);

    snprintf(cmd, sizeof(cmd),
             "cd %s/rpmbuild/RPMS/$(uname -m) && "
             "sudo dnf install -y kernel-%s*%s*.rpm kernel-headers-%s*%s*.rpm kernel-devel-%s*%s*.rpm",
             source_dir, version, tag_rpm, version, tag_rpm, version, tag_rpm);
    
    run(cmd);
}

void fedora_update_bootloader() {
    // Fedora usa BLS (Boot Loader Specification) y grubby o dnf manejan esto automáticamente al instalar el kernel.
    // Sin embargo, regenerar grub.cfg no hace daño para asegurar.
    run("sudo grub2-mkconfig -o /boot/grub2/grub.cfg");
}

const char* fedora_get_whiptail_install_cmd() {
    return "sudo dnf install -y newt";
}

DistroOperations FEDORA_OPS = {
    .name = "Fedora",
    .install_dependencies = fedora_install_dependencies,
    .build_and_install = fedora_build_and_install,
    .update_bootloader = fedora_update_bootloader,
    .get_whiptail_install_cmd = fedora_get_whiptail_install_cmd
};

#endif
// y falta agregarle soporte a rhel y todos sus clones.
// - Alexia.
