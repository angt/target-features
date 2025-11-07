CFLAGS = -Os -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -s

.PHONY: all
all: aarch64-macos-target-features x86_64-macos-target-features \
     aarch64-linux-target-features x86_64-linux-target-features

%-linux-target-features: %-target-features.c
	zig cc -target $*-linux-musl $(CFLAGS) $< -o $@ -static $(LDFLAGS)

%-macos-target-features: %-target-features.c
	zig cc -target $*-macos-none $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *-target-features
