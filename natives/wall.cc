#include <Magick++.h>
#include <napi.h>

#include <list>

using namespace std;
using namespace Magick;

Napi::Value Wall(const Napi::CallbackInfo &info) {
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
    list<Image> mid;
    readImages(&frames, Blob(data.Data(), data.Length()));
    coalesceImages(&coalesced, frames.begin(), frames.end());

    for (Image &image : coalesced) {
      image.resize(Geometry("128x128"));
      image.virtualPixelMethod(Magick::TileVirtualPixelMethod);
      image.matteColor("none");
      image.backgroundColor("none");
      image.scale(Geometry("512x512"));
      double arguments[16] = {0,   0, 57,  42, 0,   128, 63,  130,
                              128, 0, 140, 60, 128, 128, 140, 140};
      image.distort(Magick::PerspectiveDistortion, 16, arguments);
      image.scale(Geometry("800x800>"));
      image.magick(type);
      mid.push_back(image);
    }

    optimizeTransparency(mid.begin(), mid.end());

    if (type == "gif") {
      for (Image &image : mid) {
        image.quantizeDitherMethod(FloydSteinbergDitherMethod);
        image.quantize();
        if (delay != 0) image.animationDelay(delay);
      }
    }

    writeImages(mid.begin(), mid.end(), &blob);

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