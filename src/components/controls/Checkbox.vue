<!-- Material 3 checkbox. -->
<script setup lang="ts">
defineOptions({ inheritAttrs: false });
defineProps<{ disabled?: boolean }>();
const checked = defineModel<boolean>({ default: false });
</script>

<template>
  <div :class="['checkbox', { disabled }]">
    <input
      v-model="checked"
      class="real-checkbox"
      type="checkbox"
      :disabled="disabled"
      v-bind="$attrs"
    />
    <div class="box">
      <svg class="check" viewBox="0 0 18 18" aria-hidden="true">
        <path d="M3.5 9.5 7 13l7.5-7.5" />
      </svg>
    </div>
  </div>
</template>

<style scoped>
.checkbox {
  --accent: var(--side-accent, var(--md-sys-color-primary));
  --on-accent: var(--side-on-accent, var(--md-sys-color-on-primary));
  display: inline-grid;
  place-items: center;
  position: relative;
  /* 40px state layer, inside a 48px row */
  width: 40px;
  height: 40px;
  margin: -4px -11px -4px 0;

  &::before {
    content: '';
    position: absolute;
    inset: 0;
    border-radius: 50%;
    background: var(--md-sys-color-on-surface);
    opacity: 0;
    scale: 0.6;
    transition-property: opacity, scale;
    transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
    transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
  }

  &:hover::before {
    opacity: var(--md-sys-state-hover-opacity);
    scale: 1;
  }

  &:has(:focus-visible)::before {
    opacity: var(--md-sys-state-focus-opacity);
    scale: 1;
    background: var(--accent);
  }

  &:has(:focus-visible) .box {
    outline: 3px solid var(--md-sys-color-secondary);
    outline-offset: 4px;
  }
}

.real-checkbox {
  top: 0;
  position: absolute;
  opacity: 0;
  pointer-events: none;
}

.box {
  position: relative;
  box-sizing: border-box;
  width: 18px;
  height: 18px;
  border: 2px solid var(--md-sys-color-on-surface-variant);
  border-radius: 2px;
  display: grid;
  place-items: center;
  transition-property: background-color, border-color, border-radius;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
}

.check {
  width: 18px;
  height: 18px;
  fill: none;
  stroke: var(--on-accent);
  stroke-width: 2;
  stroke-dasharray: 17;
  stroke-dashoffset: 17;
  transition: stroke-dashoffset
    var(--md-sys-motion-spring-default-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial);
}

.real-checkbox:checked + .box {
  background: var(--accent);
  border-color: var(--accent);
  border-radius: 4px;

  .check {
    stroke-dashoffset: 0;
  }
}

.disabled {
  opacity: 0.38;
  pointer-events: none;

  .real-checkbox:checked + .box {
    background: var(--md-sys-color-on-surface);
    border-color: var(--md-sys-color-on-surface);
  }
}
</style>
