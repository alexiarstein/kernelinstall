/*
 * Kernel Installer - Control Program
 * Copyright (C) 2025 Alexia Michelle <alexia@goldendoglinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * This execution wrapper follows the logic of downloading, compiling
 * and installing the latest Linux Kernel from kernel.org
 * Modular version with distro-specific support
 * see CHANGELOG for more info.
 */
 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>
#include <ncurses.h>

#include "distro/common.h"
#include "distro/debian.h"
#include "distro/linuxmint.h"
#include "distro/fedora.h"
#include "distro/distros.h"

#define APP_VERSION "1.3.2"
#define _(string) gettext(string)
#define BUBU "bubu" // menos pregunta dios y perdona

// ========== INICIO FUNC AUXILIARES ==========

int run(const char *cmd) {
    printf("\n %s: %s\n", _("Running"), cmd);
    int r = system(cmd);
    if (r != 0) {
        fprintf(stderr, _(" Command failed: %s (exit %d)\n"), cmd, r);
        exit(EXIT_FAILURE);
    }
    return r;
}

// New function to verify SHA256 checksum: If matches, kernel source do not need to be re-downloaded
int verify_sha256(const char *filepath, const char *expected_sha256) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "sha256sum %s | awk '{print $1}'", filepath);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    char actual_sha256[128];
    if (!fgets(actual_sha256, sizeof(actual_sha256), fp)) {
        pclose(fp);
        return 0;
    }
    pclose(fp);
    
    // Remove newline
    char *newline = strchr(actual_sha256, '\n');
    if (newline) *newline = '\0';
    
    return strcmp(actual_sha256, expected_sha256) == 0;
}

// New function to download and return SHA256 checksum
// prevent downloading xz file, if I cant verify checksum existing
int get_kernel_sha256(const char *version, char *sha256_out, size_t sha256_size) {
    char tmp_sha_file[512];
    snprintf(tmp_sha_file, sizeof(tmp_sha_file), "/tmp/kernel-%s.sha256", version);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "wget -q -O %s https://cdn.kernel.org/pub/linux/kernel/v%c.x/sha256sums.asc",
             tmp_sha_file, version[0]);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Warning: Could not download SHA256 checksums\n");
        return -1;
    }
    
    // Extract SHA256 for our specific kernel version
    snprintf(cmd, sizeof(cmd),
             "grep 'linux-%s.tar.xz' %s | awk '{print $1}'",
             version, tmp_sha_file);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        unlink(tmp_sha_file);
        return -1;
    }
    
    if (!fgets(sha256_out, sha256_size, fp)) {
        pclose(fp);
        unlink(tmp_sha_file);
        return -1;
    }
    pclose(fp);
    unlink(tmp_sha_file);
    
    // Remove newline
    char *newline = strchr(sha256_out, '\n');
    if (newline) *newline = '\0';
    
    return strlen(sha256_out) == 64 ? 0 : -1;
}

int count_source_files(const char *dir) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "find %s -name '*.c' | wc -l", dir);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 20000; // Fallback estimado
    char buf[32];
    if (!fgets(buf, sizeof(buf), fp)) {
        pclose(fp);
        return 20000;
    }
    pclose(fp);
    return atoi(buf);
}

