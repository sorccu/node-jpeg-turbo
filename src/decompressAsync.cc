#include "exports.h"

class DecompressWorker : public Nan::AsyncWorker {
  public:
    DecompressWorker(Nan::Callback *callback, unsigned char* srcData, uint32_t srcLength, uint32_t format, uint32_t bpp, v8::Local<v8::Object> &dstObject, unsigned char* dstData, uint32_t dstBufferLength)
      : Nan::AsyncWorker(callback), srcData(srcData), srcLength(srcLength), format(format), bpp(bpp), dstData(dstData), dstBufferLength(dstBufferLength), width(0), height(0), jpegSubsamp(0), dstLength(0) {
        if (dstBufferLength > 0) {
          SaveToPersistent("dstObject", dstObject);
        }
      }
    ~DecompressWorker() {}

    void Execute () {
      int err;

      tjhandle handle = tjInitDecompress();
      if (handle == NULL) {
        SetErrorMessage(tjGetErrorStr());
        return;
      }

      err = tjDecompressHeader2(handle, this->srcData, this->srcLength, &this->width, &this->height, &this->jpegSubsamp);

      if (err != 0) {
        SetErrorMessage(tjGetErrorStr());
        tjDestroy(handle);
        return;
      }

      this->dstLength = this->width * this->height * this->bpp;

      if (this->dstBufferLength > 0) {
        if (this->dstBufferLength < this->dstLength) {
          SetErrorMessage("Insufficient output buffer");
          return;
        }
      }
      else {
        this->dstData = (unsigned char*)malloc(this->dstLength);
      }

      err = tjDecompress2(handle, this->srcData, this->srcLength, this->dstData, this->width, 0, this->height, this->format, TJFLAG_FASTDCT);

      if(err != 0) {
        SetErrorMessage(tjGetErrorStr());
        tjDestroy(handle);
        return;
      }

      err = tjDestroy(handle);
      if (err != 0) {
        SetErrorMessage(tjGetErrorStr());
        return;
      }
    }

    void HandleOKCallback () {
      Nan::HandleScope scope;

      v8::Local<v8::Object> obj = Nan::New<v8::Object>();

      if (this->dstBufferLength > 0) {
        v8::Local<v8::Object> dstObject = GetFromPersistent("dstObject").As<v8::Object>();
        obj->Set(Nan::New("data").ToLocalChecked(), dstObject);
      }
      else {
        v8::Local<v8::Object> dstObject = Nan::NewBuffer((char*)this->dstData, this->dstLength).ToLocalChecked();
        obj->Set(Nan::New("data").ToLocalChecked(), dstObject);
      }

      obj->Set(Nan::New("width").ToLocalChecked(), Nan::New(this->width));
      obj->Set(Nan::New("height").ToLocalChecked(), Nan::New(this->height));
      obj->Set(Nan::New("subsampling").ToLocalChecked(), Nan::New(this->jpegSubsamp));
      obj->Set(Nan::New("size").ToLocalChecked(), Nan::New(this->dstLength));
      obj->Set(Nan::New("bpp").ToLocalChecked(), Nan::New(this->bpp));

      v8::Local<v8::Value> argv[] = {
        Nan::Null(),
        obj
      };

      callback->Call(2, argv);
    }

  private:
    unsigned char* srcData;
    uint32_t srcLength;
    uint32_t format;
    uint32_t bpp;
    unsigned char* dstData;
    uint32_t dstBufferLength;

    int width;
    int height;
    int jpegSubsamp;
    uint32_t dstLength;
};

NAN_METHOD(Decompress) {
  if (info.Length() < 3) {
    return Nan::ThrowError(Nan::TypeError("Too few arguments"));
  }

  int cursor = 0;

  // Input buffer
  v8::Local<v8::Object> srcObject = info[cursor++].As<v8::Object>();
  if (!node::Buffer::HasInstance(srcObject)) {
    return Nan::ThrowError(Nan::TypeError("Invalid source buffer"));
  }

  unsigned char* srcData = (unsigned char*) node::Buffer::Data(srcObject);
  uint32_t srcLength = node::Buffer::Length(srcObject);

  // Output buffer
  v8::Local<v8::Object> dstObject;
  uint32_t dstBufferLength = 0;
  unsigned char* dstData;

  // Options
  v8::Local<v8::Object> options = info[cursor++].As<v8::Object>();

  if (node::Buffer::HasInstance(options) && info.Length() > cursor) {
    dstObject = options;
    options = info[cursor++].As<v8::Object>();
    dstBufferLength = node::Buffer::Length(dstObject);
    dstData = (unsigned char*) node::Buffer::Data(dstObject);
  }

  if (!options->IsObject()) {
    return Nan::ThrowError(Nan::TypeError("Options must be an Object"));
  }

  // Callback
  Nan::Callback *callback = new Nan::Callback(info[cursor++].As<v8::Function>());

  // Format of output buffer
  v8::Local<v8::Value> formatObject = options->Get(Nan::New("format").ToLocalChecked());

  if (formatObject->IsUndefined()) {
    return Nan::ThrowError(Nan::TypeError("Missing format"));
  }

  uint32_t format = formatObject->Uint32Value();

  // Figure out bpp from format (needed to calculate output buffer size)
  uint32_t bpp;
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

  Nan::AsyncQueueWorker(new DecompressWorker(callback, srcData, srcLength, format, bpp, dstObject, dstData, dstBufferLength));
}
