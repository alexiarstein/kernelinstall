// refactoricé la funcion antigua detect_distro para que sea mas simple de escalar por que se 
// estaba armando un choclo terrible a medida que agregaba más IDs. 
// ahora se pueden agregar las IDs aquí según el grupo al cual pertenecen.
// -- Alexia.


#ifndef DISTROS_H
#define DISTROS_H

#include "common.h"

// Estructura para mapear ID de distribución a tipo de distro
typedef struct {
    const char* id;
    Distro distro_type;
} DistroMap;

// Lista de distribuciones y sus tipos
static DistroMap distro_map[] = {
    // Debian y derivados directos
    {"debian", DISTRO_DEBIAN},
    {"goldendoglinux", DISTRO_DEBIAN},
    {"soplos", DISTRO_DEBIAN},

    // Mint/Ubuntu y derivados (que requieren el tratamiento de certificados)
    {"linuxmint", DISTRO_MINT},
    {"ubuntu", DISTRO_MINT},
    {"elementary", DISTRO_MINT},
    {"pop", DISTRO_MINT},
    {"zorin", DISTRO_MINT},
    {"devuan", DISTRO_DEBIAN},
    {"quirinux", DISTRO_DEBIAN},
    {"etertics", DISTRO_DEBIAN},
    {"goblin", DISTRO_DEBIAN},
    {"gobmis", DISTRO_DEBIAN},
    {"huayra", DISTRO_DEBIAN},

    // Otras distribuciones
    {"arch", DISTRO_ARCH},
    {"fedora", DISTRO_FEDORA},

    // Fin de la lista
    {NULL, DISTRO_UNKNOWN}
};

#endif
