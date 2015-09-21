#include <nan.h>
#include <turbojpeg.h>

// Unfortunately Travis still uses Ubuntu 12.04, and their libjpeg-turbo is
// super old (1.2.0). We still want to build there, but opt in to the new
// flag when possible.
#ifndef TJFLAG_FASTDCT
#define TJFLAG_FASTDCT 0
#endif

enum {
  FORMAT_RGB  = TJPF_RGB,
  FORMAT_BGR  = TJPF_BGR,
  FORMAT_RGBX = TJPF_RGBX,
  FORMAT_BGRX = TJPF_BGRX,
  FORMAT_XRGB = TJPF_XRGB,
  FORMAT_XBGR = TJPF_XBGR,
  FORMAT_GRAY = TJPF_GRAY,
  FORMAT_RGBA = TJPF_RGBA,
  FORMAT_BGRA = TJPF_BGRA,
  FORMAT_ABGR = TJPF_ABGR,
  FORMAT_ARGB = TJPF_ARGB,
};

NAN_METHOD(DecompressSync) {
  int err;
  int width, height, jpegSubsamp;
  unsigned char* srcData;
  unsigned char* dstData;
  uint32_t srcLength, dstLength;
  uint32_t bpp;
  const char* tjErr;

  if (info.Length() < 2) {
    return Nan::ThrowError(Nan::TypeError("Too few arguments"));
  }

  v8::Local<v8::Object> srcObject = info[0].As<v8::Object>();
  if (!node::Buffer::HasInstance(srcObject)) {
    return Nan::ThrowError(Nan::TypeError("Invalid source buffer"));
  }

  srcData = (unsigned char*) node::Buffer::Data(srcObject);
  srcLength = node::Buffer::Length(srcObject);

  v8::Local<v8::Object> options = info[1].As<v8::Object>();
  if (!options->IsObject()) {
    return Nan::ThrowError(Nan::TypeError("Options must be an Object"));
  }

  uint32_t format =
    options->Get(Nan::New("format").ToLocalChecked())->Uint32Value();

  switch (format) {
  case FORMAT_GRAY:
    bpp = 1;
    break;
  case FORMAT_RGB:
  case FORMAT_BGR:
    bpp = 3;
    break;
  case FORMAT_RGBX:
  case FORMAT_BGRX:
  case FORMAT_XRGB:
  case FORMAT_XBGR:
  case FORMAT_RGBA:
  case FORMAT_BGRA:
  case FORMAT_ABGR:
  case FORMAT_ARGB:
    bpp = 4;
    break;
  default:
    return Nan::ThrowError(Nan::TypeError("Invalid output format"));
  }

  v8::Local<v8::Object> dstObject =
    options->Get(Nan::New("out").ToLocalChecked()).As<v8::Object>();

  if (!node::Buffer::HasInstance(dstObject)) {
    return Nan::ThrowError(Nan::TypeError("Invalid output buffer"));
  }

  tjhandle dh = tjInitDecompress();
  if (dh == NULL) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  err = tjDecompressHeader2(
    dh, srcData, srcLength, &width, &height, &jpegSubsamp);

  if (err != 0) {
    tjErr = tjGetErrorStr();
    tjDestroy(dh);
    return Nan::ThrowError(tjErr);
  }

  dstLength = width * height * bpp;

  if (!dstObject->IsUndefined()) {
    if (node::Buffer::Length(dstObject) < dstLength) {
      return Nan::ThrowError("Insufficient output buffer");
    }
  }
  else {
    dstObject = Nan::NewBuffer(dstLength).ToLocalChecked();
  }

  dstData = (unsigned char*) node::Buffer::Data(dstObject);

  err = tjDecompress2(
    dh, srcData, srcLength, dstData, width, 0, height, format, TJFLAG_FASTDCT);

  if (err != 0) {
    tjErr = tjGetErrorStr();
    tjDestroy(dh);
    return Nan::ThrowError(tjErr);
  }

  err = tjDestroy(dh);
  if (err != 0) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  obj->Set(Nan::New("data").ToLocalChecked(), dstObject);
  obj->Set(Nan::New("width").ToLocalChecked(), Nan::New(width));
  obj->Set(Nan::New("height").ToLocalChecked(), Nan::New(height));
  obj->Set(Nan::New("size").ToLocalChecked(), Nan::New(dstLength));
  obj->Set(Nan::New("bpp").ToLocalChecked(), Nan::New(bpp));

  info.GetReturnValue().Set(obj);
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("decompressSync").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(DecompressSync)).ToLocalChecked());
  Nan::Set(target, Nan::New("FORMAT_RGB").ToLocalChecked(), Nan::New(FORMAT_RGB));
  Nan::Set(target, Nan::New("FORMAT_BGR").ToLocalChecked(), Nan::New(FORMAT_BGR));
  Nan::Set(target, Nan::New("FORMAT_RGBX").ToLocalChecked(), Nan::New(FORMAT_RGBX));
  Nan::Set(target, Nan::New("FORMAT_BGRX").ToLocalChecked(), Nan::New(FORMAT_BGRX));
  Nan::Set(target, Nan::New("FORMAT_XRGB").ToLocalChecked(), Nan::New(FORMAT_XRGB));
  Nan::Set(target, Nan::New("FORMAT_XBGR").ToLocalChecked(), Nan::New(FORMAT_XBGR));
  Nan::Set(target, Nan::New("FORMAT_GRAY").ToLocalChecked(), Nan::New(FORMAT_GRAY));
  Nan::Set(target, Nan::New("FORMAT_RGBA").ToLocalChecked(), Nan::New(FORMAT_RGBA));
  Nan::Set(target, Nan::New("FORMAT_BGRA").ToLocalChecked(), Nan::New(FORMAT_BGRA));
  Nan::Set(target, Nan::New("FORMAT_ABGR").ToLocalChecked(), Nan::New(FORMAT_ABGR));
  Nan::Set(target, Nan::New("FORMAT_ARGB").ToLocalChecked(), Nan::New(FORMAT_ARGB));
}

// There is no semi-colon after NODE_MODULE as it's not a function (see node.h).
// see http://nodejs.org/api/addons.html
NODE_MODULE(JpegTurbo, InitAll)
