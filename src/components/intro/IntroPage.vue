<script setup lang="ts">
import { onBeforeUnmount, onMounted, ref, useTemplateRef } from 'vue';
import LoadingSpinner from '@/components/LoadingSpinner.vue';
import Menu from '@/components/controls/Menu.vue';
import { ExpandIcon } from '@/components/icons';
import { showSnack } from '@/composables/useSnackbar';
import Base64Dialog from './Base64Dialog.vue';
import SlideOnScroll from './SlideOnScroll.vue';
import { startBlobs } from './blob-anim/meta';
import githubLogo from './imgs/github-logo.svg';
import logoWithText from './imgs/logo-with-text.svg';
import logo from './imgs/logo.svg';
import largePhoto from './imgs/demos/demo-large-photo.jpg';
import artwork from './imgs/demos/demo-artwork.jpg';
import deviceScreen from './imgs/demos/demo-device-screen.png';
import largePhotoIcon from './imgs/demos/icon-demo-large-photo.jpg';
import artworkIcon from './imgs/demos/icon-demo-artwork.jpg';
import deviceScreenIcon from './imgs/demos/icon-demo-device-screen.jpg';
import logoIcon from './imgs/demos/icon-demo-logo.png';
import smallSectionAsset from './imgs/info-content/small.svg';
import simpleSectionAsset from './imgs/info-content/simple.svg';
import secureSectionAsset from './imgs/info-content/secure.svg';

const emit = defineEmits<{ file: [file: File] }>();

const demos = [
  {
    description: 'Large photo',
    size: '2.8MB',
    filename: 'photo.jpg',
    shape: 'cookie-9',
    url: largePhoto,
    iconUrl: largePhotoIcon,
  },
  {
    description: 'Artwork',
    size: '2.9MB',
    filename: 'art.jpg',
    shape: 'clover-4',
    url: artwork,
    iconUrl: artworkIcon,
  },
  {
    description: 'Device screen',
    size: '1.6MB',
    filename: 'pixel3.png',
    shape: 'cookie-4',
    url: deviceScreen,
    iconUrl: deviceScreenIcon,
  },
  {
    description: 'SVG icon',
    size: '13KB',
    filename: 'squoosh.svg',
    shape: 'pentagon',
    url: logo,
    iconUrl: logoIcon,
  },
] as const;

const repoUrl = 'https://github.com/Pinguin2001/squoosh';

const blobAnimImport = matchMedia('(prefers-reduced-motion: reduce)').matches
  ? undefined
  : import('./blob-anim');

const supportsClipboardAPI = !!navigator.clipboard?.read;

/** The SVG path for each of the static blobs behind the load button. */
const blobPaths = startBlobs.map((points) =>
  points
    .map((point, i) => {
      const nextI = i === points.length - 1 ? 0 : i + 1;
      let d = '';
      if (i === 0) {
        d += `M${point[2]} ${point[3]}`;
      }
      return (
        d +
        `C${point[4]} ${point[5]} ${points[nextI][0]} ${points[nextI][1]} ${points[nextI][2]} ${points[nextI][3]}`
      );
    })
    .join(''),
);

const fileInput = useTemplateRef<HTMLInputElement>('fileInput');
const base64Dialog =
  useTemplateRef<InstanceType<typeof Base64Dialog>>('base64Dialog');
const blobCanvas = useTemplateRef<HTMLCanvasElement>('blobCanvas');
const loadImg = useTemplateRef<HTMLElement>('loadImg');

const fetchingDemoIndex = ref<number>();
const beforeInstallEvent = ref<BeforeInstallPromptEvent>();
/** The static blobs show until the animated ones take over. */
const showBlobSVG = ref(true);
const importMenuOpen = ref(false);

const importMenuItems = [
  { value: 'file', label: 'Open file…' },
  { value: 'base64', label: 'Paste base64…' },
] as const;

function onImportMenuSelect(value: (typeof importMenuItems)[number]['value']) {
  if (value === 'file') fileInput.value!.click();
  else base64Dialog.value!.show();
}

