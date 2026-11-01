<!-- File size, % change and download button for one side. -->
<script setup lang="ts">
import { computed, ref, watch } from 'vue';
import LoadingSpinner from '@/components/LoadingSpinner.vue';
import { DownloadIcon, ExpandIcon } from '@/components/icons';
import type { SourceImage } from '@/composables/useCompressor';
import prettyBytes from '@/lib/pretty-bytes';

const props = defineProps<{
  loading: boolean;
  source?: SourceImage;
  imageFile?: File;
  downloadUrl?: string;
  flipSide: boolean;
  typeLabel: string;
}>();

/** Quick jobs don't flash the spinner. */
const loadingReactionDelay = 500;

const showLoadingState = ref(props.loading);
let loadingTimeoutId: number | undefined;

watch(
  () => props.loading,
  (loading) => {
    clearTimeout(loadingTimeoutId);
    if (loading) {
      loadingTimeoutId = self.setTimeout(
        () => (showLoadingState.value = true),
        loadingReactionDelay,
      );
    } else {
      showLoadingState.value = false;
    }
  },
);

const prettySize = computed(
  () => props.imageFile && prettyBytes(props.imageFile.size),
);

const isOriginal = computed(
  () =>
    !props.source || !props.imageFile || props.source.file === props.imageFile,
);

const diff = computed(
  () =>
    props.source &&
    props.imageFile &&
    props.imageFile.size / props.source.file.size,
);

const percent = computed(() => {
  if (diff.value === undefined) return undefined;
  const absolutePercent = Math.round(Math.abs(diff.value) * 100);
  return diff.value > 1 ? absolutePercent - 100 : 100 - absolutePercent;
});
</script>

<template>
  <div
    :class="[
      'results',
      flipSide ? 'results-right' : 'results-left',
      { 'is-original': isOriginal },
    ]"
  >
    <div class="expand-arrow">
      <ExpandIcon />
    </div>
    <div class="bubble">
      <div class="size-info">
        <template v-if="prettySize">
          <span class="size-number">{{ prettySize.value }}</span>
          <span class="unit">{{ ' ' + prettySize.unit }}</span>
          <span class="type-label">{{ ' ' + typeLabel }}</span>
        </template>
        <template v-else>…</template>
      </div>
      <div class="percent-info">
        <span v-if="diff && diff !== 1" class="size-direction">
          {{ diff < 1 ? '↓' : '↑' }}
        </span>
        <span class="size-value">{{ percent || 0 }}</span>
        <span class="percent-char">%</span>
      </div>
    </div>
    <a
      :class="['download', { 'download-disable': showLoadingState }]"
      :href="downloadUrl"
      :download="imageFile ? imageFile.name : ''"
      title="Download"
    >
      <span class="download-shape"></span>
      <span class="download-icon">
        <DownloadIcon />
      </span>
      <LoadingSpinner v-if="showLoadingState" />
    </a>
  </div>
</template>

<style scoped>
.results {
  --fab-size: 56px;
  display: grid;
  grid-template-columns: [arrow] auto [bubble] 1fr [download] auto;
  align-items: center;
  gap: 8px;
  padding: 6px 6px 6px 12px;
  border-radius: var(--md-sys-shape-corner-full);
  background: var(--md-sys-color-surface-container-high);
  color: var(--md-sys-color-on-surface);
  box-shadow: var(--md-sys-elevation-2);

  @media (min-width: 600px) {
    --fab-size: 72px;
    grid-template-columns: [download] auto [bubble] 1fr;
    gap: 14px;
    padding: 10px 20px 10px 10px;
    background: var(--md-sys-color-surface-container-low);
  }
}

.expand-arrow {
  grid-column: arrow;
  display: grid;
  color: var(--md-sys-color-on-surface-variant);
  rotate: 180deg;
  transition: rotate var(--md-sys-motion-spring-fast-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);

  @media (min-width: 600px) {
    display: none;
  }

  :focus-visible & {
    color: var(--side-accent);
  }

  [content-expanded] & {
    rotate: 0deg;
  }

  svg {
    display: block;
  }
}

.bubble {
  grid-row: 1;
  grid-column: bubble;
  min-width: 0;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 8px;
}

.size-info {
  min-width: 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  font: var(--md-sys-typescale-title-medium);

  @media (min-width: 600px) {
    font: var(--md-sys-typescale-headline-small);
  }
}

.size-number {
  font-weight: var(--md-sys-typescale-emphasized-weight);
  font-variant-numeric: tabular-nums;
}

.unit {
  color: var(--side-accent);
  font-weight: 600;
}

.type-label {
  color: var(--md-sys-color-on-surface-variant);
  font: var(--md-sys-typescale-body-medium);

  @media (min-width: 600px) {
    display: none;
  }
}

/* The percentage badge */
.percent-info {
  flex: none;
  display: flex;
  align-items: baseline;
  gap: 1px;
  padding: 4px 12px;
  border-radius: var(--md-sys-shape-corner-full);
  background: var(--side-container);
  color: var(--side-on-container);
  font: var(--md-sys-typescale-title-medium);
  font-weight: var(--md-sys-typescale-emphasized-weight);
  font-variant-numeric: tabular-nums;
  transition-property: border-radius, background-color, color;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  @media (min-width: 600px) {
    padding: 6px 14px;
    font: var(--md-sys-typescale-title-large);
    font-weight: var(--md-sys-typescale-emphasized-weight);
  }
}

.size-direction {
  margin-right: 2px;
}

.percent-char {
  font-size: 0.75em;
  opacity: 0.8;
}

/* Download FAB in the "cookie" shape */
.download {
  position: relative;
  grid-row: 1;
  grid-column: download;
  display: grid;
  place-items: center;
  width: var(--fab-size);
  height: var(--fab-size);
  border-radius: 50%;
  color: var(--side-on-accent);

  .loading-spinner {
    grid-area: 1 / 1;
    position: relative;
    --color: var(--side-accent);
    --size: calc(var(--fab-size) * 0.6);
  }
}

.download-shape {
  grid-area: 1 / 1;
  width: 100%;
  height: 100%;
  background: var(--side-accent);
  clip-path: var(--shape-cookie-9);
  transition-property: clip-path, rotate, background-color;
  transition-duration: var(--md-sys-motion-spring-default-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-default-spatial);
}

.download:hover .download-shape {
  rotate: 40deg;
}

.download:active .download-shape {
  clip-path: var(--shape-sunny);
  rotate: 60deg;
}

.download-icon {
  grid-area: 1 / 1;
  position: relative;
  display: grid;
  transition-property: opacity, rotate, scale;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  svg {
    --size: calc(var(--fab-size) * 0.36);
    width: var(--size);
    height: var(--size);
    fill: currentColor;
  }
}

.download-disable {
  pointer-events: none;

  .download-shape {
    clip-path: var(--shape-circle);
    background: var(--side-container);
  }

  .download-icon {
    opacity: 0;
    rotate: 90deg;
    scale: 0.5;
  }
}

.results-right {
  @media (min-width: 600px) {
    grid-template-columns: [bubble] 1fr [download] auto;
    padding: 10px 10px 10px 20px;

    .bubble {
      flex-direction: row-reverse;
    }
  }
}

.is-original {
  .percent-info {
    background: none;
    color: var(--md-sys-color-on-surface-variant);
  }
  .download {
    color: var(--md-sys-color-on-surface);
  }
  .download-shape {
    background: var(--md-sys-color-surface-container-highest);
  }
  .unit {
    color: var(--md-sys-color-on-surface-variant);
  }
}
</style>
