<script setup lang="ts">
import { computed, ref, useTemplateRef } from 'vue';

const props = withDefaults(
  defineProps<{
    min?: number;
    max?: number;
    step?: number;
    disabled?: boolean;
    /** Decimal places in the value bubble. Defaults to the step's. */
    labelPrecision?: number;
  }>(),
  { min: 0, max: 100, step: 1 },
);
const value = defineModel<number>({ required: true });

const rangeEl = useTemplateRef<HTMLInputElement>('range');
const dragging = ref(false);
const textFocused = ref(false);
/** What's in the number field while it has focus. */
const typedText = ref('');

function getPrecision(value: number): number {
  const afterDecimal = String(value).split('.')[1];
  return afterDecimal ? afterDecimal.length : 0;
}

const displayValue = computed(() => {
  const current = value.value || 0;
  if (current >= 10000) return (current / 1000).toFixed(1) + 'k';

  const precision = props.labelPrecision || getPrecision(props.step);
  return precision
    ? current.toFixed(precision)
    : Math.round(current).toString();
});

const percent = computed(
  () => (100 * ((value.value || 0) - props.min)) / (props.max - props.min),
);

function onRangeInput(event: Event) {
  value.value = Number((event.target as HTMLInputElement).value);
}

function onTextInput(event: Event) {
  const text = (event.target as HTMLInputElement).value;
  typedText.value = text;
  if (!text.trim()) return;
  // Let the range input clamp the value and snap it to the step.
  const range = rangeEl.value!;
  range.value = text;
  value.value = Number(range.value);
}

function onTextFocus(event: FocusEvent) {
  typedText.value = (event.target as HTMLInputElement).value;
  textFocused.value = true;
}

// Not using pointer capture here due to https://bugs.webkit.org/show_bug.cgi?id=219636.
let activePointer: number | undefined;

function onPointerDown(event: PointerEvent) {
  if (activePointer !== undefined) return;
  activePointer = event.pointerId;
  dragging.value = true;

  const pointerUp = (event: PointerEvent) => {
    if (event.pointerId !== activePointer) return;
    activePointer = undefined;
    dragging.value = false;
    window.removeEventListener('pointerup', pointerUp);
    window.removeEventListener('pointercancel', pointerUp);
  };
  window.addEventListener('pointerup', pointerUp);
  window.addEventListener('pointercancel', pointerUp);
}
</script>

<template>
  <label class="range">
    <span class="label-text"><slot /></span>
    <!-- On interaction, Safari gives focus to the first element in the label,
         so the range input is deliberately first. -->
    <div class="range-wc-container">
      <div
        :class="['range-input', { disabled }]"
        :style="{
          '--value-percent': percent + '%',
          '--value-fraction': percent / 100,
          '--value-width': displayValue.length,
        }"
        @pointerdown="onPointerDown"
      >
        <input
          ref="range"
          type="range"
          :class="['input', { 'touch-active': dragging }]"
          :min="min"
          :max="max"
          :step="step"
          :value="value"
          :disabled="disabled"
          @input="onRangeInput"
        />
        <div class="thumb-wrapper">
          <div class="thumb"></div>
          <div class="value-display">{{ displayValue }}</div>
        </div>
      </div>
    </div>
    <input
      type="number"
      class="text-input"
      :value="textFocused ? typedText : value"
      :min="min"
      :max="max"
      :step="step"
      :disabled="disabled"
      @input="onTextInput"
      @focus="onTextFocus"
      @blur="textFocused = false"
    />
  </label>
</template>

<style scoped>
/* Material 3 Expressive slider: thick track with a gap around a bar handle,
   a stop indicator at the end, and a value indicator while in use. */
.range {
  --accent: var(--side-accent, var(--md-sys-color-primary));
  --inactive: var(--side-container, var(--md-sys-color-secondary-container));
  --handle-gap: 6px;
  position: relative;
  z-index: 0;
  display: grid;
  grid-template-columns: 1fr auto;
  align-items: center;
  row-gap: 2px;
}

.label-text {
  color: var(--md-sys-color-on-surface);
}

.range-wc-container {
  position: relative;
  z-index: 1;
  grid-row: 2 / 3;
  grid-column: 1 / 3;
}

