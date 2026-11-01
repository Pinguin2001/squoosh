import { sniffMimeType } from './util';

const extensions: Record<string, string> = {
  'image/png': 'png',
  'image/jpeg': 'jpg',
  'image/gif': 'gif',
  'image/bmp': 'bmp',
  'image/tiff': 'tiff',
  'image/webp': 'webp',
  'image/avif': 'avif',
  'image/jxl': 'jxl',
  'image/qoi': 'qoi',
  'image/svg+xml': 'svg',
};

function decodeBase64(base64: string): Uint8Array<ArrayBuffer> {
  // Also accept URL-safe base64 and missing padding.
  const normalized = base64
    .replace(/\s+/g, '')
    .replace(/-/g, '+')
    .replace(/_/g, '/');
  const padded = normalized.padEnd(Math.ceil(normalized.length / 4) * 4, '=');
  const binary = atob(padded);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
  return bytes;
}

function looksLikeSvg(bytes: Uint8Array): boolean {
  const start = new TextDecoder().decode(bytes.subarray(0, 1024)).trimStart();
  return start.startsWith('<') && /<svg[\s>]/.test(start);
}

/**
 * Turns a data URL or plain base64 into an image file. The type comes from
 * the data URL or, failing that, the file's contents. Throws if the text
 * isn't base64 or isn't an image we know.
 */
export async function base64ToFile(text: string): Promise<File> {
  const dataUrl = /^data:([^;,]*)((?:;[^;,]*)*),(.*)$/s.exec(text.trim());

  let bytes: Uint8Array<ArrayBuffer>;
  let type = '';
  if (dataUrl) {
    const [, mimeType, params, data] = dataUrl;
    type = mimeType.toLowerCase();
    bytes = params.split(';').includes('base64')
      ? decodeBase64(data)
      : new TextEncoder().encode(decodeURIComponent(data));
  } else {
    bytes = decodeBase64(text);
  }

  if (!extensions[type]) {
    type = looksLikeSvg(bytes)
      ? 'image/svg+xml'
      : await sniffMimeType(new Blob([bytes]));
  }
  if (!extensions[type]) throw new Error('Not a known image type');

  return new File([bytes], `image.${extensions[type]}`, { type });
}

/** Whether text could be an image as base64, cheap enough to check on paste. */
export function maybeBase64Image(text: string): boolean {
  const trimmed = text.trim();
  return (
    trimmed.startsWith('data:image/') ||
    (trimmed.length > 64 && /^[A-Za-z0-9+/=_\s-]+$/.test(trimmed))
  );
}

/** A file as plain base64 and as a data URL with its image type. */
export async function fileToBase64(
  file: Blob,
): Promise<{ base64: string; dataUrl: string }> {
  const readerUrl = await new Promise<string>((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result as string);
    reader.onerror = () => reject(reader.error);
    reader.readAsDataURL(file);
  });
  const base64 = readerUrl.slice(readerUrl.indexOf(',') + 1);
  // Pasted images have no type, so look at the contents.
  const type =
    file.type || (await sniffMimeType(file)) || 'application/octet-stream';
  return { base64, dataUrl: `data:${type};base64,${base64}` };
}
