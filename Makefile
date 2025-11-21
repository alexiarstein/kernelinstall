# Kernel Installer Makefile
# Author: Alexia Michelle <alexia@goldendoglinux.org>
# LICENSE: GNU GPL 3.0
# --------------------------------------------------

CC = gcc
PKG_CONFIG ?= pkg-config
NCURSES_CFLAGS := $(shell $(PKG_CONFIG) --cflags ncursesw 2>/dev/null)
NCURSES_LIBS := $(shell $(PKG_CONFIG) --libs ncursesw 2>/dev/null)

ifeq ($(NCURSES_LIBS),)
    NCURSES_CFLAGS = -D_GNU_SOURCE
    NCURSES_LIBS = -lncursesw
endif

CFLAGS = -Wall -Wextra -g -D_FORTIFY_SOURCE=2 $(NCURSES_CFLAGS)
LDFLAGS = $(NCURSES_LIBS)
OBJ = kernel-install.o
TARGET = kernel-installer
DISTRO_DIR = distro
DISTRO_HEADERS = $(DISTRO_DIR)/common.h $(DISTRO_DIR)/debian.h $(DISTRO_DIR)/linuxmint.h $(DISTRO_DIR)/fedora.h $(DISTRO_DIR)/distros.h

# Reglas de compilación
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

kernel-install.o: kernel-install.c $(DISTRO_HEADERS)
	$(CC) $(CFLAGS) -c kernel-install.c -o kernel-install.o

# Reglas de internacionalización - ACTUALIZADA
update-po:
	xgettext --from-code=UTF-8 -k_ -kN_ -o po/kernel-install.pot kernel-install.c $(DISTRO_HEADERS)
	msgmerge -U po/es.po po/kernel-install.pot

compile-mo:
	mkdir -p locale/es/LC_MESSAGES
	msgfmt po/es.po -o locale/es/LC_MESSAGES/kernel-install.mo

install: $(TARGET) compile-mo
	cp $(TARGET) /usr/local/bin/
	cp -r locale/ /usr/local/share/

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -rf /usr/local/share/locale/*/LC_MESSAGES/kernel-install.mo

clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf locale/

.PHONY: all install uninstall clean update-po compile-mo