int run_build_with_progress(const char *cmd, const char *source_dir) {
    int total_files = count_source_files(source_dir);
    if (total_files == 0) total_files = 1;

    initscr();
    cbreak();
    noecho();
    curs_set(0); // Ocultar cursor
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK); 
    }

    int height, width;
    getmaxyx(stdscr, height, width);


    int header_height = 1;
    int sep1_height = 1;
    int sep2_height = 1;
    int bar_height = 1;
    int log_height = height - header_height - sep1_height - sep2_height - bar_height;
    
    if (log_height < 5) log_height = 5; 

  
    WINDOW *header_win = newwin(header_height, width, 0, 0);
    WINDOW *sep1_win = newwin(sep1_height, width, 1, 0);
    WINDOW *log_win = newwin(log_height, width, 2, 0);
    WINDOW *sep2_win = newwin(sep2_height, width, height - 2, 0);
    WINDOW *bar_win = newwin(bar_height, width, height - 1, 0);

    scrollok(log_win, TRUE);

   
    char header_text[256];
    snprintf(header_text, sizeof(header_text), "Alexia Kernel Installer Version %s", APP_VERSION);
    int header_len = strnlen(header_text, sizeof(header_text));
    int header_x = (width - header_len) / 2;
    if (header_x < 0) header_x = 0;
    
    if (has_colors()) wattron(header_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(header_win, 0, header_x, "%s", header_text);
    if (has_colors()) wattroff(header_win, COLOR_PAIR(2) | A_BOLD);
    wrefresh(header_win);

   
    mvwhline(sep1_win, 0, 0, ACS_HLINE, width);
    wrefresh(sep1_win);
    
    mvwhline(sep2_win, 0, 0, ACS_HLINE, width);
    wrefresh(sep2_win);

    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);

    FILE *build_pipe = popen(full_cmd, "r");
    if (!build_pipe) {
        endwin();
        perror("popen build");
        return -1;
    }

    FILE *gauge_pipe = popen("whiptail --gauge \"Compiling kernel...\" 6 50 0", "w");
    if (!gauge_pipe) {
        // If gauge fails, just run normally? Or fail?
        // Let's try to continue without gauge if possible, but for now just fail or print to stdout.
        // But we already opened build_pipe.
        // Let's just proceed without gauge if it fails, but we need to consume output.
    }

    char line[1024];
    int current_count = 0;
    int packaging_started = 0;
    char current_status_msg[256] = ""; 

    while (1) {
        if (fgets(line, sizeof(line), build_pipe) == NULL) {
            if (errno == EINTR) {
                clearerr(build_pipe);
                endwin();
                refresh(); 
                getmaxyx(stdscr, height, width);

                log_height = height - header_height - sep1_height - sep2_height - bar_height;
                if (log_height < 5) log_height = 5;

                // redibujamos todo.
                wresize(header_win, header_height, width);
                mvwin(header_win, 0, 0);
                
                wresize(sep1_win, sep1_height, width);
                mvwin(sep1_win, 1, 0);
                
                wresize(log_win, log_height, width);
                mvwin(log_win, 2, 0);
                
                wresize(sep2_win, sep2_height, width);
                mvwin(sep2_win, height - 2, 0);
                
                wresize(bar_win, bar_height, width);
                mvwin(bar_win, height - 1, 0);

                
                werase(header_win);
                snprintf(header_text, sizeof(header_text), "Alexia Kernel Installer Version %s", APP_VERSION);
                header_len = strnlen(header_text, sizeof(header_text));
                header_x = (width - header_len) / 2;
                if (header_x < 0) header_x = 0;
                if (has_colors()) wattron(header_win, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(header_win, 0, header_x, "%s", header_text);
                if (has_colors()) wattroff(header_win, COLOR_PAIR(2) | A_BOLD);
                wrefresh(header_win);

      
                werase(sep1_win);
                mvwhline(sep1_win, 0, 0, ACS_HLINE, width);
                wrefresh(sep1_win);

                werase(sep2_win);
                mvwhline(sep2_win, 0, 0, ACS_HLINE, width);
                wrefresh(sep2_win);

             
                wrefresh(log_win); 
                
                if (packaging_started) {
                    werase(bar_win);
                    if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
                    mvwprintw(bar_win, 0, 0, "%s", current_status_msg);
                    if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
                    wrefresh(bar_win);
                }
                
                continue;
            }
            break;
        }
        wprintw(log_win, "%s", line);
        wrefresh(log_win);

        if (strstr(line, " CC ") || strstr(line, " LD ") || strstr(line, " AR ")) {
            current_count++;
            int percent = (current_count * 100) / total_files;
            if (percent > 100) percent = 100;

            // Dibujar barra
            werase(bar_win);
            mvwprintw(bar_win, 0, 0, "%s [", _("Progress:"));
            
            int bar_width = width - 20; // Espacio para "Progress: " y " XXX%"
            int filled_width = (percent * bar_width) / 100;
            
            if (has_colors()) wattron(bar_win, COLOR_PAIR(1));
            for (int i = 0; i < bar_width; i++) {
                if (i < filled_width) waddch(bar_win, '=');
                else if (i == filled_width) waddch(bar_win, '>');
                else waddch(bar_win, ' ');
            }
            if (has_colors()) wattroff(bar_win, COLOR_PAIR(1));
            
            wprintw(bar_win, "] %d%%", percent);
            wrefresh(bar_win);
        }

        
        if (!packaging_started) {
            if (strstr(line, "dpkg-deb: building package")) {
                packaging_started = 1;
                snprintf(current_status_msg, sizeof(current_status_msg), "%s", _("Building kernel and kernel headers .deb package. Please wait..."));
                werase(bar_win);
                if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(bar_win, 0, 0, "%s", current_status_msg);
                if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
                wrefresh(bar_win);
            } else if (strstr(line, "Processing files:")) {
                packaging_started = 1;
                snprintf(current_status_msg, sizeof(current_status_msg), "%s", _("Building kernel .rpm package. Please wait..."));
                werase(bar_win);
                if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(bar_win, 0, 0, "%s", current_status_msg);
                if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
                wrefresh(bar_win);
            }
            printf(_("\nPackaging started. Please wait...\n"));
        }
        
        // If packaging started, we might want to show the output
        if (packaging_started) {
            printf("%s", line);
        }
    }

    endwin(); // Restaurar terminal
    return pclose(build_pipe);
}

