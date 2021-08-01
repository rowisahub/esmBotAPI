#include <Magick++.h>
#include <napi.h>

#include <list>

using namespace std;
using namespace Magick;

Napi::Value Spin(const Napi::CallbackInfo &info) {
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

    if (type != "gif") {
      list<Image>::iterator it = coalesced.begin();
      for (int i = 0; i < 29; ++i) {
        coalesced.push_back(*it);
      }
    }

    int i = 0;
    for (Image &image : coalesced) {
      image.virtualPixelMethod(Magick::TransparentVirtualPixelMethod);
      image.scale(Geometry("256x256"));
      image.alphaChannel(Magick::SetAlphaChannel);
      double rotation[1] = {360 * i / coalesced.size()};
      image.distort(Magick::ScaleRotateTranslateDistortion, 1, rotation);
      image.magick("GIF");
      mid.push_back(image);
      i++;
    }

    for_each(mid.begin(), mid.end(),
             gifDisposeMethodImage(Magick::BackgroundDispose));

    optimizeTransparency(mid.begin(), mid.end());
    if (delay != 0) {
      for_each(mid.begin(), mid.end(), animationDelayImage(delay));
    } else if (type != "gif") {
      for_each(mid.begin(), mid.end(), animationDelayImage(5));
    }

    for (Image &image : mid) {
      image.quantizeDitherMethod(FloydSteinbergDitherMethod);
      image.quantize();
    }

    writeImages(mid.begin(), mid.end(), &blob);

    Napi::Object result = Napi::Object::New(env);
    result.Set("data", Napi::Buffer<char>::Copy(env, (char *)blob.data(),
                                                blob.length()));
    result.Set("type", "gif");
    return result;
  } catch (std::exception const &err) {
    throw Napi::Error::New(env, err.what());
  } catch (...) {
    throw Napi::Error::New(env, "Unknown error");
  }
}