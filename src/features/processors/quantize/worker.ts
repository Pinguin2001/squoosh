/**
 * Copyright 2020 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import type { Options } from './meta';

async function init() {
  const imagequant = await import('codecs/imagequant/pkg/squoosh_imagequant');
  await imagequant.default();
  return imagequant;
}

let wasmReady: ReturnType<typeof init>;

export default async function process(
  data: ImageData,
  opts: Options,
): Promise<ImageData> {
  if (!wasmReady) wasmReady = init();
  const imagequant = await wasmReady;

  // wasm-bindgen copies the pixels in; any byte array view works.
  const pixels = data.data as unknown as Uint8Array;
  const result = opts.zx
    ? imagequant.zx_quantize(pixels, data.width, data.height, opts.dither)
    : imagequant.quantize(
        pixels,
        data.width,
        data.height,
        opts.maxNumColors,
        opts.dither,
      );

  return new ImageData(
    result as Uint8ClampedArray<ArrayBuffer>,
    data.width,
    data.height,
  );
}
