#include "exports.h"
#include "compress.h"

class CompressWorker : public Nan::AsyncWorker {
  public:
    CompressWorker(Nan::Callback *callback, unsigned char* srcData, uint32_t width, uint32_t stride, uint32_t height, uint32_t bpp, uint32_t format, uint32_t jpegSubsamp, int quality, v8::Local<v8::Object> &dstObject, unsigned char* dstData, uint32_t dstBufferLength)
      : Nan::AsyncWorker(callback), srcData(srcData), width(width), stride(stride), height(height), bpp(bpp), format(format), jpegSubsamp(jpegSubsamp), quality(quality),
        dstData(dstData), dstBufferLength(dstBufferLength) {
        if (dstBufferLength > 0) {
          SaveToPersistent("dstObject", dstObject);
        }
      }
    ~CompressWorker() {}

    void Execute () {
      int err;

      // Set up buffers if required
      int flags = TJFLAG_FASTDCT;

      this->dstLength = tjBufSize(this->width, this->height, this->jpegSubsamp);

      if (this->dstBufferLength > 0) {
        if (this->dstLength > this->dstBufferLength) {
          SetErrorMessage("Pontentially insufficient output buffer");
          return;
        }
        flags |= TJFLAG_NOREALLOC;
      }

      tjhandle handle = tjInitCompress();
      if (handle == NULL) {
        SetErrorMessage(tjGetErrorStr());
        return;
      }

      err = tjCompress2(
        handle,
        this->srcData,
        this->width,
        this->stride * this->bpp,
        this->height,
        this->format,
        &this->dstData,
        &this->jpegSize,
        this->jpegSubsamp,
        this->quality,
        flags
      );

      if (err != 0) {
        SetErrorMessage(tjGetErrorStr());
        tjDestroy(handle);
        return;
      }

      err = tjDestroy(handle);
      if (err != 0) {
        SetErrorMessage(tjGetErrorStr());
        if (this->dstBufferLength == 0) {
          tjFree(dstData);
        }
        return;
      }
    }

    void HandleOKCallback () {
      Nan::HandleScope scope;

      v8::Local<v8::Object> obj = Nan::New<v8::Object>();
      v8::Local<v8::Object> dstObject;

      if (this->dstBufferLength > 0) {
        dstObject = GetFromPersistent("dstObject").As<v8::Object>();
      }
      else {
        dstObject = Nan::NewBuffer((char*)this->dstData, this->jpegSize, compressBufferFreeCallback, NULL).ToLocalChecked();
      }

      obj->Set(Nan::New("data").ToLocalChecked(), dstObject);
      obj->Set(Nan::New("size").ToLocalChecked(), Nan::New((uint32_t) this->jpegSize));

      v8::Local<v8::Value> argv[] = {
        Nan::Null(),
        obj
      };

      callback->Call(2, argv);
    }

  private:
    unsigned char* srcData;
    uint32_t width;
    uint32_t stride;
    uint32_t height;
    uint32_t bpp;
    uint32_t format;
    uint32_t jpegSubsamp;
    int quality;
    unsigned char* dstData;
    uint32_t dstBufferLength;

    uint32_t dstLength;
    unsigned long jpegSize;
};

NAN_METHOD(Compress) {
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

  // Output buffer
  v8::Local<v8::Object> dstObject;
  uint32_t dstBufferLength = 0;
  unsigned char* dstData = NULL;

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

  // Format of input buffer
  v8::Local<v8::Value> formatObject = options->Get(Nan::New("format").ToLocalChecked());

  if (formatObject->IsUndefined()) {
    return Nan::ThrowError(Nan::TypeError("Missing format"));
  }

  uint32_t format = formatObject->Uint32Value();

  // Figure out bpp from format (needed later for stride)
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

  // Subsampling
  v8::Local<v8::Value> sampObject = options->Get(Nan::New("subsampling").ToLocalChecked());

  uint32_t jpegSubsamp = sampObject->IsUndefined() ? DEFAULT_SUBSAMPLING : sampObject->Uint32Value();

  switch (jpegSubsamp) {
    case SAMP_444:
    case SAMP_422:
    case SAMP_420:
    case SAMP_GRAY:
    case SAMP_440:
      break;
    default:
      return Nan::ThrowError(Nan::TypeError("Invalid subsampling method"));
  }

  // Width
  v8::Local<v8::Value> widthObject = options->Get(Nan::New("width").ToLocalChecked());

  if (widthObject->IsUndefined()) {
    return Nan::ThrowError(Nan::TypeError("Missing width"));
  }

  uint32_t width = widthObject->Uint32Value();

  // Height
  v8::Local<v8::Value> heightObject = options->Get(Nan::New("height").ToLocalChecked());

  if (heightObject->IsUndefined()) {
    return Nan::ThrowError(Nan::TypeError("Missing height"));
  }

  uint32_t height = heightObject->Uint32Value();

  // Stride
  v8::Local<v8::Value> strideObject = options->Get(Nan::New("stride").ToLocalChecked());

  uint32_t stride = strideObject->IsUndefined() ? width : strideObject->Uint32Value();

  // Quality
  v8::Local<v8::Value> qualityObject = options->Get(Nan::New("quality").ToLocalChecked());

  int quality = qualityObject->IsUndefined() ? DEFAULT_QUALITY : qualityObject->Uint32Value();

  Nan::AsyncQueueWorker(new CompressWorker(callback, srcData, width, stride, height,  bpp, format, jpegSubsamp, quality, dstObject, dstData, dstBufferLength));
}
