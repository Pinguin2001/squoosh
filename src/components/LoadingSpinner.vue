<!-- Material 3 Expressive loading indicator: a shape that spins and morphs
     through the expressive shapes. It stays hidden for --delay, so quick loads
     don't flash it. -->
<template>
  <div class="loading-spinner" role="progressbar" aria-label="Loading">
    <div class="shape"></div>
  </div>
</template>

<style scoped>
@keyframes spinner-delay {
  from {
    visibility: hidden;
  }
}

@keyframes spinner-morph {
  0%,
  100% {
    clip-path: var(--shape-soft-burst);
  }
  14% {
    clip-path: var(--shape-cookie-9);
  }
  28% {
    clip-path: var(--shape-pentagon);
  }
  42% {
    clip-path: var(--shape-oval);
  }
  57% {
    clip-path: var(--shape-sunny);
  }
  71% {
    clip-path: var(--shape-cookie-4);
  }
  85% {
    clip-path: var(--shape-circle);
  }
}

@keyframes spinner-rotate {
  to {
    rotate: 1turn;
  }
}

.loading-spinner {
  --size: 48px;
  --color: var(--md-sys-color-primary);
  --delay: 300ms;

  pointer-events: none;
  display: inline-grid;
  place-items: center;
  width: var(--size);
  height: var(--size);
  /* Hidden until the delay is over */
  animation: spinner-delay var(--delay) step-end;
}

.shape {
  /* The indicator is 38/48 of its container in the spec. */
  width: 79%;
  height: 79%;
  background: var(--color);
  clip-path: var(--shape-soft-burst);
  animation:
    spinner-morph 4.9s ease-in-out infinite,
    spinner-rotate 4.9s linear infinite;
}

@media (prefers-reduced-motion: reduce) {
  .shape {
    animation: spinner-rotate 2s linear infinite;
  }
}
</style>
