CFLAGS = -Os -ffunction-sections -fdata-sections
LDFLAGS = -static -Wl,--gc-sections -s

aarch64-target-features: CC = aarch64-linux-musl-gcc

.PHONY: all
all: aarch64-target-features

.PHONY: clean
clean:
	rm -f aarch64-target-features
