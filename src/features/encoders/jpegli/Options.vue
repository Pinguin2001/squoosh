<script setup lang="ts">
import { ref } from 'vue';
import Checkbox from '@/components/controls/Checkbox.vue';
import Expander from '@/components/controls/Expander.vue';
import Range from '@/components/controls/Range.vue';
import Revealer from '@/components/controls/Revealer.vue';
import Select from '@/components/controls/Select.vue';
import type { EncodeOptions } from './meta';

const options = defineModel<EncodeOptions>({ required: true });
const showAdvanced = ref(false);
</script>

<template>
  <form class="options-section" @submit.prevent>
    <div class="option-one-cell">
      <Range v-model="options.quality" :min="1" :max="100">Quality:</Range>
    </div>
    <label class="option-reveal">
      <Revealer v-model="showAdvanced" />
      Advanced settings
    </label>
    <Expander>
      <div v-if="showAdvanced">
        <label class="option-text-first">
          Chroma subsampling:
          <Select v-model="options.chroma_subsample">
            <option :value="444">4:4:4 (none)</option>
            <option :value="422">4:2:2</option>
            <option :value="440">4:4:0</option>
            <option :value="420">4:2:0</option>
          </Select>
        </label>
        <div class="option-one-cell">
          <Range v-model="options.progressive_level" :min="0" :max="2">
            Progressive level:
          </Range>
        </div>
        <label class="option-toggle">
          Adaptive quantization
          <Checkbox v-model="options.adaptive_quantization" />
        </label>
        <label class="option-toggle">
          Standard quantization tables
          <Checkbox v-model="options.std_quant_tables" />
        </label>
      </div>
    </Expander>
  </form>
</template>
