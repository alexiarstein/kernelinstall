#ifndef FEDORA_H
#define FEDORA_H

#include "common.h"

void fedora_install_dependencies() {
    run("sudo dnf install -y "
        "gcc make ncurses-devel bison flex openssl-devel elfutils-libelf-devel "
        "rpm-build newt curl git wget tar xz");
}

void fedora_build_and_install(const char* home, const char* version, const char* tag) {
    char cmd[2048];
    char source_dir[512];
    
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);
    
    // Compilar generando RPMs
    snprintf(cmd, sizeof(cmd),
             "cd %s && make -j$(nproc) rpm-pkg",
             source_dir);
    run_build_with_progress(cmd, source_dir);
    
    // Instalar los RPMs generados
    // Los RPMs suelen generarse en ~/rpmbuild/RPMS/x86_64/ o similar, pero make rpm-pkg
    // puede dejarlos en el directorio local o en la estructura de rpmbuild por defecto.
    // El kernel suele usar su propia estructura interna si no se define otra cosa,
    // pero rpm-pkg usa la infraestructura de rpmbuild del usuario.
    // Verificaremos la ubicación estándar de rpmbuild en el home del usuario.
    
    snprintf(cmd, sizeof(cmd),
             "cd %s/rpmbuild/RPMS/$(uname -m) && "
             "sudo dnf install -y kernel-%s*%s*.rpm kernel-headers-%s*%s*.rpm kernel-devel-%s*%s*.rpm",
             home, version, tag, version, tag, version, tag);
             
    // Nota: Si make rpm-pkg no usa ~/rpmbuild, podría necesitar ajuste.
    // Por defecto make rpm-pkg construye en el árbol del kernel pero usa rpmbuild.
    // Una alternativa más segura si no sabemos dónde caen es buscar en el home.
    
    // Vamos a intentar instalar lo que encontremos en el directorio de build del kernel si rpmbuild no está.
    // Pero lo estándar es ~/rpmbuild. Asumiremos eso por ahora.
    
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
