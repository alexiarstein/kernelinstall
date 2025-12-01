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
#include <time.h>
#include <sys/select.h>
#include <ncurses.h>
#include "distro/common.h"
#include "distro/debian.h"
#include "distro/linuxmint.h"
#include "distro/fedora.h"
#include "distro/distros.h"

#define APP_VERSION "1.4.1"
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

void update_packaging_timer(WINDOW *bar_win, time_t start_time, char *status_msg, size_t msg_size) {
    time_t now = time(NULL);
    double elapsed = difftime(now, start_time);
    int hours = (int)elapsed / 3600;
    int minutes = ((int)elapsed % 3600) / 60;
    int seconds = (int)elapsed % 60;
    
    // Determinar mensaje base (DEB o RPM)
    const char* base_msg = _("Building kernel and kernel headers .deb package. Please wait...");
    if (strstr(status_msg, "rpm")) {
        base_msg = _("Building kernel .rpm package. Please wait...");
    }

    snprintf(status_msg, msg_size, 
             "%s [ %s: %02d:%02d:%02d ]", 
             base_msg,
             _("Elapsed"), hours, minutes, seconds);
    
    werase(bar_win);
    if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(bar_win, 0, 0, "%s", status_msg);
    if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
    wrefresh(bar_win);
}

// Funciones para System Load Monitor - trabajo original en https://github.com/alexiarstein/systemload

int get_cpu_count() {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return 1;
    
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processor", 9) == 0) {
            count++;
        }
    }
    fclose(fp);
    if (count == 0) count = 1;
    return count;
}

void get_load_avg(double *loads) {
    char *old_locale = setlocale(LC_NUMERIC, NULL);
    char *saved_locale = old_locale ? strdup(old_locale) : NULL;
    
    setlocale(LC_NUMERIC, "C");
    
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        fscanf(fp, "%lf %lf %lf", &loads[0], &loads[1], &loads[2]);
        fclose(fp);
    } else {
        loads[0] = loads[1] = loads[2] = 0.0;
    }
    
    if (saved_locale) {
        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);
    }
}

void draw_system_load(WINDOW *win, int cpu_count) {
    double loads[3] = {0.0, 0.0, 0.0};
    get_load_avg(loads);
    
    double current_usage = (loads[0] / cpu_count) * 100.0;
    if (current_usage > 100.0) current_usage = 100.0;

    werase(win);
    box(win, 0, 0);
    
    int width = getmaxx(win);
    
    if (has_colors()) wattron(win, A_BOLD);
    mvwprintw(win, 1, (width - 11) / 2, "%s", _("SYSTEM LOAD"));
    if (has_colors()) wattroff(win, A_BOLD);
    
    mvwhline(win, 2, 1, ACS_HLINE, width - 2);

    // Barra de carga
    int bar_width = width - 4;
    int filled = (int)((current_usage / 100.0) * bar_width);
    
    int color_pair = 1; // verde
    if (current_usage > 70) color_pair = 3; // rojo
    else if (current_usage > 35) color_pair = 4; // amarillo
    
    if (has_colors()) wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, 4, 2, "[");
    for (int i = 0; i < bar_width - 2; i++) {
        if (i < filled) waddch(win, '|');
        else waddch(win, ' ');
    }
    waddch(win, ']');
    if (has_colors()) wattroff(win, COLOR_PAIR(color_pair));
    
    mvwprintw(win, 5, (width - 10) / 2, "%.2f%%", current_usage);

    // Stats numéricos
    int start_y = 7;
    mvwprintw(win, start_y, 2, "%s", _("Load Average:"));
    mvwprintw(win, start_y + 1, 4, "1m : %.2f", loads[0]);
    mvwprintw(win, start_y + 2, 4, "5m : %.2f", loads[1]);
    mvwprintw(win, start_y + 3, 4, "15m: %.2f", loads[2]);
    
    mvwprintw(win, start_y + 5, 2, "%s: %d", _("Cores"), cpu_count);

    wrefresh(win);
}

