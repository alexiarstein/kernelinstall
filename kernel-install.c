// Kernel Installer - Control Program
// This execution wrapper follows the logic of downloading, compiling
// and installing the latest Linux Kernel from kernel.org
// also generates the linux image in .deb format
// Only works for debian-based distros (for now!)
// Author: Alexia Michelle <alexia@goldendoglinux.org>
// LICENSE: GNU GPL 3.0 (see LICENSE for more info)
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>
#define APP_VERSION "1.0.0"
#define _(string) gettext(string)
#define N_(string) string
#define BUBU "bubu" // No pregunten. Esto esta aquí por un buen motivo.
int run(const char *cmd) {
    printf("\n %s: %s\n", _("Running"), cmd);
    int r = system(cmd);
    if (r != 0) {
        fprintf(stderr, _(" Command failed: %s (exit %d)\n"), cmd, r);
        exit(EXIT_FAILURE);
    }
    return r;
}
// este engendro llamado aplicación necesita whiptail para el dialogo de bienvenida.
// si el equipo no tiene whiptail instalado, descargara e instalará el paquete.
// también reiniciamos la aplicación.

int check_and_install_whiptail() {
    if (system("which whiptail > /dev/null 2>&1") != 0) {
        printf(_("whiptail not found. Installing...\n"));

        if (system("sudo apt update && sudo apt install -y whiptail") != 0) {
            fprintf(stderr, _("Failed to install whiptail\n"));
            return -1;
        }

        printf(_("whiptail installed successfully. Restarting application...\n"));

        execl("/proc/self/exe", "/proc/self/exe", NULL);

        // si por algun motivo pincha el restart.
        perror(_("Failed to restart"));
        return -1;
    }
    return 0;
}

int show_welcome_dialog() {
    char command[1024];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yesno \"%s " APP_VERSION "\\n\\n"
             "%s\\n\\n"
             "%s\\n\\n"
             "%s?\" 15 60",
             _("Alexia Kernel Installer"),
             _("Alexia Kernel Installer Version"),
             _("This program will download, compile and install the latest stable kernel from kernel.org."),
             _("The process may take up to three hours in some systems."),
             _("Do you wish to continue"));
    
    int result = system(command);
    return result; // 0 = OK, 1 = CANCELAR
}

int ask_cleanup() {
    char command[512];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yesno \"%s?\" 10 50",
             _("Cleanup Build Files"),
             _("Do you want to clean up the build files"));
    
    int result = system(command);
    return result; // 0 = si, 1 = no
}

void show_completion_dialog(const char *kernel_version) {
    char command[1024];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yes-button \"%s\" --no-button \"%s\" "
             "--yesno \"%s %s.\\n\\n%s.\" 12 60", 
             _("Installation Complete"),
             _("Reboot Now"),
             _("Reboot Later"),
             _("Kernel"),
             kernel_version,
             _("has been successfully installed"));
    
    int result = system(command);
    
    if (result == 0) {
        printf(_("Rebooting system...\n"));
        system("sudo reboot");
    } else {
        printf("\n%s\n", _("Remember to reboot the machine to boot with the latest kernel"));
        printf("%s\n", _("Thank you for using my software"));
        printf("%s\n", _("Please keep it free for everyone"));
        printf("%s\n", _("Alexia."));
    }
}

