#include <nan.h>
#include <turbojpeg.h>

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
  size_t length;

  if (info.Length() < 1) {
    return Nan::ThrowError("Too few arguments");
  }

  v8::Local<v8::Object> srcObject = info[0].As<v8::Object>();
  srcData = (unsigned char*) node::Buffer::Data(srcObject);
  length = node::Buffer::Length(srcObject);

  tjhandle dh = tjInitDecompress();
  if (dh == NULL) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  err = tjDecompressHeader2(dh, srcData, length, &width, &height, &jpegSubsamp);
  if (err != 0) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  v8::Local<v8::Object> dstObject = Nan::NewBuffer(width * height * 3).ToLocalChecked();
  dstData = (unsigned char*) node::Buffer::Data(dstObject);

  err = tjDecompress2(dh, srcData, length, dstData, width, 0, height, TJPF_RGB, TJFLAG_FASTDCT);
  if (err != 0) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  err = tjDestroy(dh);
  if (err != 0) {
    return Nan::ThrowError(tjGetErrorStr());
  }

  info.GetReturnValue().Set(dstObject);
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("decompressSync").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(DecompressSync)).ToLocalChecked());
}

// There is no semi-colon after NODE_MODULE as it's not a function (see node.h).
// see http://nodejs.org/api/addons.html
NODE_MODULE(JpegTurbo, InitAll)