int run_build_with_progress(const char *cmd, const char *source_dir) {
    int total_files = count_source_files(source_dir);
    total_files = (total_files * 85) / 100;
    if (total_files == 0) total_files = 1;

    initscr();
    cbreak();
    noecho();
    curs_set(0); // Ocultar cursor
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK); 
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
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
    
    // Split layout: 70% Log, 30% Stats
    int log_width = (width * 70) / 100;
    int stats_width = width - log_width;
    
    WINDOW *log_win = newwin(log_height, log_width, 2, 0);
    WINDOW *stats_win = newwin(log_height, stats_width, 2, log_width);
    
    WINDOW *sep2_win = newwin(sep2_height, width, height - 2, 0);
    WINDOW *bar_win = newwin(bar_height, width, height - 1, 0);

    scrollok(log_win, TRUE);
    
    // Initial stats draw
    int cpu_count = get_cpu_count();
    draw_system_load(stats_win, cpu_count);

   
    char header_text[256];
    snprintf(header_text, sizeof(header_text), _("Kernel Installer Version %s %s"), APP_VERSION, _("by Alexia Michelle <https://github.com/alexiarstein/kernelinstall>"));
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

    char line[1024];
    int current_count = 0;
    int last_percent = 0; // Para evitar que la barra retroceda
    int packaging_started = 0;
    time_t packaging_start_time = 0;
    char current_status_msg[256] = ""; 

    int fd = fileno(build_pipe);
    fd_set readfds;
    struct timeval timeout; 
    time_t last_stats_update = time(NULL); 

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        
        // Timeout de 1 segundo para actualizar el reloj
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (ret == -1) {
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

                // recalculando el split
                log_width = (width * 70) / 100;
                stats_width = width - log_width;
                wresize(log_win, log_height, log_width);
                mvwin(log_win, 2, 0);
                wresize(stats_win, log_height, stats_width);
                mvwin(stats_win, 2, log_width);
                
                werase(header_win);
                snprintf(header_text, sizeof(header_text), _("Kernel Installer Version %s %s"), APP_VERSION, _("by Alexia Michelle <https://github.com/alexiarstein/kernelinstall>"));
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
                draw_system_load(stats_win, cpu_count);
                
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
        } else if (ret == 0) {
            if (packaging_started) {
                update_packaging_timer(bar_win, packaging_start_time, current_status_msg, sizeof(current_status_msg));
            }
            
            // Actualizar stats cada 2 segundos (aprox)
            time_t now = time(NULL);
            if (difftime(now, last_stats_update) >= 2.0) {
                draw_system_load(stats_win, cpu_count);
                last_stats_update = now;
            }
            continue;
        }
        if (fgets(line, sizeof(line), build_pipe) == NULL) {
            break;
        }
        wprintw(log_win, "%s", line);
        wrefresh(log_win);

        if (strstr(line, " CC ") || strstr(line, " LD ") || strstr(line, " AR ") ||
            strstr(line, " SIGN ") || strstr(line, " INSTALL ") || strstr(line, " DEPMOD ") ||
            strstr(line, " XZ ")) {
            current_count++;
            int percent = (current_count * 100) / total_files;
            
            if (strstr(line, " LD ") && strstr(line, "vmlinux")) {
                if (percent < 90) percent = 90;
            }
            if (strstr(line, " XZ ") && strstr(line, "arch/")) {
                if (percent < 95) percent = 95;
            }

            if (percent > 99) percent = 99;
            
            // potencial fix para eliminar el glitch en la progressbar (veremos que pasa)
            if (percent < last_percent) percent = last_percent;
            last_percent = percent;

            if (!packaging_started) {
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
        }

        
        if (!packaging_started) {
            if (strstr(line, "dpkg-deb: building package") || strstr(line, "dpkg-deb: construyendo el paquete")) {    
                packaging_started = 1;
                packaging_start_time = time(NULL);
                snprintf(current_status_msg, sizeof(current_status_msg), "%s", _("Building kernel and kernel headers .deb package. Please wait..."));
                werase(bar_win);
                if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(bar_win, 0, 0, "%s", current_status_msg);
                if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
                wrefresh(bar_win);
            } else if (strstr(line, "Processing files:") || strstr(line, "rpmbuild")) {
                packaging_started = 1;
                packaging_start_time = time(NULL);
                snprintf(current_status_msg, sizeof(current_status_msg), "%s", _("Building kernel .rpm package. Please wait..."));
                werase(bar_win);
                if (has_colors()) wattron(bar_win, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(bar_win, 0, 0, "%s", current_status_msg);
                if (has_colors()) wattroff(bar_win, COLOR_PAIR(2) | A_BOLD);
                wrefresh(bar_win);
            }
        } else {
            update_packaging_timer(bar_win, packaging_start_time, current_status_msg, sizeof(current_status_msg));
            
            time_t now = time(NULL);
            if (difftime(now, last_stats_update) >= 2.0) {
                draw_system_load(stats_win, cpu_count);
                last_stats_update = now;
            }
        }
    }

    endwin(); // Restaurar terminal
    return pclose(build_pipe);
}

