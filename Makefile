CFLAGS = -Os -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -s

.PHONY: all
all: aarch64-macos-target-features \
     aarch64-linux-target-features

aarch64-macos-target-features: aarch64-target-features.c
	zig cc -target aarch64-macos-none $(CFLAGS) $< -o $@ $(LDFLAGS)

aarch64-linux-target-features: aarch64-target-features.c
	zig cc -target aarch64-linux-musl $(CFLAGS) $< -o $@ -static $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *-target-features
