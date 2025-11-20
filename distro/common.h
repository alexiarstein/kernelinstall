#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>

#define _(string) gettext(string)

typedef enum {
    DISTRO_DEBIAN,
    DISTRO_MINT,    // Linux Mint y Ubuntu
    DISTRO_ARCH,
    DISTRO_FEDORA,
    DISTRO_UNKNOWN
} Distro;

// Estructura para operaciones de distribución
typedef struct {
    const char* name;
    void (*install_dependencies)();
    void (*build_and_install)(const char* home, const char* version, const char* tag);
    void (*update_bootloader)();
    const char* (*get_whiptail_install_cmd)();
} DistroOperations;

// Funciones comunes
int run(const char *cmd);
int run_build_with_progress(const char *cmd, const char *source_dir);
Distro detect_distro();
DistroOperations* get_distro_operations(Distro distro);

// Funciones de UI
int check_and_install_whiptail(Distro distro);
int show_welcome_dialog();
int ask_cleanup();
void show_completion_dialog(const char *kernel_version, Distro distro);

// Funciones específicas para Mint
void mint_generate_certificate();
int mint_ask_secure_boot_enrollment();
void mint_enroll_secure_boot_key();

#endif
