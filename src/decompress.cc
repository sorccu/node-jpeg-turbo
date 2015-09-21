#include "exports.h"

// Unfortunately Travis still uses Ubuntu 12.04, and their libjpeg-turbo is
// super old (1.2.0). We still want to build there, but opt in to the new
// flag when possible.
#ifndef TJFLAG_FASTDCT
#define TJFLAG_FASTDCT 0
#endif

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

  if (!dstObject->IsUndefined() && !node::Buffer::HasInstance(dstObject)) {
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