function onBeforeInstallPrompt(event: BeforeInstallPromptEvent) {
  // Don't show the mini-infobar on mobile
  event.preventDefault();
  // Save the beforeinstallprompt event so it can be called later.
  beforeInstallEvent.value = event;
}

function onAppInstalled() {
  // We don't need the install button, if it's shown
  beforeInstallEvent.value = undefined;
}

onMounted(async () => {
  window.addEventListener('beforeinstallprompt', onBeforeInstallPrompt);
  window.addEventListener('appinstalled', onAppInstalled);

  if (!blobAnimImport) return;
  const { startBlobAnim } = await blobAnimImport;
  // Unmounted in the meantime?
  if (!blobCanvas.value) return;
  showBlobSVG.value = false;
  startBlobAnim(blobCanvas.value, loadImg.value!);
});

onBeforeUnmount(() => {
  window.removeEventListener('beforeinstallprompt', onBeforeInstallPrompt);
  window.removeEventListener('appinstalled', onAppInstalled);
});

function onFileChange(event: Event) {
  const input = event.target as HTMLInputElement;
  const file = input.files?.[0];
  if (!file) return;
  input.value = '';
  emit('file', file);
}

async function onDemoClick(index: number) {
  try {
    fetchingDemoIndex.value = index;
    const demo = demos[index];
    const blob = await fetch(demo.url).then((r) => r.blob());
    emit('file', new File([blob], demo.filename, { type: blob.type }));
  } catch {
    fetchingDemoIndex.value = undefined;
    showSnack("Couldn't fetch demo image");
  }
}

async function onInstallClick() {
  // Show the browser install prompt
  await beforeInstallEvent.value?.prompt();
}

async function onPasteClick() {
  let clipboardItems: ClipboardItem[];

  try {
    clipboardItems = await navigator.clipboard.read();
  } catch {
    showSnack('No permission to access clipboard');
    return;
  }

  for (const item of clipboardItems) {
    const type = item.types.find((type) => type.startsWith('image/'));
    if (!type) continue;
    emit('file', new File([await item.getType(type)], 'image.unknown'));
    return;
  }

  showSnack('No image found in the clipboard');
}
</script>

