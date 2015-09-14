# node-jpeg-turbo

**node-jpeg-turbo** provides minimal [libjpeg-turbo](http://libjpeg-turbo.virtualgl.org/) bindings for [Node.js](https://nodejs.org/). It is very, very fast compared to other alternatives, such as [node-imagemagick-native](https://github.com/mash/node-imagemagick-native) and [jpeg-js](https://github.com/eugeneware/jpeg-js). Currently the focus is on decompressing images as quickly as possible.

## Requirements

On OS X:

```bash
brew install jpeg-turbo
```

On Ubuntu 12.04, 14.04:

```bash
apt-get install libjpeg-turbo8-dev
```

Others:

Search your package manager for `libjpeg-turbo`, `turbojpeg` or similar. Make sure to install the `-dev` package if available. On Debian `libturbojpeg1-dev` might work.

## Installation

Install from [NPM](https://www.npmjs.com/):

```bash
npm install --save jpeg-turbo
```

## Thanks

* https://github.com/A2K/node-jpeg-turbo-scaler
* https://github.com/mash/node-imagemagick-native

## License

See [LICENSE](LICENSE).

Copyright © Simo Kinnunen. All Rights Reserved.
