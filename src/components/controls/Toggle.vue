<!-- Material 3 switch. -->
<script setup lang="ts">
defineOptions({ inheritAttrs: false });
const checked = defineModel<boolean>({ default: false });
</script>

<template>
  <div class="checkbox">
    <input
      v-model="checked"
      class="real-checkbox"
      type="checkbox"
      role="switch"
      v-bind="$attrs"
    />
    <div class="track">
      <div class="thumb">
        <svg class="check" viewBox="0 0 24 24" aria-hidden="true">
          <path
            d="M9.55 18 3.85 12.3l1.43-1.43 4.27 4.28 9.17-9.18 1.43 1.43z"
          />
        </svg>
      </div>
    </div>
  </div>
</template>

<style scoped>
.checkbox {
  display: inline-grid;
  position: relative;
  /* 48px tall touch target around the 32px track */
  padding: 8px 0;
}

.track {
  box-sizing: border-box;
  width: 52px;
  height: 32px;
  border-radius: var(--md-sys-shape-corner-full);
  border: 2px solid var(--md-sys-color-outline);
  background: var(--md-sys-color-surface-container-highest);
  display: flex;
  align-items: center;
  transition-property: background-color, border-color;
  transition-duration: var(--md-sys-motion-spring-default-effects-duration);
  transition-timing-function: var(--md-sys-motion-spring-effects);
}

.thumb {
  --thumb-size: 16px;
  position: relative;
  display: grid;
  place-items: center;
  width: var(--thumb-size);
  height: var(--thumb-size);
  /* Centre of the thumb sits 16px from the left edge (inside the border). */
  margin-left: calc(14px - var(--thumb-size) / 2);
  border-radius: 50%;
  background: var(--md-sys-color-outline);
  transition-property: width, height, margin-left, background-color;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  /* State layer */
  &::before {
    content: '';
    position: absolute;
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background: var(--md-sys-color-on-surface);
    opacity: 0;
    transition: opacity var(--md-sys-motion-spring-fast-effects-duration);
  }
}

.check {
  width: 16px;
  height: 16px;
  fill: var(--side-accent, var(--md-sys-color-primary));
  opacity: 0;
  scale: 0.4;
  transition-property: opacity, scale;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
}

.checkbox:hover .thumb::before {
  opacity: var(--md-sys-state-hover-opacity);
}

.checkbox:active .thumb {
  --thumb-size: 28px;
}

.checkbox:has(:focus-visible) .track {
  outline: 3px solid var(--md-sys-color-secondary);
  outline-offset: 2px;
}

.real-checkbox {
  top: 0;
  position: absolute;
  opacity: 0;
  pointer-events: none;

  &:checked + .track {
    background: var(--side-accent, var(--md-sys-color-primary));
    border-color: var(--side-accent, var(--md-sys-color-primary));

    .thumb {
      --thumb-size: 24px;
      margin-left: calc(34px - var(--thumb-size) / 2);
      background: var(--side-on-accent, var(--md-sys-color-on-primary));

      &::before {
        background: var(--side-accent, var(--md-sys-color-primary));
      }
    }

    .check {
      opacity: 1;
      scale: 1;
    }
  }
}

.checkbox:active .real-checkbox:checked + .track .thumb {
  --thumb-size: 28px;
}
</style>
