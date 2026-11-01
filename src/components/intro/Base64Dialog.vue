<!-- Asks for a base64 image (a data URL or plain base64) and opens it. -->
<script setup lang="ts">
import { ref, useTemplateRef } from 'vue';
import { base64ToFile } from '@/lib/base64';

const emit = defineEmits<{ file: [file: File] }>();

const dialog = useTemplateRef<HTMLDialogElement>('dialog');
const text = ref('');
const error = ref('');

function show() {
  error.value = '';
  dialog.value!.showModal();
}

defineExpose({ show });

async function onSubmit() {
  if (!text.value.trim()) {
    error.value = 'Paste a data URL or base64 first.';
    return;
  }
  try {
    const file = await base64ToFile(text.value);
    dialog.value!.close();
    text.value = '';
    emit('file', file);
  } catch {
    error.value = "That isn't a base64 image SquooshRE can open.";
  }
}
</script>

<template>
  <dialog ref="dialog" class="dialog" aria-labelledby="base64-title">
    <form method="dialog" class="content" @submit.prevent="onSubmit">
      <h2 id="base64-title" class="title">Open a base64 image</h2>
      <textarea
        v-model="text"
        class="input"
        placeholder="data:image/png;base64,iVBORw0KGgo… or plain base64"
        spellcheck="false"
        autocomplete="off"
        autofocus
        :aria-invalid="!!error"
        aria-describedby="base64-error"
        @input="error = ''"
      />
      <p id="base64-error" class="error" role="alert">{{ error }}</p>
      <div class="actions">
        <button
          class="unbutton text-btn"
          type="button"
          @click="dialog!.close()"
        >
          Cancel
        </button>
        <button class="unbutton filled-btn" type="submit">Open</button>
      </div>
    </form>
  </dialog>
</template>

<style scoped>
.dialog {
  box-sizing: border-box;
  width: min(560px, calc(100vw - 32px));
  padding: 0;
  border: none;
  border-radius: var(--md-sys-shape-corner-extra-large);
  background: var(--md-sys-color-surface-container-high);
  color: var(--md-sys-color-on-surface);
  box-shadow: var(--md-sys-elevation-3);

  &::backdrop {
    background: rgb(0 0 0 / 0.4);
  }

  &[open] {
    animation: dialog-in var(--md-sys-motion-spring-default-spatial-duration)
      var(--md-sys-motion-spring-fast-spatial);
  }
}

.content {
  display: grid;
  gap: 16px;
  padding: 24px;
}

.title {
  margin: 0;
  font: var(--md-sys-typescale-headline-small);
}

.input {
  box-sizing: border-box;
  width: 100%;
  min-height: 160px;
  padding: 12px 16px;
  border: 1px solid var(--md-sys-color-outline);
  border-radius: var(--md-sys-shape-corner-medium);
  background: transparent;
  color: inherit;
  font:
    0.875rem / 1.4 ui-monospace,
    monospace;
  word-break: break-all;
  resize: vertical;

  &:focus-visible {
    outline: 2px solid var(--md-sys-color-primary);
    outline-offset: -1px;
  }

  &[aria-invalid='true'] {
    border-color: var(--md-sys-color-error);
  }
}

.error {
  min-height: 1.25em;
  margin: -8px 0 0;
  color: var(--md-sys-color-error);
  font: var(--md-sys-typescale-body-medium);
}

.actions {
  display: flex;
  justify-content: end;
  gap: 8px;
}

.text-btn,
.filled-btn {
  min-height: 40px;
  padding: 0 24px;
  border-radius: var(--md-sys-shape-corner-full);
  font: var(--md-sys-typescale-label-large);
  transition-property: border-radius, background-color;
  transition-duration: var(--md-sys-motion-spring-fast-spatial-duration);
  transition-timing-function: var(--md-sys-motion-spring-fast-spatial);

  &:active {
    border-radius: var(--md-sys-shape-corner-small);
  }
}

.text-btn {
  color: var(--md-sys-color-primary);

  &:hover {
    background: color-mix(in srgb, currentColor 8%, transparent);
  }
}

.filled-btn {
  background: var(--md-sys-color-primary);
  color: var(--md-sys-color-on-primary);
}

@keyframes dialog-in {
  from {
    opacity: 0;
    scale: 0.9;
  }
}
</style>
