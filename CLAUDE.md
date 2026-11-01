# SquooshRE (fork of Squoosh)

Image compression web app, renamed SquooshRE (the logo's pink "RE" is Outfit
Bold, OFL; "Squoosh" stays in the original Google Sans subset). This fork rewrote the UI from Preact + Rollup to
**Vue 3 + Vite + TypeScript** (merged into `dev` in November 2026) and is
modernizing the codecs. `dev` is the main branch. Owner: Julian (`Pinguin2001` on GitHub, remote
`origin`; `upstream` is GoogleChromeLabs/squoosh and is never pushed to).

Planned hosting: Cloudflare Pages at `squoosh.julianhasreiter.eu` (needs the
COOP/COEP headers from `public/_headers`; GitHub Pages can't send them).

## Commands

```sh
bun install --frozen-lockfile
bun run dev        # Vite on :5000, sends COOP/COEP
bun run build      # vue-tsc -b && vite build → dist/
bun run preview    # serves dist/ incl. the service worker, :5000
bun run typecheck  # vue-tsc -b
```

`.claude/launch.json` has `dev` and `preview` for the browser pane. If a build
fails inside `workbox-build` (lru-cache / path-scurry), the node_modules tree
is stale: `rm -rf node_modules && bun install --frozen-lockfile`.

## Layout

- `src/App.vue`: intro vs editor, `/editor` pushState + popstate, drag & drop
  and paste anywhere, share target, SW registration.
- `src/components/`: `controls/` (Checkbox, Toggle, Revealer, Select, Range,
  Expander; all `v-model`), `editor/` (CompressView, ImageOutput,
  ResultsBubble, OptionsPanel, MultiPanel), `intro/`, SnackBar,
  LoadingSpinner, `icons.ts` (render functions).
- `src/composables/`: `useCompressor` (the whole image pipeline),
  `usePanZoom`, `useSnackbar` (`showSnack` is a plain import),
  `useDerivedOptions`, `useMediaQuery`.
- `src/features/`: per codec `meta.ts` (options + defaults), `worker.ts`
  (runs in the web worker), `Options.vue`. `registry.ts` lists every
  encoder/processor; `options-components.ts` maps encoders to panels.
  `src/features/README.md` explains adding an encoder.
- `src/worker/index.ts` exposes all worker functions via comlink;
  `src/worker/bridge.ts` (WorkerBridge: queue, abort, 10 s idle timeout) has
  an explicit `methodNames` list that must match (compile-time checked).
- `src/sw.ts`: Workbox service worker (injectManifest).
- `codecs/`: prebuilt wasm codecs, committed, built with Docker.
- `docs/codec-modernization-plan.md`: codec plan (phases 1–3 done, 4 open).

## How things work (non-obvious)

- **Settings are edited in place** (Vue reactive). `useCompressor` detects
  changes by comparing serialized settings (`JSON.stringify`); disabled
  processors are left out of the key. The result cache uses the same keys.
  Defaults in `registry.ts` are factory functions so nothing shares objects.
- **`encoderMap` keys are stored in saved side settings** (localStorage
  `leftSideSettings` / `rightSideSettings`): never rename them. Stored
  settings are sanitized; unknown/unsupported encoders load as "Original
  image".
- **Decoding is native-first**: AVIF and WebP only via the browser; JPEG XL
  and QOI have wasm fallbacks that load only when the browser can't decode.
- **Browser encoders** (Browser AVIF/JPEG/PNG/WebP) are feature-tested with
  a 1×1 `toDataURL` probe and hidden when unsupported. Browsers silently fall
  back to PNG (or JPEG in Safari) for unsupported types; `canvasEncode`
  throws on that. Safari has no Browser WebP; Chrome has no Browser AVIF.
- **Multithreaded codecs need `crossOriginIsolated === true`**, otherwise
  they silently use the single-threaded builds.
- Codecs are imported dynamically in workers so the worker starts without
  them. Type-only imports use `import type` (`verbatimModuleSyntax`).
- **Service worker**: precaches the build except `.wasm`; wasm is cached
  cache-first and all at once when the editor opens (`CACHE_ALL` message);
  pruned on activate. Demo images cached at runtime. Update prompt via
  `virtual:pwa-register`. The SW only runs in `preview`/production.
- Scoped CSS uses native nesting; class names are kebab-case copies of the
  old CSS modules. Shared option styles and side themes are global
  (`src/styles/options.css`).
- **Material 3 Expressive look** (agreed departure from the original UI):
  `src/styles/tokens.css` holds the colour roles (generated once with
  Material Color Utilities from pink #ff3385 = primary and blue #5fb4e4 =
  tertiary; dark scheme follows the system only), shape, type scale and
  spring motion (`linear()` curves). `src/styles/shapes.css` has the
  expressive shapes as 64-point polygons so `clip-path` can morph between
  them. Side themes set `--side-accent`/`--side-container` (+ `on-` pairs):
  left = primary, right = tertiary. Font: Roboto Flex (latin, wght axis,
  OFL) self-hosted in `src/assets/fonts/`, precached by the SW. Root font
  size is 100% (was 12px).

## Building codecs

Needs Docker. On this Mac: Colima (`colima start --cpu 8 --memory 8
--vm-type vz --vz-rosetta`; `colima stop` when done). Build a codec with
`cd codecs/<name> && ../build-cpp.sh` (C++) or `../build-rust.sh [./build.sh]`
(Rust). Images: Emscripten 6.0.13 (`codecs/cpp.Dockerfile`), Rust 1.99 +
nightly-2026-10-01 for threaded OxiPNG (`codecs/rust.Dockerfile`,
`codecs/oxipng/rust-toolchain`). Make only rebuilds missing outputs: delete
the `.js` output to relink, or the codec's `node_modules/.../build` dir to
reconfigure. `chmod 644` the `.wasm` files afterwards (the container makes
them executable). Pitfalls already solved, keep them:

- Links use `emcc`, so `LDFLAGS` has `-s DEFAULT_TO_CXX`.
- `-s STACK_SIZE=5MB -s DEFAULT_PTHREAD_STACK_SIZE=2MB`: with the 64 KB
  default libaom overflows and threaded AVIF throws `unwind`.
- SIMD builds must compile **and link** with `-msimd128` or LTO drops SIMD
  silently. Verify: `wasm-dis --all-features x.wasm | grep -c v128` (> 0).
- autoconf (MozJPEG): `emconfigure ./configure --host=wasm32-unknown-emscripten`.
- Use `$(AR)`, not `llvm-ar` (not on PATH).
- Threaded Rust needs the shared-memory link args in `codecs/oxipng/build.sh`.
- Only SIMD builds are shipped (WebP, JPEG XL); threaded/single-threaded
  split stays because threads need cross-origin isolation.
- libjxl 0.12 is built once with CMake (`jxl jxl_dec jxl_cms jxl_threads`).
  The glue uses the public `encode.h`/`decode.h` API;
  `JxlColorEncodingSetToSRGB` is not in the decoder library. Highway
  already emits only Wasm SIMD kernels (`HWY_COMPILE_ONLY_STATIC` changes
  nothing), so the 2.6 MB encoder is libjxl itself.
- Jpegli (`codecs/jpegli`) is pinned to a google/jpegli commit (no
  releases yet) and only builds `jpegli-static` + `hwy`, single-threaded.
  XYB mode isn't exposed: it needs the caller to convert colours and write
  the ICC profile (see jpegli's `lib/extras/enc/jpegli.cc`).
- libimagequant 4 and resize/hqx/rotate/OxiPNG are Rust (`build-rust.sh`).
- The pre-commit hook runs prettier, clang-format (`brew install
clang-format`) and rustfmt; a missing formatter fails the commit.

## Verifying changes

- Compare **in the same browser/OS** (SVG rasterization differs between
  Windows and macOS). Logo demo, defaults, in the macOS browser pane, exact
  bytes after the phase 4 upgrades: AVIF 8379, Jpegli 22093, JPEG XL 23332,
  MozJPEG 18788, OxiPNG 29267, QOI 40566, WebP 14010, Browser WebP ~16.0 kB. Before
  libimagequant 4 (squoosh.app still gives these): OxiPNG + reduce palette
  17586, + resize 50 % Lanczos3 17554, hqx 14879. Codec rebuilds without
  version changes must stay byte-identical. (The same demo gives different
  sizes on Windows, e.g. OxiPNG 45.1 kB.)
- Test on `localhost`, not `127.0.0.1`. Even there, nested workers fail
  while the pane is hidden, so threaded codecs (AVIF/JXL mt, parallel
  OxiPNG) hang in the app with no error; the old builds behave the same.
  Check `*_mt.js` builds by importing them on the page's main thread.
- Benchmark old vs new codec builds by copying the old outputs into
  `node_modules/.squooshre-bench/` (git-ignored, served by Vite) and
  importing both with `{ noInitialRun: true }`.
- Drive the UI with real input events in the browser pane; Vite reloads
  the page whenever files under `codecs/` change (scripts get cut off).
- The pane's main tab emulates dark mode, which makes the page background
  look dark; use `resize_window colorScheme: light` + reload to compare.
- Unregister service workers and clear caches when switching between
  `dev` and `preview` on port 5000.

## Open items

- Codec plan phase 4 is done (one commit per codec, with old-vs-new
  numbers in each commit message). Open questions are at the end of
  `docs/codec-modernization-plan.md`.
- Unreproduced: a `RangeError: offset is out of bounds` in the resize worker
  and one oversized Lanczos result after very large hqx resizes (hqx used
  to over-allocate 4×; fixed, so possibly gone).
- Not tested: Safari/Firefox after the codec rebuild, two-finger pinch on
  touch devices, the install button.

## Working with Julian

- Commit only when asked; one commit per step, messages like
  `wip: …` during the rewrite, descriptive subject + bullet body. End with
  the Co-Authored-By line.
- Keep the look and behaviour of the original app 1:1 unless agreed.
- Decisions from the rewrite: no analytics (README privacy section says so),
  keep the PWA (offline, install, share target), no prerendering (plain
  SPA), no vue-router or Pinia (two views, state in `useCompressor`),
  `browserGIF` and WebP v2 removed, custom elements replaced by Vue
  components, generated codec files replaced by `registry.ts`.