int check_and_install_whiptail(Distro distro) {
    if (system("which whiptail > /dev/null 2>&1") != 0) {
        printf(_("whiptail not found. Installing...\n"));

        DistroOperations* ops = get_distro_operations(distro);
        if (!ops || !ops->get_whiptail_install_cmd) {
            fprintf(stderr, _("Cannot install whiptail on this distribution\n"));
            return -1;
        }

        if (system(ops->get_whiptail_install_cmd()) != 0) {
            fprintf(stderr, _("Failed to install whiptail\n"));
            return -1;
        }

        printf(_("whiptail installed successfully. Restarting application...\n"));
        execl("/proc/self/exe", "/proc/self/exe", NULL);
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
    return result;
}

int ask_cleanup() {
    char command[512];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yesno \"%s?\" 10 50",
             _("Cleanup Build Files"),
             _("Do you want to clean up the build files"));
    
    int result = system(command);
    return result;
}

// New function to check if kernel is already built, to skip rebuild if not necessary
int is_kernel_built(const char *source_dir, const char *version, const char *tag) {
    char vmlinuz_path[1024];
    char system_map_path[1024];
    
    // Check for vmlinuz (different names on different architectures)
    snprintf(vmlinuz_path, sizeof(vmlinuz_path), 
             "%s/arch/x86/boot/bzImage", source_dir);
    
    snprintf(system_map_path, sizeof(system_map_path),
             "%s/System.map", source_dir);
    
    struct stat st;
    int vmlinuz_exists = (stat(vmlinuz_path, &st) == 0);
    int system_map_exists = (stat(system_map_path, &st) == 0);
    
    if (vmlinuz_exists && system_map_exists) {
        // Additional check: verify the kernel version in the built image
        char version_check[2048];  // Increased buffer size to avoid truncation
        snprintf(version_check, sizeof(version_check),
                 "strings %s | grep -q 'Linux version %s%s'",
                 vmlinuz_path, version, tag);
        
        if (system(version_check) == 0) {
            return 1; // Kernel is already built with correct version
        }
    }
    
    return 0; // Kernel not built or version mismatch
}

// New function to check if kernel packages already exist
int are_packages_built(const char *home, const char *version, const char *tag, Distro distro) {
    char package_path[1024];
    
    if (distro == DISTRO_DEBIAN || distro == DISTRO_MINT) {
        // Check for .deb packages
        snprintf(package_path, sizeof(package_path),
                 "%s/kernel_build/linux-image-%s%s_*.deb", home, version, tag);
        
        // Use glob to check if pattern matches any files
        char check_cmd[1024];
        snprintf(check_cmd, sizeof(check_cmd),
                 "ls %s/kernel_build/linux-image-%s%s_*.deb 2>/dev/null | grep -q .",
                 home, version, tag);
        
        return (system(check_cmd) == 0);
        
    } else if (distro == DISTRO_FEDORA) {
        // Check for .rpm packages
        snprintf(package_path, sizeof(package_path),
                 "%s/kernel_build/linux-%s/kernel-%s%s*.rpm", 
                 home, version, version, tag);
        
        char check_cmd[1024];
        snprintf(check_cmd, sizeof(check_cmd),
                 "ls %s/kernel_build/linux-%s/kernel-%s%s*.rpm 2>/dev/null | grep -q .",
                 home, version, version, tag);
        
        return (system(check_cmd) == 0);
    }
    
    return 0;
}

// New function to ask user about rebuild
int ask_rebuild() {
    char command[768];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yesno \"%s\\n\\n%s\\n\\n%s?\" 14 70",
             "Kernel Already Built",
             "The kernel appears to be already compiled in the build directory.",
             "Building again will take 2-3 hours and may not be necessary.",
             "Do you want to rebuild from scratch");
    
    return system(command);
}

// ========== FIN DE FUNCIONES AUXILIARES ==========