void package_headers_deb(const char* home, const char* version, const char* tag) {
    char cmd[16384];
    char full_version[128];
    snprintf(full_version, sizeof(full_version), "%s%s", version, tag);
    
    char source_dir[1024];
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);
    
    char deb_dir[2048];
    snprintf(deb_dir, sizeof(deb_dir), "%s/debian/linux-headers", source_dir);
    
    char header_dir[4096];
    snprintf(header_dir, sizeof(header_dir), "%s/usr/src/linux-headers-%s", deb_dir, full_version);

    printf(_("Creating custom headers package for %s...\n"), full_version);

    // fix: limpiamos los archivos anteriores para evitar artifacts
    snprintf(cmd, sizeof(cmd), "rm -rf %s/debian/linux-headers", source_dir);
    run(cmd);
    
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", header_dir);
    run(cmd);

    // fix: preparamos el header
    snprintf(cmd, sizeof(cmd), "cd %s && make modules_prepare", source_dir);
    run(cmd);

 

    // Para que vmware no se queje que le faltan cosas a las cabeceras del kernel.
    snprintf(cmd, sizeof(cmd), "cd %s && cp .config Module.symvers Kconfig Makefile System.map %s/", source_dir, header_dir);
    run(cmd);
    // incluimos tooodo <en voz del loco del TC>
    snprintf(cmd, sizeof(cmd), "cd %s && cp -r arch scripts tools include %s/", source_dir, header_dir);
    run(cmd);

    snprintf(cmd, sizeof(cmd), 
             "if [ -f %s/tools/objtool/objtool ]; then "
             "mkdir -p %s/tools/objtool; "
             "cp %s/tools/objtool/objtool %s/tools/objtool/; "
             "fi", 
             source_dir, header_dir, source_dir, header_dir);
    run(cmd);

    // Mejora: Empaquetamos de la forma 'moderna'
    char control_dir[4096];
    snprintf(control_dir, sizeof(control_dir), "%s/DEBIAN", deb_dir);
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", control_dir);
    run(cmd);

    char control_file[8192];
    snprintf(control_file, sizeof(control_file), "%s/control", control_dir);
    
    FILE *f = fopen(control_file, "w");
    if (f) {
        fprintf(f, "Package: linux-headers-%s\n", full_version);
        fprintf(f, "Version: %s-1\n", full_version);
        fprintf(f, "Architecture: amd64\n");
        fprintf(f, "Maintainer: Alexia <root@goldendog.lan>\n"); // TODO: Cambiar por el nombre del usuario
        fprintf(f, "Section: kernel\n");
        fprintf(f, "Priority: optional\n");
        fprintf(f, "Provides: linux-headers\n");
        fprintf(f, "Description: Custom Linux headers %s (compatible with VMware/NVIDIA/VirtualBox)\n", full_version);
        fprintf(f, " Custom kernel headers completos para la versión %s.\n", full_version);
        fclose(f);
    } else {
        perror("Failed to create control file");
    }

    // Mejora: Creamos un postinst para que el usuario no tenga que preocuparse por los enlaces simbólicos
    char postinst_file[8192];
    snprintf(postinst_file, sizeof(postinst_file), "%s/postinst", control_dir);
    
    FILE *fp = fopen(postinst_file, "w");
    if (fp) {
        fprintf(fp, "#!/bin/sh\n");
        fprintf(fp, "set -e\n");
        fprintf(fp, "if [ \"$1\" = \"configure\" ]; then\n");
        fprintf(fp, "    echo \"Setting up symlinks for kernel %s...\"\n", full_version);
        fprintf(fp, "    # Ensure the build symlink points to these headers\n");
        fprintf(fp, "    if [ -d /lib/modules/%s ]; then\n", full_version);
        fprintf(fp, "        ln -sf /usr/src/linux-headers-%s /lib/modules/%s/build\n", full_version, full_version);
        fprintf(fp, "        ln -sf /usr/src/linux-headers-%s /lib/modules/%s/source\n", full_version, full_version);
        fprintf(fp, "    fi\n");
        fprintf(fp, "fi\n");
        fprintf(fp, "exit 0\n");
        fclose(fp);
        
        snprintf(cmd, sizeof(cmd), "chmod 0755 %s", postinst_file);
        run(cmd);
    } else {
        perror("Failed to create postinst file");
    }

    snprintf(cmd, sizeof(cmd), "dpkg-deb --build %s %s/kernel_build/linux-headers-%s_custom_amd64.deb", deb_dir, home, full_version);
    run(cmd);
}

