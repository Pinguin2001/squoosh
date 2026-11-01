<script setup lang="ts" generic="T">
import { ExpandIcon } from '@/components/icons';

defineOptions({ inheritAttrs: false });
defineProps<{ large?: boolean }>();
const value = defineModel<T>();
</script>

<template>
  <div :class="['select', { large }]">
    <select v-model="value" class="builtin-select" v-bind="$attrs">
      <slot />
    </select>
    <div class="arrow">
      <ExpandIcon />
    </div>
  </div>
</template>

<style scoped>
.select {
  position: relative;
  min-width: 0;
}

.builtin-select {
  box-sizing: border-box;
  width: 100%;
  min-height: 40px;
  padding: 0 40px 0 14px;
  border: none;
  border-radius: var(--md-sys-shape-corner-medium);
  background: var(--md-sys-color-surface-container-highest);
  color: var(--md-sys-color-on-surface);
  font: var(--md-sys-typescale-body-large);
  -webkit-appearance: none;
  -moz-appearance: none;
  appearance: none;
  cursor: pointer;
  transition-property: border-radius, background-color;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  &:hover {
    background: color-mix(
      in srgb,
      var(--md-sys-color-on-surface) 8%,
      var(--md-sys-color-surface-container-highest)
    );
  }

  &:focus-visible {
    outline: 3px solid var(--md-sys-color-secondary);
    outline-offset: 2px;
  }

  &:active {
    border-radius: var(--md-sys-shape-corner-small);
  }

  option {
    background: var(--md-sys-color-surface-container);
    color: var(--md-sys-color-on-surface);
  }
}

.arrow {
  position: absolute;
  right: 8px;
  top: 50%;
  transform: translateY(-50%);
  color: var(--md-sys-color-on-surface-variant);
  pointer-events: none;

  svg {
    display: block;
  }
}

.large .builtin-select {
  min-height: 56px;
  padding-left: 20px;
  border-radius: var(--md-sys-shape-corner-large);
  background: var(--side-container, var(--md-sys-color-secondary-container));
  color: var(--side-on-container, var(--md-sys-color-on-secondary-container));
  font: var(--md-sys-typescale-title-medium);

  &:hover {
    background: color-mix(
      in srgb,
      var(--side-on-container, var(--md-sys-color-on-secondary-container)) 8%,
      var(--side-container, var(--md-sys-color-secondary-container))
    );
  }

  &:active {
    border-radius: var(--md-sys-shape-corner-medium);
  }
}

.large .arrow {
  right: 14px;
  color: var(--side-on-container, var(--md-sys-color-on-secondary-container));
}
</style>
