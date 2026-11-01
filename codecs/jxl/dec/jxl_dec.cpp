#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <jxl/cms.h>
#include <jxl/color_encoding.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>

#include <vector>

using namespace emscripten;

thread_local const val Uint8ClampedArray = val::global("Uint8ClampedArray");
thread_local const val ImageData = val::global("ImageData");

#define EXPECT_SUCCESS(a)       \
  if ((a) != JXL_DEC_SUCCESS) { \
    return val::null();         \
  }

// Decodes the (first frame of the) image to 8-bit sRGB RGBA. Used only in
// browsers that can't decode JPEG XL themselves.
val decode(std::string data) {
  auto decoder = JxlDecoderMake(/*memory_manager=*/nullptr);
  JxlDecoder* dec = decoder.get();

  EXPECT_SUCCESS(JxlDecoderSubscribeEvents(
      dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE));
  // libjxl converts to sRGB with its own colour management.
  EXPECT_SUCCESS(JxlDecoderSetCms(dec, *JxlGetDefaultCms()));
  EXPECT_SUCCESS(JxlDecoderSetUnpremultiplyAlpha(dec, JXL_TRUE));
  EXPECT_SUCCESS(
      JxlDecoderSetInput(dec, reinterpret_cast<const uint8_t*>(data.data()), data.size()));
  JxlDecoderCloseInput(dec);

  const JxlPixelFormat format = {4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
  JxlBasicInfo info;
  std::vector<uint8_t> pixels;

  for (;;) {
    JxlDecoderStatus status = JxlDecoderProcessInput(dec);

    if (status == JXL_DEC_BASIC_INFO) {
      EXPECT_SUCCESS(JxlDecoderGetBasicInfo(dec, &info));
    } else if (status == JXL_DEC_COLOR_ENCODING) {
      // sRGB (JxlColorEncodingSetToSRGB is only in the encoder library).
      JxlColorEncoding srgb = {};
      srgb.color_space = JXL_COLOR_SPACE_RGB;
      srgb.white_point = JXL_WHITE_POINT_D65;
      srgb.primaries = JXL_PRIMARIES_SRGB;
      srgb.transfer_function = JXL_TRANSFER_FUNCTION_SRGB;
      srgb.rendering_intent = JXL_RENDERING_INTENT_PERCEPTUAL;
      EXPECT_SUCCESS(JxlDecoderSetOutputColorProfile(dec, &srgb, nullptr, 0));
    } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
      size_t buffer_size;
      EXPECT_SUCCESS(JxlDecoderImageOutBufferSize(dec, &format, &buffer_size));
      pixels.resize(buffer_size);
      EXPECT_SUCCESS(JxlDecoderSetImageOutBuffer(dec, &format, pixels.data(), pixels.size()));
    } else if (status == JXL_DEC_FULL_IMAGE) {
      // Animations: only the first frame.
      break;
    } else {
      return val::null();
    }
  }

  return ImageData.new_(Uint8ClampedArray.new_(typed_memory_view(pixels.size(), pixels.data())),
                        info.xsize, info.ysize);
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("decode", &decode);
}