void install_debian_based_kernel(const char* home, const char* version, const char* tag) {
    char cmd[8192];
    char source_dir[1024];
    
    snprintf(source_dir, sizeof(source_dir), "%s/kernel_build/linux-%s", home, version);
    
    snprintf(cmd, sizeof(cmd),
             "cd %s && LC_ALL=C stdbuf -oL -eL fakeroot make -j$(nproc) bindeb-pkg",
             source_dir);
    run_build_with_progress(cmd, source_dir);
    
   
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-image-%s*%s*.deb",
             home, version, tag);
    run(cmd);

    package_headers_deb(home, version, tag);
    
    char full_version[128];
    snprintf(full_version, sizeof(full_version), "%s%s", version, tag);
    snprintf(cmd, sizeof(cmd),
             "cd %s/kernel_build && "
             "sudo dpkg -i linux-headers-%s_custom_amd64.deb",
             home, full_version);
    run(cmd);
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

// NUEVA FUNCIÓN: Secure Boot para Mint/Ubuntu
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
    
    if (home == NULL) {
        fprintf(stderr, _("Could not determine home directory\n"));
        exit(EXIT_FAILURE);
    }

    // Detectar distro y obtener operaciones
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

    // Instalar las dependencias específicas de la distro.
    printf(_("Installing required packages for %s...\n"), ops->name);
    ops->install_dependencies();

    // Para Mint/Ubuntu: generar certificado GoldenDogLinux
    if (distro == DISTRO_MINT) {
        mint_generate_certificate();
    }


    // Descargar la versión más reciente del kernel
    // DESARROLLADORES/AS: si quieren que descargue una versión específica, pueden modificar el archivo kernelver.txt
    // y se descarga el archivo comprimido en lugar de un git clone por que sinó tarda mucho.
    // - Alexia.
    printf(_("Fetching latest kernel version from kernel.org...\n"));

    char tmp_file[512];
    snprintf(tmp_file, sizeof(tmp_file), "%s/kernel_build/kernelver.txt", home);
    
    char fetch_cmd[1024];
    snprintf(fetch_cmd, sizeof(fetch_cmd),
             "curl -s https://www.kernel.org/ | "
             "grep -A1 'latest_link' | grep -oE '[0-9]+\\.[0-9]+(\\.[0-9]+)?' | "
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

  
    char cmd[1024];
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


    printf(_("Building and installing kernel for %s...\n"), ops->name);
    ops->build_and_install(home, latest, TAG);

    
    printf(_("Updating bootloader for %s...\n"), ops->name);
    ops->update_bootloader();

    // Para Mint/Ubuntu: ofrecer enrolar la maquina con Secure Boot
    if (distro == DISTRO_MINT) {
        if (mint_ask_secure_boot_enrollment() == 0) {
            mint_enroll_secure_boot_key();
        } else {
            printf(_("Secure Boot enrollment skipped.\n"));
            printf(_("You can enroll the certificate later with: sudo mokutil --import /var/lib/shim-signed/mok/MOK_goldendoglinux.der\n"));
        }
    }

    // Limpieza final.
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
