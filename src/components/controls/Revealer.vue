<!-- A chevron icon button that turns as its section opens. -->
<script setup lang="ts">
import { ExpandIcon } from '@/components/icons';

defineOptions({ inheritAttrs: false });
const checked = defineModel<boolean>({ default: false });
</script>

<template>
  <div class="checkbox">
    <input
      v-model="checked"
      class="real-checkbox"
      type="checkbox"
      v-bind="$attrs"
    />
    <div class="arrow">
      <ExpandIcon />
    </div>
  </div>
</template>

<style scoped>
.checkbox {
  display: inline-grid;
  position: relative;
  margin: -8px 0 -8px -8px;
}

.arrow {
  display: grid;
  place-items: center;
  width: 40px;
  height: 40px;
  border-radius: 20px;
  color: var(--md-sys-color-on-surface-variant);
  background: transparent;
  transition-property: rotate, background-color, border-radius, color;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
  rotate: -90deg;

  svg {
    display: block;
  }
}

.checkbox:has(:focus-visible) .arrow {
  outline: 3px solid var(--md-sys-color-secondary);
  outline-offset: 0;
}

.real-checkbox {
  top: 0;
  position: absolute;
  opacity: 0;
  pointer-events: none;

  &:checked + .arrow {
    rotate: 0deg;
    /* Selected icon buttons go from round to square. */
    border-radius: var(--md-sys-shape-corner-medium);
    background: var(--side-container, var(--md-sys-color-secondary-container));
    color: var(--side-on-container, var(--md-sys-color-on-secondary-container));
  }
}
</style>
