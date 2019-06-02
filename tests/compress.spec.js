const { compressSync, compress, bufferSize, SAMP_444, FORMAT_BGR, FORMAT_BGRA, FORMAT_GRAY } = require("..");
const { promisify } = require("util");

const compress2 = promisify(compress);

describe("compress", () => {
  test("check compressSync parameters", () => {
    const okOptions = {
      width: 10,
      height: 10,
      format: FORMAT_BGR
    };
    // Check the options are ok
    const tmpBuffer = Buffer.alloc(300);
    compressSync(tmpBuffer, okOptions);
    compressSync(tmpBuffer, Buffer.alloc(90000), okOptions);

    expect(() => compressSync()).toThrow();
    expect(() => compressSync(null, okOptions)).toThrow();
    expect(() => compressSync(undefined, okOptions)).toThrow();
    expect(() => compressSync({}, okOptions)).toThrow();

    expect(() => compressSync(tmpBuffer, undefined)).toThrow();
    expect(() => compressSync(tmpBuffer, null)).toThrow();
    expect(() => compressSync(tmpBuffer, {})).toThrow();

    expect(() => compressSync(tmpBuffer, undefined, okOptions)).toThrow();
    expect(() => compressSync(tmpBuffer, null, okOptions)).toThrow();
    expect(() => compressSync(tmpBuffer, {}, okOptions)).toThrow();
  });

  test("check compressSync options", () => {
    const okOptions = {
      width: 10,
      height: 10,
      format: FORMAT_BGR
    };
    // Check the options are ok
    const tmpBuffer = Buffer.alloc(300);
    compressSync(tmpBuffer, okOptions);

    // Width & Height
    expect(() =>
      compressSync(tmpBuffer, {
        width: -1,
        height: 10,
        format: FORMAT_BGR
      })
    ).toThrow();
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: -1,
        format: FORMAT_BGR
      })
    ).toThrow();

    // Format
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: -1
      })
    ).toThrow();
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: 50
      })
    ).toThrow();

    // Quality
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: FORMAT_BGR,
        quality: 101
      })
    ).toThrow();
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: FORMAT_BGR,
        quality: -1
      })
    ).toThrow();
    compressSync(tmpBuffer, {
      width: 10,
      height: 10,
      format: FORMAT_BGR,
      quality: 98
    });

    // Subsampling
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: FORMAT_BGR,
        subsampling: -1
      })
    ).toThrow();
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: FORMAT_BGR,
        subsampling: 10
      })
    ).toThrow();
    compressSync(tmpBuffer, {
      width: 10,
      height: 10,
      format: FORMAT_BGR,
      subsampling: SAMP_444
    });

    // Stride
    expect(() =>
      compressSync(tmpBuffer, {
        width: 10,
        height: 10,
        format: FORMAT_BGR,
        stride: -1
      })
    ).toThrow();
    compressSync(Buffer.alloc(100 * 10 * 3), {
      width: 10,
      height: 10,
      format: FORMAT_BGR,
      stride: 100
    });
  });

  test("check compressSync source buffer length", () => {
    let testSize = (options, target) => {
      compressSync(Buffer.alloc(target), options);
      compressSync(Buffer.alloc(target * 2), options);
      expect(() => compressSync(Buffer.alloc(target - 1), options)).toThrow();
    };

    testSize(
      {
        width: 10,
        height: 10,
        format: FORMAT_BGR
      },
      300
    );

    testSize(
      {
        width: 10,
        height: 10,
        format: FORMAT_BGRA
      },
      400
    );

    testSize(
      {
        width: 1,
        height: 20,
        format: FORMAT_BGR
      },
      60
    );

    testSize(
      {
        width: 20,
        height: 10,
        format: FORMAT_BGRA
      },
      800
    );

    expect(() =>
      compressSync(Buffer.alloc(0), {
        width: 20,
        height: 10,
        format: FORMAT_BGRA
      })
    ).toThrow();

    expect(() =>
      compressSync(Buffer.alloc(0), {
        width: -20,
        height: 10,
        format: FORMAT_BGRA
      })
    ).toThrow();
  });

  test("check compressSync dest buffer length", () => {
    const source = Buffer.alloc(800);
    const options = {
      width: 20,
      height: 10,
      format: FORMAT_BGRA
    };
    compressSync(source, Buffer.alloc(10000000), options);
    compressSync(source, Buffer.alloc(0), options);

    expect(() => compressSync(source, Buffer.alloc(10), options)).toThrow();
    expect(() => compressSync(source, Buffer.alloc(1000), options)).toThrow();
  });

  test("check libjpeg errors throw", async () => {
    const source = Buffer.alloc(800);
    const options = {
      width: 20,
      height: 10,
      format: FORMAT_GRAY
    };
    expect(() => compressSync(source, options)).toThrow();
    expect(async () => await compress2(source, options).rejects).toBeTruthy();
  });

  test("check result length", async () => {
    const source = Buffer.alloc(800);
    const dest = Buffer.alloc(10000);
    const options = {
      width: 20,
      height: 10,
      format: FORMAT_BGR
    };
    const res1 = compressSync(source, dest, options);
    expect(res1.length).toBeLessThanOrEqual(dest.length);
    expect(res1.length).toBeGreaterThan(500);

    const res2 = await compress2(source, dest, options);
    expect(res2.size).toEqual(res1.length);

    const res3 = compressSync(source, options);
    expect(res3.length).toEqual(res1.length);

    const res4 = await compress2(source, options);
    expect(res4.size).toEqual(res1.length);
  });

  function generateRandomData(length) {
    const res = Buffer.alloc(length);
    for (let i = 0; i < length; i++) {
      res[i] = Math.round(Math.random() * 255);
    }
    return res;
  }

  test('check with "real" data', async () => {
    const source1 = generateRandomData(30000);
    const options = {
      width: 50,
      height: 200,
      format: FORMAT_BGR
    };
    const dest = Buffer.alloc(bufferSize(options));
    const res1 = compressSync(source1, dest, options);
    expect(res1.length).toBeLessThanOrEqual(dest.length);
    expect(res1.length).toBeGreaterThan(500);

    const res2 = await compress2(source1, dest, options);
    expect(res2.size).toEqual(res1.length);

    const res3 = compressSync(source1, options);
    expect(res3.length).toEqual(res1.length);

    const res4 = await compress2(source1, options);
    expect(res4.size).toEqual(res1.length);
  });
});
