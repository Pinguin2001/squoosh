import type { JpegliModule } from 'codecs/jpegli/enc/jpegli_enc';
import type { EncodeOptions } from './meta';
import { initEmscriptenModule } from '@/features/worker-utils';

let emscriptenModule: Promise<JpegliModule>;

export default async function encode(
  data: ImageData,
  options: EncodeOptions,
): Promise<ArrayBuffer> {
  if (!emscriptenModule) {
    const jpegliEncoder = await import('codecs/jpegli/enc/jpegli_enc');
    emscriptenModule = initEmscriptenModule(jpegliEncoder.default);
  }

  const module = await emscriptenModule;
  const result = module.encode(data.data, data.width, data.height, options);

  if (!result) throw new Error('Encoding error.');

  return result.buffer;
}
