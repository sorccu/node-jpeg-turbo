const { decompressSync, decompress, SAMP_444, FORMAT_BGR, FORMAT_BGRA, FORMAT_GRAY } = require("..");
const { promisify } = require("util");
const { readFileSync } = require("fs");
const path = require("path");

const decompress2 = promisify(decompress);

const sampleJpeg1 = readFileSync(path.join(__dirname, "github_logo.jpg"));
const sampleJpeg1Pixels = 560 * 560;

describe("decompress", () => {
  test("check decompressSync parameters", () => {
    const okOptions = {
      format: FORMAT_BGR
    };
    decompressSync(sampleJpeg1, okOptions);
    decompressSync(sampleJpeg1, Buffer.alloc(sampleJpeg1Pixels * 3), okOptions);

    expect(() => decompressSync()).toThrow();
    expect(() => decompressSync(null, okOptions)).toThrow();
    expect(() => decompressSync(undefined, okOptions)).toThrow();
    expect(() => decompressSync({}, okOptions)).toThrow();
  });

  test("check decompressSync options", () => {
    const okOptions = {
      format: FORMAT_BGR
    };
    decompressSync(sampleJpeg1, {});
    decompressSync(sampleJpeg1, okOptions);

    // Format
    expect(() =>
      decompressSync(sampleJpeg1, {
        format: -1
      })
    ).toThrow();
    expect(() =>
      decompressSync(sampleJpeg1, {
        format: 50
      })
    ).toThrow();
  });

  test("check decompressSync dest buffer length", () => {
    const options = {
      format: FORMAT_BGRA
    };
    decompressSync(sampleJpeg1, Buffer.alloc(10000000), options);
    decompressSync(sampleJpeg1, Buffer.alloc(sampleJpeg1Pixels * 4), options);
    decompressSync(sampleJpeg1, Buffer.alloc(sampleJpeg1Pixels * 3), { format: FORMAT_BGR });
    decompressSync(sampleJpeg1, Buffer.alloc(0), options);

    expect(() => decompressSync(sampleJpeg1, Buffer.alloc(10), options)).toThrow();
    expect(() => decompressSync(sampleJpeg1, Buffer.alloc(1000), options)).toThrow();
    expect(() => decompressSync(sampleJpeg1, Buffer.alloc(sampleJpeg1Pixels * 3), options)).toThrow();
  });

  test("check libjpeg errors throw", async () => {
    const source = Buffer.alloc(800);
    const options = {
      format: FORMAT_GRAY
    };
    expect(() => compressSync(source, options)).toThrow();
    expect(async () => await compress2(source, options).rejects).toBeTruthy();
  });

  test("check result length", async () => {
    const target = sampleJpeg1Pixels * 3;
    const dest = Buffer.alloc(target);
    const options = {
      width: 20,
      height: 10,
      format: FORMAT_BGR
    };
    const res1 = decompressSync(sampleJpeg1, dest, options);
    expect(res1.data.length).toEqual(target);
    expect(res1.width).toEqual(560);
    expect(res1.height).toEqual(560);
    expect(res1.format).toEqual(options.format);
    expect(res1.size).toEqual(target);

    const res2 = await decompress2(sampleJpeg1, dest, options);
    expect(res2.data.length).toEqual(target);
    expect(res2).toEqual(res1);

    const res3 = decompressSync(sampleJpeg1, options);
    expect(res3.data.length).toEqual(target);

    const res4 = await decompress2(sampleJpeg1, options);
    expect(res4.data.length).toEqual(target);

    // Overfeed the dest buffer
    const res5 = decompressSync(sampleJpeg1, Buffer.alloc(target * 2), options);
    expect(res5.data.length).toEqual(target);
  });
});