Distro detect_distro() {
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) return DISTRO_UNKNOWN;
    
    char line[256];
    Distro detected = DISTRO_UNKNOWN;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "ID=", 3) == 0) {
            char *id_value = line + 3;
            if (id_value[0] == '"') {
                id_value++;
                char *end_quote = strchr(id_value, '"');
                if (end_quote) *end_quote = '\0';
            } else {
                char *newline = strchr(id_value, '\n');
                if (newline) *newline = '\0';
            }
            for (int i = 0; distro_map[i].id != NULL; i++) {
                if (strcmp(id_value, distro_map[i].id) == 0) {
                    detected = distro_map[i].distro_type;
                    break;
                }
            }
            break;
        }
    }
    
    fclose(fp);
    return detected;
}


DistroOperations* get_distro_operations(Distro distro) {
    switch (distro) {
        case DISTRO_DEBIAN:
            return &DEBIAN_OPS;
        case DISTRO_MINT:
            return &MINT_OPS;
        case DISTRO_FEDORA:
            return &FEDORA_OPS;
        default:
            return NULL;
    }
}

// NUEVA FUNCIÓN: Manejar Secure Boot para Mint/Ubuntu
void handle_secure_boot_enrollment(Distro distro) {
    if (distro == DISTRO_MINT) {
        // Generar certificado GoldenDogLinux
        mint_generate_certificate();
        
        // Preguntar si quiere enrolar para Secure Boot
        if (mint_ask_secure_boot_enrollment() == 0) {
            mint_enroll_secure_boot_key();
        } else {
            printf(_("Secure Boot enrollment skipped.\n"));
            printf(_("You can enroll the certificate later with: sudo mokutil --import /var/lib/shim-signed/mok/MOK_goldendoglinux.der\n"));
        }
    }
}

void show_completion_dialog(const char *kernel_version, Distro distro) {
    char command[1024];
    snprintf(command, sizeof(command),
             "whiptail --title \"%s\" "
             "--yes-button \"%s\" --no-button \"%s\" "
             "--yesno \"%s %s.\\n\\n%s.\\n\\n"
             "%s.\" 14 60", 
             _("Installation Complete"),
             _("Reboot Now"),
             _("Reboot Later"),
             _("Kernel"),
             kernel_version,
             _("has been successfully installed"),
             _("If you enrolled Secure Boot, complete the enrollment during reboot"));
    
    int result = system(command);
    
    if (result == 0) {
        printf(_("Rebooting system...\n"));
        if (distro == DISTRO_MINT) {
            printf(_("Remember: If you enrolled Secure Boot, look for the blue MOK Manager screen!\n"));
        }
        system("sudo reboot");
    } else {
        printf("\n%s\n", _("Remember to reboot the machine to boot with the latest kernel"));
        if (distro == DISTRO_MINT) {
            printf("%s\n", _("If you enrolled Secure Boot, complete the enrollment during reboot"));
        }
        printf("%s\n", _("Thank you for using my software"));
        printf("%s\n", _("Please keep it free for everyone"));
        printf("%s\n", _("Alexia."));
    }
}

