<!--
  A dropdown menu, positioned against its parent (which needs
  `position: relative` and should contain the button that opens it).
-->
<script setup lang="ts" generic="T extends string">
import { nextTick, onBeforeUnmount, useTemplateRef, watch } from 'vue';

export interface MenuItem<V extends string> {
  value: V;
  label: string;
}

const props = withDefaults(
  defineProps<{
    items: readonly MenuItem<T>[];
    placement?: 'below' | 'above';
    align?: 'start' | 'end';
  }>(),
  { placement: 'below', align: 'start' },
);
const emit = defineEmits<{ select: [value: T] }>();
const open = defineModel<boolean>('open', { required: true });

const list = useTemplateRef<HTMLElement>('list');
/** Focus goes back here when the menu closes. */
let opener: HTMLElement | null = null;

function itemButtons() {
  return [...(list.value?.querySelectorAll('button') ?? [])];
}

function close(restoreFocus: boolean) {
  open.value = false;
  if (restoreFocus) opener?.focus();
}

function onDocumentPointerDown(event: PointerEvent) {
  // The parent holds the opening button, which toggles the menu itself.
  const container = list.value?.parentElement;
  if (!container?.contains(event.target as Node)) close(false);
}

watch(open, async (isOpen) => {
  if (isOpen) {
    opener = document.activeElement as HTMLElement | null;
    document.addEventListener('pointerdown', onDocumentPointerDown);
    await nextTick();
    itemButtons()[0]?.focus();
  } else {
    document.removeEventListener('pointerdown', onDocumentPointerDown);
  }
});

onBeforeUnmount(() =>
  document.removeEventListener('pointerdown', onDocumentPointerDown),
);

function onKeyDown(event: KeyboardEvent) {
  const buttons = itemButtons();
  const index = buttons.indexOf(document.activeElement as HTMLButtonElement);
  if (event.key === 'Escape') {
    event.preventDefault();
    close(true);
  } else if (event.key === 'ArrowDown' || event.key === 'ArrowUp') {
    event.preventDefault();
    const step = event.key === 'ArrowDown' ? 1 : -1;
    buttons[(index + step + buttons.length) % buttons.length]?.focus();
  } else if (event.key === 'Tab') {
    close(false);
  }
}

function onSelect(value: T) {
  close(true);
  emit('select', value);
}
</script>

<template>
  <Transition name="menu">
    <ul
      v-if="open"
      ref="list"
      :class="['menu', `menu-${props.placement}`, `menu-${props.align}`]"
      role="menu"
      @keydown="onKeyDown"
    >
      <li v-for="item in items" :key="item.value" role="none">
        <button
          class="unbutton menu-item"
          role="menuitem"
          type="button"
          @click="onSelect(item.value)"
        >
          {{ item.label }}
        </button>
      </li>
    </ul>
  </Transition>
</template>

<style scoped>
.menu {
  position: absolute;
  z-index: 10;
  min-width: 200px;
  margin: 0;
  padding: 8px 0;
  list-style: none;
  border-radius: var(--md-sys-shape-corner-large);
  background: var(--md-sys-color-surface-container);
  color: var(--md-sys-color-on-surface);
  box-shadow: var(--md-sys-elevation-2);
  text-align: start;
  text-transform: none;
  letter-spacing: normal;
}

.menu-below {
  top: calc(100% + 4px);
  transform-origin: top;
}

.menu-above {
  bottom: calc(100% + 4px);
  transform-origin: bottom;
}

.menu-start {
  left: 0;
}

.menu-end {
  right: 0;
}

.menu-item {
  box-sizing: border-box;
  width: 100%;
  min-height: 48px;
  padding: 0 16px;
  text-align: start;
  white-space: nowrap;
  font: var(--md-sys-typescale-label-large);
  font-size: 1rem;
  transition: background-color var(--md-sys-motion-spring-fast-effects-duration);

  &:hover,
  &:focus-visible {
    background: color-mix(in srgb, currentColor 10%, transparent);
  }

  &:focus-visible {
    outline: none;
  }
}

.menu-enter-active,
.menu-leave-active {
  transition-property: opacity, scale;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);
}

.menu-enter-from,
.menu-leave-to {
  opacity: 0;
  scale: 1 0.8;
}
</style>
