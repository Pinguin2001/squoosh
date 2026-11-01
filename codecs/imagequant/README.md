# ImageQuant

- Source: <https://github.com/ImageOptim/libimagequant> (the `imagequant` crate)
- Version: 4.4.1
- License: GPL-3.0-or-later

Built with `../build-rust.sh` (wasm-pack) into `pkg/`.

## API

### `quantize(data, width, height, numColors, dither): Uint8ClampedArray`

Reduces RGBA pixels to at most `numColors` colours, with dithering from 0
to 1, and returns RGBA pixels again.

### `zx_quantize(data, width, height, dither): Uint8ClampedArray`

Mimics the ZX Spectrum: each 8×8 block gets two colours from the ZX palette,
both "regular" or both "bright".
