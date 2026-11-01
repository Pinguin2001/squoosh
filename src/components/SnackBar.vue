<script setup lang="ts">
import { ref, watch } from 'vue';
import { snackQueue, type Snack } from '@/composables/useSnackbar';

const hiding = ref(false);
let timeoutId: number | undefined;

function finish(snack: Snack, action: string) {
  if (hiding.value) return;
  clearTimeout(timeoutId);
  snack.resolve(action);
  hiding.value = true;
}

function onAnimationEnd() {
  if (!hiding.value) return;
  hiding.value = false;
  snackQueue.shift();
}

watch(
  () => snackQueue[0],
  (snack) => {
    if (snack?.timeout) {
      timeoutId = self.setTimeout(() => finish(snack, ''), snack.timeout);
    }
  },
  { immediate: true },
);
</script>

<template>
  <div class="snack-bar">
    <div
      v-if="snackQueue[0]"
      :key="snackQueue[0].id"
      class="snackbar"
      aria-live="assertive"
      aria-atomic="true"
      :aria-hidden="hiding ? 'true' : 'false'"
      @animationend.self="onAnimationEnd"
    >
      <div class="text">{{ snackQueue[0].message }}</div>
      <button
        v-for="action in snackQueue[0].actions"
        :key="action"
        class="button"
        @click="finish(snackQueue[0], action)"
      >
        {{ action }}
      </button>
    </div>
  </div>
</template>

<style scoped>
.snack-bar {
  display: block;
  position: fixed;
  left: 0;
  bottom: 0;
  width: 100%;
  height: 0;
  overflow: visible;
}

.snackbar {
  position: fixed;
  display: flex;
  align-items: center;
  box-sizing: border-box;
  left: 50%;
  bottom: 24px;
  min-height: 48px;
  width: 344px;
  max-width: calc(100% - 24px);
  translate: -50% 0;
  padding: 4px 8px 4px 16px;
  background: var(--md-sys-color-inverse-surface);
  color: var(--md-sys-color-inverse-on-surface);
  font: var(--md-sys-typescale-body-medium);
  border-radius: var(--md-sys-shape-corner-large);
  box-shadow: var(--md-sys-elevation-3);
  transform-origin: center bottom;
  z-index: 100;
  cursor: default;
  will-change: transform;
  animation: snackbar-show var(--md-sys-motion-spring-default-spatial-duration)
    var(--md-sys-motion-spring-fast-spatial) forwards 1;
}
.snackbar[aria-hidden='true'] {
  animation: snackbar-hide var(--md-sys-motion-spring-default-effects-duration)
    var(--md-sys-motion-spring-effects) forwards 1;
}
@keyframes snackbar-show {
  from {
    opacity: 0;
    transform: translateY(24px) scale(0.8);
  }
}
@keyframes snackbar-hide {
  to {
    opacity: 0;
    transform: translateY(100%);
  }
}

@media (max-width: 400px) {
  .snackbar {
    bottom: 12px;
  }
}

.text {
  flex: 1 1 auto;
  padding: 10px 0;
}

.button {
  flex: none;
  min-height: 40px;
  margin-left: 8px;
  padding: 0 12px;
  background: none;
  border: none;
  border-radius: var(--md-sys-shape-corner-full);
  color: var(--md-sys-color-inverse-primary);
  font: var(--md-sys-typescale-label-large);
  cursor: pointer;
  transition: background-color var(--md-sys-motion-spring-fast-effects-duration);
}
.button:hover {
  background-color: color-mix(in srgb, currentColor 8%, transparent);
}
.button:focus-visible {
  outline: 3px solid var(--md-sys-color-inverse-primary);
  outline-offset: 0;
}
</style>
