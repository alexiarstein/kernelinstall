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
Distro detect_distro();

#endif
