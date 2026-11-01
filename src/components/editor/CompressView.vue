<!-- The editor: the image comparison, and the options and results for each
     side. -->
<script setup lang="ts">
import { computed, onMounted, toRef } from 'vue';
import { useCompressor, type Side } from '@/composables/useCompressor';
import { useMediaQuery } from '@/composables/useMediaQuery';
import { showSnack } from '@/composables/useSnackbar';
import { encoderMap } from '@/features/registry';
import { cacheAllCodecs } from '@/lib/sw-bridge';
import ImageOutput from './ImageOutput.vue';
import MultiPanel from './MultiPanel.vue';
import OptionsPanel from './OptionsPanel.vue';
import ResultsBubble from './ResultsBubble.vue';

const props = defineProps<{ file: File }>();
const emit = defineEmits<{ back: [] }>();

const compressor = useCompressor(toRef(props, 'file'), showSnack);
const { source, loading, preprocessorState, sides, hasSavedSettings } =
  compressor;

// Using the editor is a good sign the codecs are worth keeping offline.
onMounted(cacheAllCodecs);

const mobileView = useMediaQuery('(max-width: 599px)');

function typeLabel(side: Side) {
  const encoderState = side.latestSettings.encoderState;
  if (encoderState) return encoderMap[encoderState.type].meta.label;
  return side.file ? side.file.name : 'Original Image';
}

/**
 * For rendering, we ideally want the settings that were used to create the
 * data, not the latest settings.
 */
function imgContain(side: Side) {
  const { resize } = (side.encodedSettings ?? side.latestSettings)
    .processorState;
  return resize.enabled && resize.fitMethod === 'contain';
}

const leftImgContain = computed(() => imgContain(sides[0]));
const rightImgContain = computed(() => imgContain(sides[1]));

function onRotate() {
  compressor.setRotation(
    ((preprocessorState.rotate.rotate + 90) % 360) as 0 | 90 | 180 | 270,
  );
}
</script>

<template>
  <div class="compress">
    <ImageOutput
      :source="source"
      :vertical="mobileView"
      :left-compressed="sides[0].data"
      :right-compressed="sides[1].data"
      :left-img-contain="leftImgContain"
      :right-img-contain="rightImgContain"
      @rotate="onRotate"
    />
    <button class="unbutton back" title="Back" @click="emit('back')">
      <svg viewBox="0 0 24 24" aria-hidden="true">
        <path
          d="M19 6.41 17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"
        />
      </svg>
    </button>

    <div v-if="mobileView" class="options">
      <MultiPanel
        class="multi-panel"
        :count="2"
        :panel-classes="['options-1-theme', 'options-2-theme']"
      >
        <template v-for="index in [0, 1] as const" #[`heading-${index}`]>
          <ResultsBubble
            :download-url="sides[index].downloadUrl"
            :image-file="sides[index].file"
            :source="source"
            :loading="loading || sides[index].loading"
            :flip-side="true"
            :type-label="typeLabel(sides[index])"
          />
        </template>
        <template v-for="index in [0, 1] as const" #[`content-${index}`]>
          <OptionsPanel
            :side="sides[index]"
            :source="source"
            :can-import="hasSavedSettings[index]"
            @encoder-type-change="compressor.setEncoderType(index, $event)"
            @copy-to-other-side="compressor.copyToOtherSide(index)"
            @save-settings="compressor.saveSideSettings(index)"
            @import-settings="compressor.importSideSettings(index)"
          />
        </template>
      </MultiPanel>
    </div>
    <template v-else>
      <div
        v-for="index in [0, 1] as const"
        :key="index"
        :class="[
          'options',
          index === 0
            ? 'options-1 options-1-theme'
            : 'options-2 options-2-theme',
        ]"
      >
        <OptionsPanel
          :side="sides[index]"
          :source="source"
          :can-import="hasSavedSettings[index]"
          @encoder-type-change="compressor.setEncoderType(index, $event)"
          @copy-to-other-side="compressor.copyToOtherSide(index)"
          @save-settings="compressor.saveSideSettings(index)"
          @import-settings="compressor.importSideSettings(index)"
        />
        <ResultsBubble
          :download-url="sides[index].downloadUrl"
          :image-file="sides[index].file"
          :source="source"
          :loading="loading || sides[index].loading"
          :flip-side="index === 1"
          :type-label="typeLabel(sides[index])"
        />
      </div>
    </template>
  </div>
</template>

<style scoped>
.compress {
  width: 100%;
  height: 100%;
  contain: strict;
  display: grid;
  grid-template-rows: max-content 1fr;
  grid-template-areas:
    'header'
    'opts';

  @media (min-width: 600px) {
    grid-template-rows: max-content 1fr;
    grid-template-columns: max-content 1fr max-content;
    grid-template-areas:
      'header header header'
      'optsLeft viewportOpts optsRight';
  }
}

.options {
  position: relative;
  color: var(--md-sys-color-on-surface);
  max-width: 400px;
  margin: 0 auto 12px;
  width: calc(100% - 24px);
  max-height: 100%;
  overflow: hidden;
  grid-area: opts;
  display: grid;
  grid-template-rows: 1fr max-content;
  align-content: end;
  align-self: end;
  gap: 8px;

  @media (min-width: 600px) {
    width: 320px;
    margin: 0 16px 16px;
  }
}

.options-1 {
  grid-area: optsLeft;
}

.options-2 {
  grid-area: optsRight;
}

/* The options card floats over the image. */
.options > :deep(.options-scroller),
.multi-panel :deep(.options-scroller) {
  box-shadow: var(--md-sys-elevation-2);
}

.multi-panel {
  position: relative;
  display: flex;
  flex-flow: column;
  overflow: hidden;
  /* Room for the cards' shadows */
  padding: 4px;
  margin: -4px;

  /* Reorder so headings appear after content: */
  & > :deep(:nth-child(1)) {
    order: 2;
    margin-bottom: 8px;
  }

  & > :deep(:nth-child(2)) {
    order: 1;
  }

  & > :deep(:nth-child(3)) {
    order: 4;
  }

  & > :deep(:nth-child(4)) {
    order: 3;
  }

  & > :deep(.panel-content[aria-expanded='true']) {
    margin-bottom: 8px;
    border-radius: var(--md-sys-shape-corner-extra-large);
  }

  & > :deep(.panel-heading) {
    border-radius: var(--md-sys-shape-corner-full);
    cursor: pointer;
  }
}

/* Filled tonal icon button */
.back {
  position: relative;
  z-index: 1;
  grid-area: header;
  margin: 12px;
  justify-self: start;
  align-self: start;
  display: grid;
  place-items: center;
  width: 48px;
  height: 48px;
  border-radius: 50%;
  background: var(--md-sys-color-primary-container);
  color: var(--md-sys-color-on-primary-container);
  box-shadow: var(--md-sys-elevation-2);
  transition-property: border-radius, scale;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  & > svg {
    width: 24px;
    height: 24px;
    fill: currentColor;
  }

  &:hover {
    border-radius: var(--md-sys-shape-corner-large);
  }

  &:active {
    border-radius: var(--md-sys-shape-corner-medium);
    scale: 0.95;
  }

  @media (min-width: 600px) {
    margin: 16px;
    width: 56px;
    height: 56px;
  }
}
</style>
