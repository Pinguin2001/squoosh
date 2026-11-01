import type { EncodeOptions } from 'codecs/jpegli/enc/jpegli_enc';

export type { EncodeOptions };

export const label = 'Jpegli';
export const mimeType = 'image/jpeg';
export const extension = 'jpg';
export const defaultOptions: EncodeOptions = {
  quality: 75,
  chroma_subsample: 420,
  progressive_level: 2,
  adaptive_quantization: true,
  std_quant_tables: false,
};