<template>
  <div class="intro abs-fill">
    <input ref="fileInput" class="hide" type="file" @change="onFileChange" />
    <div class="main">
      <canvas ref="blobCanvas" class="blob-canvas abs-fill" />
      <h1 class="logo-container">
        <img
          class="logo"
          :src="logoWithText"
          alt="SquooshRE"
          width="638"
          height="162"
        />
      </h1>
      <div ref="loadImg" class="load-img">
        <svg
          v-if="showBlobSVG"
          class="blob-svg abs-fill"
          viewBox="-1.25 -1.25 2.5 2.5"
          preserveAspectRatio="xMidYMid slice"
        >
          <path v-for="(d, i) in blobPaths" :key="i" :d="d" />
        </svg>
        <div class="load-img-content">
          <div :class="['load-split', { 'menu-open': importMenuOpen }]">
            <button class="unbutton load-btn" @click="fileInput!.click()">
              <svg viewBox="0 0 24 24" class="load-icon" aria-hidden="true">
                <path
                  d="M19 7v3h-2V7h-3V5h3V2h2v3h3v2h-3zm-3 4V8h-3V5H5a2 2 0 00-2 2v12c0 1.1.9 2 2 2h12a2 2 0 002-2v-8h-3zM5 19l3-4 2 3 3-4 4 5H5z"
                />
              </svg>
              Choose image
            </button>
            <button
              class="unbutton load-menu-btn"
              aria-label="More ways to open an image"
              aria-haspopup="menu"
              :aria-expanded="importMenuOpen"
              @click="importMenuOpen = !importMenuOpen"
            >
              <ExpandIcon />
            </button>
            <Menu
              v-model:open="importMenuOpen"
              :items="importMenuItems"
              align="end"
              @select="onImportMenuSelect"
            />
          </div>
          <div class="load-hint">
            <span class="drop-text">Drop </span>OR
            <button
              v-if="supportsClipboardAPI"
              class="unbutton paste-btn"
              @click="onPasteClick"
            >
              Paste
            </button>
            <template v-else>Paste</template>
          </div>
        </div>
      </div>
    </div>
    <div class="demos-container">
      <div class="content-padding">
        <p class="demo-title">Or <strong>try one</strong> of these:</p>
        <ul class="demos">
          <li v-for="(demo, i) in demos" :key="demo.filename">
            <button class="unbutton demo" @click="onDemoClick(i)">
              <div
                class="demo-icon-container"
                :style="{ '--demo-shape': `var(--shape-${demo.shape})` }"
              >
                <img class="demo-icon" :src="demo.iconUrl" alt="" />
                <div
                  v-if="fetchingDemoIndex === i"
                  class="demo-loader abs-fill"
                >
                  <LoadingSpinner />
                </div>
              </div>
              <span class="demo-description">{{ demo.description }}</span>
              <span class="demo-size">{{ demo.size }}</span>
            </button>
          </li>
        </ul>
      </div>
    </div>

    <section class="info">
      <div class="info-container">
        <SlideOnScroll>
          <div class="info-content">
            <div class="info-text-wrapper">
              <h2 class="info-title">Small</h2>
              <p class="info-caption">
                Smaller images mean faster load times. SquooshRE can reduce file
                size and maintain high quality.
              </p>
            </div>
            <div class="info-img-wrapper">
              <img
                class="info-img"
                :src="smallSectionAsset"
                alt="silhouette of a large 1.4 megabyte image shrunk into a smaller 80 kilobyte image"
                width="536"
                height="522"
              />
            </div>
          </div>
        </SlideOnScroll>
      </div>
    </section>

    <section class="info">
      <div class="info-container">
        <SlideOnScroll>
          <div class="info-content">
            <div class="info-text-wrapper">
              <h2 class="info-title">Simple</h2>
              <p class="info-caption">
                Open your image, inspect the differences, then save instantly.
                Feeling adventurous? Adjust the settings for even smaller files.
              </p>
            </div>
            <div class="info-img-wrapper">
              <img
                class="info-img"
                :src="simpleSectionAsset"
                alt="grid of multiple shrunk images displaying various options"
                width="538"
                height="384"
              />
            </div>
          </div>
        </SlideOnScroll>
      </div>
    </section>

    <section class="info">
      <div class="info-container">
        <SlideOnScroll>
          <div class="info-content">
            <div class="info-text-wrapper">
              <h2 class="info-title">Secure</h2>
              <p class="info-caption">
                Worried about privacy? Images never leave your device since
                SquooshRE does all the work locally.
              </p>
            </div>
            <div class="info-img-wrapper">
              <img
                class="info-img"
                :src="secureSectionAsset"
                alt="silhouette of a cloud with a 'no' symbol on top"
                width="498"
                height="333"
              />
            </div>
          </div>
        </SlideOnScroll>
      </div>
    </section>

    <footer class="footer">
      <div class="footer-container">
        <div class="footer-padding">
          <footer class="footer-items">
            <a
              class="footer-link"
              :href="`${repoUrl}/blob/dev/README.md#privacy`"
            >
              Privacy
            </a>
            <a class="footer-link footer-link-with-logo" :href="repoUrl">
              <img :src="githubLogo" alt="" width="18" height="18" />
              Source on Github
            </a>
          </footer>
        </div>
      </div>
    </footer>
    <Base64Dialog ref="base64Dialog" @file="emit('file', $event)" />
    <button
      v-if="beforeInstallEvent"
      class="unbutton install-btn"
      @click="onInstallClick"
    >
      Install
    </button>
  </div>
</template>