int main(void) {
    // internacionalizacion, creo que lo escribí bien.
    setlocale(LC_ALL, "");
    
    if (bindtextdomain("kernel-install", "./locale") == NULL) {
        bindtextdomain("kernel-install", "/usr/local/share/locale");
    }
    textdomain("kernel-install");
    
    const char *TAG = "-lexi-amd64";
    const char *home = getenv("HOME");
    
    if (home == NULL) {
        fprintf(stderr, _("Could not determine home directory\n"));
        exit(EXIT_FAILURE);
    }

    if (check_and_install_whiptail() != 0) {
        fprintf(stderr, _("Whiptail installation failed. Continuing with text mode...\n"));
    }
    
    if (show_welcome_dialog() != 0) {
        printf(_("Installation cancelled by user.\n"));
        return 0;
    }

// evaluamos un fix ante una potencial vulnerabilidad.
/*    char build_dir[512];
    snprintf(build_dir, sizeof(build_dir), "%s/kernel_build", home);
    printf(_("Creating build directory: %s\n"), build_dir);
    
    struct stat st = {0};
    if (stat(build_dir, &st) == -1) {
        if (mkdir(build_dir, 0755) != 0) {
            perror(_("Failed to create build directory"));
            exit(EXIT_FAILURE);
        }
    }
*/
char build_dir[512];
snprintf(build_dir, sizeof(build_dir), "%s/kernel_build", home);
printf(_("Creating build directory: %s\n"), build_dir);

if (mkdir(build_dir, 0755) != 0) {
    if (errno == EEXIST) {
        // Verificar que realmente sea un directorio
        struct stat st;
        if (stat(build_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
            // Es un directorio, todo bien
        } else {
            fprintf(stderr, _("Path exists but is not a directory: %s\n"), build_dir);
            exit(EXIT_FAILURE);
        }
    } else {
        perror(_("Failed to create build directory"));
        exit(EXIT_FAILURE);
    }
}
// fix exitoso ^^
    printf(_("Installing required packages...\n"));
    run("sudo apt update");
    run("sudo apt install -y "
        "build-essential libncurses-dev bison flex libssl-dev libelf-dev "
        "bc wget tar xz-utils fakeroot gettext libc6-dev curl git  debhelper libdw-dev rsync locales");

    printf(_("Fetching latest kernel version from kernel.org...\n"));

    char tmp_file[512];
    snprintf(tmp_file, sizeof(tmp_file), "%s/kernel_build/kernelver.txt", home);
    
    char fetch_cmd[1024];
    snprintf(fetch_cmd, sizeof(fetch_cmd),
             "curl -s https://www.kernel.org/ | "
             "grep -A1 'latest_link' | grep -oE '[0-9]+\\.[0-9]+\\.[0-9]+' | "
             "head -1 > %s", tmp_file);
    run(fetch_cmd);

    FILE *f = fopen(tmp_file, "r");
    if (!f) {
        fprintf(stderr, _("Could not fetch latest kernel version.\n"));
        exit(EXIT_FAILURE);
    }

// test, reemplazar strcspn por strchr para que sonarqube no se vuelva loquita.
   /*
 char latest[32];
    if (!fgets(latest, sizeof(latest), f)) {
        fprintf(stderr, _("Empty version string.\n"));
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);
    latest[strcspn(latest, "\n")] = 0; 
*/

char latest[32];
if (!fgets(latest, sizeof(latest), f)) {
    fprintf(stderr, _("Empty version string.\n"));
    fclose(f);
    exit(EXIT_FAILURE);
}
fclose(f);

// Buscar newline con strchr
char *newline = strchr(latest, '\n');
if (newline) {
    *newline = '\0';  // Eliminar newline si existe
}

    printf(_("Latest stable kernel: %s\n"), latest);

    char cmd[1024];

// improvement: hace toda la cochinada en el homedir asi no pedimos sudo para ir a /usr/src
// asegurense de tener mucho espacio en disco para esa parti.

    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "wget -O linux-%s.tar.xz https://cdn.kernel.org/pub/linux/kernel/v%c.x/linux-%s.tar.xz",
             home, latest, latest[0], latest);
    run(cmd);

    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && tar -xf linux-%s.tar.xz", home, latest);
    run(cmd);

    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && "
             "cp /boot/config-$(uname -r) .config && "
             "yes \"\" | make oldconfig", home, latest);
    run(cmd);

    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && "
             "sed -i 's/^CONFIG_LOCALVERSION=.*/CONFIG_LOCALVERSION=\"%s\"/' .config",
             home, latest, TAG);
    run(cmd);

// compilamos en el home tmb.
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build/linux-%s && fakeroot make -j$(nproc) bindeb-pkg",
             home, latest);
    run(cmd);

// para instalar la imagen y los headers si necesitamos sudo
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb linux-headers-%s*%s*.deb",
             home, latest, TAG, latest, TAG);
    run(cmd);

    run("sudo update-grub");

    if (ask_cleanup() == 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s/kernel_build", home);
        run(cmd);
        printf(_("Build files cleaned up.\n"));
    }

    char full_kernel_version[64];
    snprintf(full_kernel_version, sizeof(full_kernel_version), "%s%s", latest, TAG);
    show_completion_dialog(full_kernel_version);

    return 0;
}
/*
 *
 * Resta evaluarlo mas en sistemas debian, clones de debian
 * y luego ver como podemos refactorizarlo para que soporte distros
 * del ecosistema rhel y arch.
 *
 * - Alexia.
 *
*/
