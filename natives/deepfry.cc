#include <Magick++.h>
#include <napi.h>

#include <list>
#include <iostream>

using namespace std;
using namespace Magick;

Napi::Value Deepfry(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  try {
    Napi::Object obj = info[0].As<Napi::Object>();
    Napi::Buffer<char> data = obj.Get("data").As<Napi::Buffer<char>>();
    string type = obj.Get("type").As<Napi::String>().Utf8Value();
    int delay =
        obj.Has("delay") ? obj.Get("delay").As<Napi::Number>().Int32Value() : 0;

    Blob blob;

    list<Image> frames;
    list<Image> coalesced;
    list<Image> blurred;
    readImages(&frames, Blob(data.Data(), data.Length()));
    coalesceImages(&coalesced, frames.begin(), frames.end());

    for (Image &image : coalesced) {
      Blob temp;
      image.level(QuantumRange / 2, QuantumRange / 2);
      image.quality(1);
      image.magick("JPEG");
      image.write(&temp);
      Image newImage(temp);
      newImage.magick(type);
      newImage.animationDelay(delay == 0 ? image.animationDelay() : delay);
      blurred.push_back(newImage);
    }

    optimizeTransparency(blurred.begin(), blurred.end());

    if (type == "gif") {
      for (Image &image : blurred) {
        image.quantizeDitherMethod(FloydSteinbergDitherMethod);
        image.quantize();
      }
    }

    writeImages(blurred.begin(), blurred.end(), &blob);

    Napi::Object result = Napi::Object::New(env);
    result.Set("data", Napi::Buffer<char>::Copy(env, (char *)blob.data(),
                                                blob.length()));
    result.Set("type", type);
    return result;
  } catch (std::exception const &err) {
    throw Napi::Error::New(env, err.what());
  } catch (...) {
    throw Napi::Error::New(env, "Unknown error");
  }
}