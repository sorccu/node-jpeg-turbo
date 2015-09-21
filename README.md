# node-jpeg-turbo

**node-jpeg-turbo** provides minimal [libjpeg-turbo](http://libjpeg-turbo.virtualgl.org/) bindings for [Node.js](https://nodejs.org/). It is very, very fast compared to other alternatives, such as [node-imagemagick-native](https://github.com/mash/node-imagemagick-native) or [jpeg-js](https://github.com/eugeneware/jpeg-js).

Currently the focus is on decompressing images as quickly as possible.

## Requirements

We use [NAN](https://github.com/nodejs/nan) to guarantee maximum v8 API compatibility, so in theory any [Node.js](https://nodejs.org/) or [io.js](https://iojs.org/) version should work fine.

Due to massive linking pain on Ubuntu, we embed and build `libjpeg-turbo` directly with `node-gyp`. Unfortunately this adds an extra requirement, as the build process needs `yasm` to enable all optimizations.

Here's how to install `yasm`:

**On OS X**

```bash
brew install yasm
```

**On Ubuntu 12.04, 14.04**

```bash
apt-get install yasm
```

**Important!** Ubuntu 12.04 comes with GCC 4.6, which is too old to compile the module. More information is available [here](https://github.com/travis-ci/travis-ci/issues/1379).

**On Debian**

```bash
apt-get install yasm
```

**On Alpine Linux**

```bash
apk add yasm
```

**Others**

Search your package manager for `yasm`.

## Installation

Make sure you've got the [requirements](#requirements) installed. Then simply install from [NPM](https://www.npmjs.com/):

```bash
npm install --save jpeg-turbo
```

## Thanks

* https://github.com/A2K/node-jpeg-turbo-scaler
* https://github.com/mash/node-imagemagick-native
* https://github.com/google/skia/blob/master/gyp/libjpeg-turbo.gyp

## License

See [LICENSE](LICENSE).

Copyright © Simo Kinnunen. All Rights Reserved.
