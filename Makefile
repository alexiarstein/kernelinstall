# Makefile for Alexia Kernel Installer
# Author: Alexia Michelle <alexia@goldendoglinux.org>

# Variables
PACKAGE = kernel-install
VERSION = 1.0.0
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = 
SRC = kernel-install.c
OBJ = $(SRC:.c=.o)
BINDIR = /usr/local/bin
LOCALEDIR = /usr/local/share/locale
PODIR = po
MOFILES = $(wildcard $(PODIR)/*.po)
LOCALES = $(patsubst $(PODIR)/%.po,%,$(MOFILES))

# Default target
all: $(PACKAGE) translations

# Build the main binary
$(PACKAGE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Generate POT file (translation template)
pot: $(SRC)
	xgettext --keyword=_ --keyword=N_ --package-name="$(PACKAGE)" \
	         --package-version="$(VERSION)" --output=$(PODIR)/$(PACKAGE).pot $(SRC)

# Compile translations
translations: $(MOFILES)
	@for lang in $(LOCALES); do \
		echo "Compiling $$lang translation..."; \
		mkdir -p locale/$$lang/LC_MESSAGES; \
		msgfmt $(PODIR)/$$lang.po -o locale/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done

# Create a new translation file
init-po:
	@if [ -z "$(LANG)" ]; then \
		echo "Usage: make init-po LANG=xx"; \
		exit 1; \
	fi
	msginit --no-translator --locale=$(LANG).UTF-8 \
	        --input=$(PODIR)/$(PACKAGE).pot --output=$(PODIR)/$(LANG).po

# Update existing translation files
update-po: pot
	@for po in $(MOFILES); do \
		lang=$$(basename $$po .po); \
		echo "Updating $$lang translation..."; \
		msgmerge --update $$po $(PODIR)/$(PACKAGE).pot; \
	done

# Install the program and translations
install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(PACKAGE) $(DESTDIR)$(BINDIR)/
	@for lang in $(LOCALES); do \
		if [ -f "locale/$$lang/LC_MESSAGES/$(PACKAGE).mo" ]; then \
			echo "Installing $$lang translation..."; \
			install -d $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES; \
			install -m 644 locale/$$lang/LC_MESSAGES/$(PACKAGE).mo \
			         $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/; \
		fi; \
	done
	@echo ""
	@echo "Installation complete!"
	@echo "Binary installed to: $(BINDIR)/$(PACKAGE)"
	@echo "Translations installed to: $(LOCALEDIR)"
	@echo ""
	@echo "You can now run: $(PACKAGE)"

# Uninstall the program
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PACKAGE)
	@for lang in $(LOCALES); do \
		rm -f $(DESTDIR)$(LOCALEDIR)/$$lang/LC_MESSAGES/$(PACKAGE).mo; \
	done
	@echo "Uninstallation complete!"

# Clean build files
clean:
	rm -f $(PACKAGE) $(OBJ)
	rm -rf locale

# Clean everything including translations
distclean: clean
	rm -f $(PODIR)/$(PACKAGE).pot

# Create a distribution tarball
dist: distclean
	mkdir -p $(PACKAGE)-$(VERSION)
	cp -r $(SRC) Makefile README.md LICENSE $(PODIR) $(PACKAGE)-$(VERSION)/
	tar -czf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)/
	rm -rf $(PACKAGE)-$(VERSION)
	@echo "Distribution package created: $(PACKAGE)-$(VERSION).tar.gz"

# Test the build
test: all
	@echo "Testing the build..."
	@./$(PACKAGE) --help || true

# Show help
help:
	@echo "Alexia Kernel Installer Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build the program and translations (default)"
	@echo "  install      - Install to /usr/local/bin"
	@echo "  uninstall    - Remove installed files"
	@echo "  clean        - Remove build files"
	@echo "  distclean    - Remove all generated files"
	@echo "  pot          - Generate translation template"
	@echo "  translations - Compile all translations"
	@echo "  init-po      - Create new translation (use: make init-po LANG=xx)"
	@echo "  update-po    - Update existing translations with new strings"
	@echo "  dist         - Create distribution tarball"
	@echo "  test         - Test the build"
	@echo "  help         - Show this help message"

.PHONY: all install uninstall clean distclean dist pot translations init-po update-po test help
