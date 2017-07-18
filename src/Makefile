
CC = /usr/bin/x86_64-w64-mingw32-gcc
LD = /usr/bin/x86_64-w64-mingw32-ld
STRIP = /usr/bin/x86_64-w64-mingw32-strip

LIBGCRYPT_CONFIG = /usr/x86_64-w64-mingw32/bin/libgcrypt-config

DEPDIR = ./.deps

CFLAGS = -Wall -O3 `$(LIBGCRYPT_CONFIG) --cflags`
LDFLAGS = -lwinscard `$(LIBGCRYPT_CONFIG) --libs`

OBJS_COMMON = scard_common.o felica_lites.o mac_fls.o key_db.o
OBJS = VerifyMAC.o WriteCK.o $(OBJS_COMMON)

DEPS = $(patsubst %.o,$(DEPDIR)/%.d,$(OBJS))
SRCS = $(patsubst %.o,%.c,$(OBJS))

BINARIES = VerifyMAC.exe WriteCK.exe

.PHONY: all depend clean distclean

%.o: %.c
	@echo 'CC' $< '=>' $@
	@$(CC) $(CFLAGS) -c -o $@ $<

all: $(BINARIES)

$(DEPDIR)/%.d: %.c
	@mkdir -p $(DEPDIR)/
	@echo "  "$< '=>' $@
	@$(CC) $(CFLAGS) -MM -o $@ $<

ifeq ($(findstring $(MAKECMDGOALS), clean distclean),)
  -include $(DEPS)
endif

VerifyMAC.exe: VerifyMAC.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@

WriteCK.exe: WriteCK.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@

clean:
	rm -f $(OBJS) $(BINARIES)

distclean:
	rm -f $(OBJS) $(BINARIES)
	rm -rf $(DEPDIR)

#EOF