.text-input {
  grid-row: 1 / 2;
  grid-column: 2 / 3;

  box-sizing: border-box;
  width: 64px;
  height: 32px;
  padding: 0 10px;
  border: none;
  border-radius: var(--md-sys-shape-corner-small);
  background: var(--md-sys-color-surface-container-highest);
  color: var(--md-sys-color-on-surface);
  font: var(--md-sys-typescale-label-large);
  font-variant-numeric: tabular-nums;
  text-align: right;
  transition: border-radius var(--md-sys-motion-spring-fast-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);

  &:focus {
    outline: 2px solid var(--accent);
    outline-offset: 0;
    border-radius: var(--md-sys-shape-corner-medium);
  }

  &:disabled {
    opacity: 0.38;
  }

  /* Remove the number controls */
  -moz-appearance: textfield;

  &::-webkit-outer-spin-button,
  &::-webkit-inner-spin-button {
    -moz-appearance: none;
    -webkit-appearance: none;
    margin: 0;
  }
}

.range-input {
  /* Handle centre, in px from the left. The native thumb below is 16px wide,
     so this matches where the browser maps the pointer. */
  --handle-x: calc(8px + (100% - 16px) * var(--value-fraction, 0));
  position: relative;
  display: flex;
  height: 44px;
  width: 100%;
  overflow: visible;
}

.range-input.disabled {
  opacity: 0.38;
  filter: grayscale(1);
}

/* Active track */
.range-input::before,
/* Inactive track, with the stop indicator */
.range-input::after {
  content: '';
  position: absolute;
  top: 14px;
  height: 16px;
  pointer-events: none;
}

.range-input::before {
  left: 0;
  width: max(0px, calc(var(--handle-x) - var(--handle-gap)));
  border-radius: 8px 2px 2px 8px;
  background: var(--accent);
}

.range-input::after {
  left: calc(var(--handle-x) + var(--handle-gap));
  right: 0;
  border-radius: 2px 8px 8px 2px;
  background:
    radial-gradient(
      circle at calc(100% - 8px) 50%,
      var(--accent) 2px,
      transparent 2.5px
    ),
    var(--inactive);
}

.input {
  position: relative;
  width: 100%;
  height: 100%;
  padding: 0;
  margin: 0;
  opacity: 0;
  -webkit-appearance: none;
  appearance: none;
  background: none;
  cursor: pointer;

  &::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 16px;
    height: 44px;
  }

  &::-moz-range-thumb {
    width: 16px;
    height: 44px;
    border: none;
  }
}

.thumb-wrapper {
  position: absolute;
  left: 8px;
  right: 8px;
  top: 0;
  height: 100%;
  overflow: visible;
  pointer-events: none;
  transform: translate(var(--value-percent, 0%), 0);
}

.thumb {
  position: absolute;
  top: 0;
  left: 0;
  width: 4px;
  height: 44px;
  margin-left: -2px;
  border-radius: 2px;
  background: var(--accent);
  transition-property: width, margin-left;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
}

.range-input:has(:focus-visible) .thumb {
  outline: 3px solid var(--md-sys-color-secondary);
  outline-offset: 2px;
}

.value-display {
  position: absolute;
  left: 0;
  bottom: calc(100% + 4px);
  box-sizing: border-box;
  min-width: 48px;
  height: 44px;
  padding: 0 16px;
  display: grid;
  place-items: center;
  transform: translateX(-50%) scale(0.4);
  transform-origin: 50% 100%;
  opacity: 0;
  border-radius: var(--md-sys-shape-corner-full);
  background: var(--md-sys-color-inverse-surface);
  color: var(--md-sys-color-inverse-on-surface);
  font: var(--md-sys-typescale-label-large);
  font-variant-numeric: tabular-nums;
  white-space: nowrap;
  transition-property: opacity, transform;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
  will-change: transform;
  pointer-events: none;
}

.touch-active + .thumb-wrapper,
.input:focus-visible + .thumb-wrapper {
  .value-display {
    opacity: 1;
    transform: translateX(-50%) scale(1);
  }
}

/* The handle narrows while it's held. */
.touch-active + .thumb-wrapper .thumb {
  width: 2px;
  margin-left: -1px;
}
</style>
