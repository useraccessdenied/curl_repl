CC       ?= cc
CFLAGS   = -std=c11 -Wall -Wextra -g $(shell curl-config --cflags)
LIBS     = $(shell curl-config --libs)
TARGET   = curl_repl
SRCDIR   = src
BUILDDIR = build
SRCS     = $(SRCDIR)/main.c \
           $(SRCDIR)/repl.c \
           $(SRCDIR)/protocol.c \
           $(SRCDIR)/command.c \
           $(SRCDIR)/ws.c \
           $(SRCDIR)/telnet.c \
           $(SRCDIR)/mqtt.c \
           $(SRCDIR)/script.c
OBJS     = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

all: $(TARGET)

# --- macOS universal binary (fat binary: arm64 + x86_64) ---
.PHONY: macos-universal
macos-universal: CFLAGS += -arch arm64 -arch x86_64
macos-universal: LIBS := $(shell curl-config --libs)
macos-universal: $(TARGET)

# --- Cross-compilation helper ---
# Override CC, CFLAGS, LIBS per target, then rebuild.
# Use curl-config from the cross-toolchain prefix, fall back to pkg-config.
#
# Example:  make cross TARGET_ARCH=aarch64-linux-gnu
#           make cross TARGET_ARCH=x86_64-w64-mingw32  PREFIX=/usr/…
#
.PHONY: cross
cross: CC       = $(TARGET_ARCH)-gcc
cross: CFLAGS  += --sysroot=/usr/$(TARGET_ARCH)
cross: LIBS     = $(shell $(TARGET_ARCH)-pkg-config --libs libcurl 2>/dev/null || echo "-lcurl")
cross: CFLAGS  += $(shell $(TARGET_ARCH)-pkg-config --cflags libcurl 2>/dev/null)
cross: $(TARGET)

# --- Convenience cross-build targets (CI uses these) ---
LINUX_CROSS_TARGETS = x86_64-linux-gnu i686-linux-gnu aarch64-linux-gnu arm-linux-gnueabihf
WINDOWS_CROSS_TARGETS = x86_64-w64-mingw32 i686-w64-mingw32 aarch64-w64-mingw32

.PHONY: $(LINUX_CROSS_TARGETS:%=cross-linux-%)
cross-linux-%: TARGET_ARCH = $*
cross-linux-%: TARGET_SUFFIX =
cross-linux-%: cross
	@mv $(TARGET) $(TARGET)-$(TARGET_ARCH)$(TARGET_SUFFIX)

.PHONY: $(WINDOWS_CROSS_TARGETS:%=cross-windows-%)
cross-windows-%: TARGET_ARCH = $*
cross-windows-%: TARGET_SUFFIX = .exe
cross-windows-%: cross
	@mv $(TARGET).exe $(TARGET)-$(TARGET_ARCH)$(TARGET_SUFFIX)

# --- Build all dist artifacts ---
ARCHS_LINUX   = x86_64-linux-gnu aarch64-linux-gnu arm-linux-gnueabihf i686-linux-gnu
ARCHS_WINDOWS = x86_64-w64-mingw32 i686-w64-mingw32 aarch64-w64-mingw32
DIST_DIR      = dist
DIST_TARGETS  = $(ARCHS_LINUX:%=$(DIST_DIR)/$(TARGET)-%) \
                $(ARCHS_WINDOWS:%=$(DIST_DIR)/$(TARGET)-%.exe)

ifeq ($(shell uname), Darwin)
  DIST_TARGETS += $(DIST_DIR)/$(TARGET)-macos-universal
endif

.PHONY: dist
dist: | $(DIST_DIR)
dist: $(DIST_TARGETS)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

$(DIST_DIR)/$(TARGET)-macos-universal: macos-universal
	cp $(TARGET) $@
	rm -f $(TARGET)

# Pattern rules for cross-compiled binaries
$(DIST_DIR)/$(TARGET)-%: cross-linux-%
	cp $(TARGET)-$* $@
	rm -f $(TARGET)-$*

$(DIST_DIR)/$(TARGET)-%.exe: cross-windows-%
	cp $(TARGET)-$* $@
	rm -f $(TARGET)-$*
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

checksrc:
	perl scripts/checksrc.pl $(SRCDIR)/*.c $(SRCDIR)/*.h

clean:
	rm -rf $(BUILDDIR) $(TARGET) dist

.PHONY: all checksrc clean macos-universal cross dist \
        $(LINUX_CROSS_TARGETS:%=cross-linux-%) \
        $(WINDOWS_CROSS_TARGETS:%=cross-windows-%)
