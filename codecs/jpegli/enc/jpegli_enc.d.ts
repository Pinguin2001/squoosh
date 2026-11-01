/// <reference path="../../../emscripten-types.d.ts" />

export type JpegliChromaSubsample = 444 | 440 | 422 | 420;

export interface EncodeOptions {
  quality: number;
  chroma_subsample: JpegliChromaSubsample;
  progressive_level: number;
  adaptive_quantization: boolean;
  std_quant_tables: boolean;
}

export interface JpegliModule extends EmscriptenWasm.Module {
  encode(
    data: BufferSource,
    width: number,
    height: number,
    options: EncodeOptions,
  ): Uint8Array<ArrayBuffer> | null;
}

declare var moduleFactory: EmscriptenWasm.ModuleFactory<JpegliModule>;

export default moduleFactory;
