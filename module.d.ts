export type Format = number;
export const FORMAT_RGB: Format;
export const FORMAT_BGR: Format;
export const FORMAT_RGBX: Format;
export const FORMAT_BGRX: Format;
export const FORMAT_XRGB: Format;
export const FORMAT_XBGR: Format;
export const FORMAT_GRAY: Format;
export const FORMAT_RGBA: Format;
export const FORMAT_BGRA: Format;
export const FORMAT_ABGR: Format;
export const FORMAT_ARGB: Format;

export type SubSampling = number;
export const SAMP_444: SubSampling;
export const SAMP_422: SubSampling;
export const SAMP_420: SubSampling;
export const SAMP_GRAY: SubSampling;
export const SAMP_440: SubSampling;

export interface BufferSizeOptions {
  width: number;
  height: number;
  subsampling?: SubSampling;
}

export interface EncodeOptions extends BufferSizeOptions {
  format: Format;
  stride?: number;
  quality?: number;
}

export function bufferSize(options: BufferSizeOptions): number;
export function compressSync(raw: Buffer, options: EncodeOptions): Buffer;
export function compressSync(raw: Buffer, preallocatedOut: Buffer, options: EncodeOptions): Buffer;
export function compress(raw: Buffer, options: EncodeOptions, callback: (err: any, image: Buffer) => void): void;
export function compress(
  raw: Buffer,
  preallocatedOut: Buffer,
  options: EncodeOptions,
  callback: (err: any, image: Buffer) => void
): void;

export interface DecodeOptions {
  format: Format;
}
export interface DecodeResult {
  data: Buffer;
  width: number;
  height: number;
  size: number;
  format: any;
}

export function decompressSync(image: Buffer, preallocatedOut: Buffer, options?: DecodeOptions): DecodeResult;
export function decompressSync(image: Buffer, options?: DecodeOptions): DecodeResult;

export function decompress(
  image: Buffer,
  preallocatedOut: Buffer,
  options: DecodeOptions,
  callback: (err: any, image: Buffer) => void
): DecodeResult;
export function decompress(
  image: Buffer,
  options: DecodeOptions,
  callback: (err: any, image: Buffer) => void
): DecodeResult;
export function decompress(image: Buffer, callback: (err: any, image: Buffer) => void): DecodeResult;