<style scoped>
.intro {
  -webkit-overflow-scrolling: touch;
  overflow: auto;
  overscroll-behavior: contain;
  display: grid;
  grid-template-rows: 1fr max-content max-content;
  font: var(--md-sys-typescale-body-large);
  color: var(--md-sys-color-on-surface);
  background: var(--md-sys-color-surface);
}

.blob-canvas {
  width: 100%;
  height: 100%;
}

.hide {
  display: none;
}

.main {
  min-height: 541px;
  display: grid;
  grid-template-rows: max-content max-content;
  justify-items: center;
  position: relative;
  /* Extra room at the bottom for the demos sheet's rounded corners */
  padding-bottom: 48px;
  --blob-pink: var(--brand-pink);
  --center-blob-opacity: 0.3;

  @media (min-width: 600px) {
    min-height: 688px;
  }

  @media (prefers-color-scheme: dark) {
    --center-blob-opacity: 0.4;
  }
}

.logo-container {
  margin: 5rem 0 1rem;
}

.logo {
  transform: translate(-1%, 0);
  width: 256px;
  height: auto;
}

.load-img {
  position: relative;
}

.blob-svg {
  width: 100%;
  height: 100%;
  fill: var(--blob-pink);

  & path {
    opacity: var(--center-blob-opacity);
  }
}

.load-img-content {
  position: relative;
  --size: 22rem;
  max-width: var(--size);
  width: 100vw;
  height: var(--size);
  display: grid;
  grid-template-rows: max-content max-content;
  justify-items: center;
  align-content: center;
  gap: 16px;

  @media (min-width: 600px) {
    --size: 27rem;
  }
}

/* Large split button: "Choose image" plus a menu of other ways to open one */
.load-split {
  position: relative;
  display: flex;
  gap: 2px;
  border-radius: var(--md-sys-shape-corner-large-increased);
  box-shadow: var(--md-sys-elevation-3);
}

