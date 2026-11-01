# SquooshRE

SquooshRE is an image compression web app that reduces image sizes through numerous formats. It is a fork of [Squoosh] by Google Chrome Labs, rebuilt on Vue 3 and Vite with updated codecs.

# Privacy

SquooshRE does not send your image to a server. All image compression processes locally.

SquooshRE does not use analytics or collect any data about you.

# Developing

To develop for SquooshRE:

1. Install [Bun](https://bun.sh) (1.3 or later)
1. Clone the repository
1. To install dependencies, run:
   ```sh
   bun install
   ```
1. Start the development server by running:
   ```sh
   bun run dev
   ```
   It serves on port 5000 (`--port` picks another one).

Other scripts:

- `bun run build` type-checks and builds the app into `dist`.
- `bun run preview` serves that build, including the service worker.
- `bun run typecheck` only type-checks.

The app is Vue 3 + Vite. The codecs in `codecs` are prebuilt and run in a
web worker; see [src/features/README.md](/src/features/README.md) for how
they plug into the UI.

# Contributing

SquooshRE is an open-source project that appreciates all community involvement. To contribute to the project, follow the [contribute guide](/CONTRIBUTING.md).

[squoosh]: https://squoosh.app