int main(void) {
    setlocale(LC_ALL, "");
    
    if (bindtextdomain("kernel-install", "./locale") == NULL) {
        bindtextdomain("kernel-install", "/usr/local/share/locale");
    }
    textdomain("kernel-install");
    
    const char *TAG = "-lexi-amd64";
    const char *home = getenv("HOME");
    char cmd[1024];  // Declare cmd here
    
    if (home == NULL) {
        fprintf(stderr, _("Could not determine home directory\n"));
        exit(EXIT_FAILURE);
    }

    // Detectar distribución y obtener operaciones
    Distro distro = detect_distro();
    DistroOperations* ops = get_distro_operations(distro);
    
    if (!ops) {
        fprintf(stderr, _("Unsupported Linux distribution. Currently only Debian-based systems are supported.\n"));
        exit(EXIT_FAILURE);
    }
    
    printf(_("Detected distribution: %s\n"), ops->name);
    
    if (check_and_install_whiptail(distro) != 0) {
        fprintf(stderr, _("Whiptail installation failed. Continuing with text mode...\n"));
    }
    
    if (show_welcome_dialog() != 0) {
        printf(_("Installation cancelled by user.\n"));
        return 0;
    }

    char build_dir[512];
    snprintf(build_dir, sizeof(build_dir), "%s/kernel_build", home);
    printf(_("Creating build directory: %s\n"), build_dir);
// patch de seguridad.
    if (mkdir(build_dir, 0755) != 0) {
        if (errno == EEXIST) {
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

    // Instalar las dependencias específicas de la distribución
    printf(_("Installing required packages for %s...\n"), ops->name);
    ops->install_dependencies();

    // Para Mint/Ubuntu: generar certificado GoldenDogLinux
    if (distro == DISTRO_MINT) {
        mint_generate_certificate();
    }


    // Descargar la versión más reciente del kernel
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

    char latest[32];
    if (!fgets(latest, sizeof(latest), f)) {
        fprintf(stderr, _("Empty version string.\n"));
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);

    char *newline = strchr(latest, '\n');
    if (newline) {
        *newline = '\0';
    }

    printf(_("Latest stable kernel: %s\n"), latest);

    // Check if kernel tarball already exists
    char tarball_path[512];
    snprintf(tarball_path, sizeof(tarball_path),
             "%s/kernel_build/linux-%s.tar.xz", home, latest);
    
    int need_download = 1;
    struct stat st;
    if (stat(tarball_path, &st) == 0) {
        printf("Kernel source tarball already exists. Verifying checksum...\n");
        
        char expected_sha256[128];
        if (get_kernel_sha256(latest, expected_sha256, sizeof(expected_sha256)) == 0) {
            if (verify_sha256(tarball_path, expected_sha256)) {
                printf("Checksum verification passed. Kernel source already downloaded, reusing existing file.\n");
                need_download = 0;
            } else {
                printf("Checksum verification failed. Existing file is corrupted or outdated.\n");
                printf("Deleting existing file and downloading fresh copy from kernel.org...\n");
                unlink(tarball_path);
            }
        } else {
            printf("Warning: Could not verify checksum. Reusing existing file.\n");
            need_download = 0;
        }
    }

    // Descargar el kernel solo si es necesario
    if (need_download) {
        snprintf(cmd, sizeof(cmd),
                 "cd %s/kernel_build && "
                 "wget -O linux-%s.tar.xz https://cdn.kernel.org/pub/linux/kernel/v%c.x/linux-%s.tar.xz",
                 home, latest, latest[0], latest);
        run(cmd);
    }

    // Check if source is already extracted
    char source_dir[512];
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, latest);
    
    int need_extract = 1;
    if (stat(source_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Kernel source directory already exists. Skipping extraction.\n");
        need_extract = 0;
    }

    if (need_extract) {
        snprintf(cmd, sizeof(cmd),
                 "cd %s/kernel_build && tar -xf linux-%s.tar.xz", home, latest);
        run(cmd);
    }

    // Check if kernel is already built
    int kernel_already_built = is_kernel_built(source_dir, latest, TAG);
    int packages_already_built = are_packages_built(home, latest, TAG, distro);
    
    if (kernel_already_built || packages_already_built) {
        printf("\n========================================\n");
        if (kernel_already_built) {
            printf("Compiled kernel binary (vmlinuz) detected in build directory.\n");
        }
        if (packages_already_built) {
            printf("Installation packages (.deb/.rpm) already exist in build directory.\n");
        }
        printf("Build appears to be complete.\n");
        printf("========================================\n\n");
        
        if (ask_rebuild() != 0) {
            printf("Skipping rebuild. Using existing compiled kernel.\n");
            printf("Proceeding directly to installation...\n\n");
            
            // Skip to installation phase
            goto install_phase;
        } else {
            printf("User chose to rebuild. Starting clean build...\n");
            // Clean the build directory
            snprintf(cmd, sizeof(cmd),
                     "cd %s/kernel_build/linux-%s && make mrproper",
                     home, latest);
            run(cmd);
        }
    }

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


    printf(_("Building and installing kernel for %s...\n"), ops->name);
    ops->build_and_install(home, latest, TAG);

install_phase:
    // Actualizar bootloader
    printf(_("Updating bootloader for %s...\n"), ops->name);
    ops->update_bootloader();

    // Para Mint/Ubuntu: ofrecer enrolamiento Secure Boot
    if (distro == DISTRO_MINT) {
        if (mint_ask_secure_boot_enrollment() == 0) {
            mint_enroll_secure_boot_key();
        } else {
            printf(_("Secure Boot enrollment skipped.\n"));
            printf(_("You can enroll the certificate later with: sudo mokutil --import /var/lib/shim-signed/mok/MOK_goldendoglinux.der\n"));
        }
    }

    // Limpieza
    if (ask_cleanup() == 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s/kernel_build", home);
        run(cmd);
        printf(_("Build files cleaned up.\n"));
    }

    char full_kernel_version[64];
    snprintf(full_kernel_version, sizeof(full_kernel_version), "%s%s", latest, TAG);
    show_completion_dialog(full_kernel_version, distro);

    return 0;
}