.load-btn,
.load-menu-btn {
  height: 80px;
  background: var(--md-sys-color-primary);
  color: var(--md-sys-color-on-primary);
  transition-property: border-radius, scale, box-shadow;
  transition-duration: var(--md-sys-motion-spring-default-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  &:focus-visible {
    outline-offset: 4px;
  }
}

.load-btn {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 0 28px 0 26px;
  border-radius: var(--md-sys-shape-corner-large-increased)
    var(--md-sys-shape-corner-small) var(--md-sys-shape-corner-small)
    var(--md-sys-shape-corner-large-increased);
  font: var(--md-sys-typescale-title-large);
  font-weight: var(--md-sys-typescale-emphasized-weight);

  &:hover {
    border-radius: 40px var(--md-sys-shape-corner-medium)
      var(--md-sys-shape-corner-medium) 40px;
  }

  &:active {
    border-radius: var(--md-sys-shape-corner-medium);
    scale: 0.97;
  }
}

.load-menu-btn {
  display: grid;
  place-items: center;
  width: 64px;
  border-radius: var(--md-sys-shape-corner-small)
    var(--md-sys-shape-corner-large-increased)
    var(--md-sys-shape-corner-large-increased) var(--md-sys-shape-corner-small);

  svg {
    width: 32px;
    height: 32px;
    transition: rotate var(--md-sys-motion-spring-fast-spatial-duration)
      var(--md-sys-motion-spring-fast-spatial);
  }

  &:hover {
    border-radius: var(--md-sys-shape-corner-medium) 40px 40px
      var(--md-sys-shape-corner-medium);
  }

  &:active {
    scale: 0.94;
  }

  /* While its menu is open, the trailing button turns fully round. */
  .menu-open & {
    border-radius: 40px;

    svg {
      rotate: 180deg;
    }
  }
}

.load-icon {
  --size: 32px;
  width: var(--size);
  height: var(--size);
  fill: currentColor;
}

.load-hint {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 4px 6px 4px 16px;
  border-radius: var(--md-sys-shape-corner-full);
  background: color-mix(
    in srgb,
    var(--md-sys-color-surface-container-lowest) 80%,
    transparent
  );
  color: var(--md-sys-color-on-surface-variant);
  font: var(--md-sys-typescale-label-large);
  text-transform: uppercase;
  letter-spacing: 0.04em;
}

/* Text button */
.paste-btn {
  min-height: 40px;
  padding: 0 12px;
  border-radius: var(--md-sys-shape-corner-full);
  color: var(--md-sys-color-primary);
  font: inherit;
  letter-spacing: inherit;
  text-transform: inherit;
  transition: background-color var(--md-sys-motion-spring-fast-effects-duration);

  &:hover {
    background: color-mix(in srgb, currentColor 8%, transparent);
  }
}

.drop-text {
  @media (max-width: 599px) {
    display: none;
  }
}

/* The demos sheet: blue, like the right-hand side of the editor. */
.demos-container {
  position: relative;
  margin-top: -48px;
  padding-bottom: 48px;
  border-radius: var(--md-sys-shape-corner-extra-extra-large)
    var(--md-sys-shape-corner-extra-extra-large) 0 0;
  background: var(--md-sys-color-tertiary-container);
  color: var(--md-sys-color-on-tertiary-container);
}

.content-padding {
  padding: 40px 16px 32px;
}

.demo-title {
  margin: 0;
  font: var(--md-sys-typescale-headline-medium);
  text-align: center;

  strong {
    font-weight: var(--md-sys-typescale-emphasized-weight);
  }
}

.demos {
  display: grid;
  gap: 12px;
  justify-content: center;
  padding: 0;
  margin: 32px auto 0;
  max-width: 880px;
  --demo-size: 88px;
  grid-template-columns: repeat(2, minmax(0, 160px));

  @media (min-width: 740px) {
    --demo-size: 112px;
    gap: 16px;
    grid-template-columns: repeat(4, minmax(0, 200px));
  }

  & > li {
    display: block;
  }
}

/* Demo cards */
.demo {
  box-sizing: border-box;
  width: 100%;
  height: 100%;
  display: grid;
  justify-items: center;
  align-content: start;
  gap: 4px;
  padding: 20px 12px 16px;
  border-radius: var(--md-sys-shape-corner-extra-large);
  background: var(--md-sys-color-surface-container-lowest);
  color: var(--md-sys-color-on-surface);
  transition-property: border-radius, scale, box-shadow;
  transition-duration: var(--md-sys-motion-spring-default-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  &:hover,
  &:focus-visible {
    border-radius: var(--md-sys-shape-corner-extra-extra-large);
    box-shadow: var(--md-sys-elevation-2);
  }

  &:active {
    scale: 0.96;
    border-radius: var(--md-sys-shape-corner-large);
  }
}

.demo-icon-container {
  position: relative;
  width: var(--demo-size);
  height: var(--demo-size);
  margin-bottom: 8px;
  clip-path: var(--demo-shape);
  transition-property: clip-path, rotate;
  transition-duration: var(--md-sys-motion-spring-default-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-default-spatial);

  .demo:hover &,
  .demo:focus-visible & {
    clip-path: var(--shape-sunny);
    rotate: 15deg;
  }
}

.demo-icon {
  width: 100%;
  height: 100%;
  display: block;
  object-fit: cover;
  transition: rotate var(--md-sys-motion-spring-default-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);

  /* Keep the picture upright while its frame turns. */
  .demo:hover &,
  .demo:focus-visible & {
    rotate: -15deg;
  }
}

.demo-description {
  font: var(--md-sys-typescale-title-medium);
}

.demo-size {
  padding: 2px 10px;
  border-radius: var(--md-sys-shape-corner-full);
  background: var(--md-sys-color-tertiary-container);
  color: var(--md-sys-color-on-tertiary-container);
  font: var(--md-sys-typescale-label-medium);
}

.demo-loader {
  background: rgb(0 0 0 / 0.5);
  display: grid;
  justify-content: center;
  align-content: center;
  animation: fade-in 600ms ease-in-out;

  & > .loading-spinner {
    --color: #fff;
  }
}

/* Info: a surface sheet over the demos */
.info {
  background: var(--md-sys-color-surface);
  position: relative;
  padding: 3rem 1.5rem;

  @media (min-width: 1200px) {
    padding: 4rem 0;
  }
}

.info:first-of-type {
  margin-top: -48px;
  padding-top: 4rem;
  border-radius: var(--md-sys-shape-corner-extra-extra-large)
    var(--md-sys-shape-corner-extra-extra-large) 0 0;
}

.info-container {
  max-width: 1000px;
  margin: 0 auto;
}

.info-content {
  display: grid;
  align-items: center;
  grid-template-columns: 1fr;
  gap: 1.5rem;

  grid-template-areas:
    'text'
    'img';

  @media (min-width: 712px) {
    grid-template-columns: 1fr 1fr;
    grid-template-areas: 'text img';
    gap: 3rem;

    .info:nth-of-type(even) & {
      grid-template-areas: 'img text';
    }
  }
}

.info-title {
  margin: 0;
  color: var(--md-sys-color-primary);
  font: var(--md-sys-typescale-display-large);
  font-weight: 800;
  letter-spacing: -0.02em;

  @media (min-width: 712px) {
    font-size: 4.5rem;
    line-height: 1.1;
  }
}

.info-caption {
  margin: 1rem 0 0.5rem 0;
  font: var(--md-sys-typescale-title-large);
  line-height: 1.6;
  color: var(--md-sys-color-on-surface-variant);
}

.info-text-wrapper {
  display: flex;
  flex-flow: column wrap;
  max-width: 27em;
  justify-self: center;
  grid-area: text;

  @media (min-width: 712px) {
    justify-self: start;
    .info:nth-of-type(even) & {
      text-align: right;
      justify-self: end;
    }
  }
}

.info-img-wrapper {
  grid-area: img;
}

.info-img {
  display: block;
  width: 100%;
  height: auto;
  max-width: 400px;
  margin: 0 auto;

  @media (min-width: 768px) {
    margin: 0 0 0 auto;

    .info:nth-of-type(even) & {
      margin: 0;
    }
  }
}

.footer {
  background: var(--md-sys-color-surface);
  padding-top: 3rem;
}

.footer-container {
  background: var(--md-sys-color-surface-container);
  border-radius: var(--md-sys-shape-corner-extra-extra-large)
    var(--md-sys-shape-corner-extra-extra-large) 0 0;
}

.footer-padding {
  padding: 2rem 1.5rem;
}

.footer-items {
  display: grid;
  grid-auto-rows: max-content;
  justify-items: center;
  gap: 8px;

  @media (min-width: 480px) {
    justify-content: end;
    grid-auto-flow: column;
    align-items: center;
  }
}

/* Tonal buttons */
.footer-link {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  min-height: 40px;
  padding: 0 20px;
  border-radius: 20px;
  background: var(--md-sys-color-secondary-container);
  color: var(--md-sys-color-on-secondary-container);
  font: var(--md-sys-typescale-label-large);
  text-decoration: none;
  transition: border-radius var(--md-sys-motion-spring-fast-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);

  &:hover {
    border-radius: var(--md-sys-shape-corner-medium);
  }
}

.footer-link-with-logo img {
  width: 18px;
  height: 18px;
}

@keyframes fade-in {
  from {
    opacity: 0;
  }
}

.install-btn {
  position: absolute;
  top: 16px;
  right: 16px;
  min-height: 48px;
  padding: 0 24px;
  border-radius: 24px;
  background: var(--md-sys-color-secondary-container);
  color: var(--md-sys-color-on-secondary-container);
  font: var(--md-sys-typescale-label-large);
  font-size: 1rem;
  box-shadow: var(--md-sys-elevation-1);
  animation: fade-in 600ms ease-in-out;
  transition: border-radius var(--md-sys-motion-spring-fast-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);

  &:hover {
    border-radius: var(--md-sys-shape-corner-medium);
  }
}
</style>
