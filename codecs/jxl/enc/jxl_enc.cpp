#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <jxl/encode.h>
#include <jxl/encode_cxx.h>

#include <algorithm>
#include <cmath>
#include <vector>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>
#endif

using namespace emscripten;

thread_local const val Uint8Array = val::global("Uint8Array");

struct JXLOptions {
  int effort;
  float quality;
  bool progressive;
  int epf;
  bool lossyPalette;
  size_t decodingSpeedTier;
  float photonNoiseIso;
  bool lossyModular;
};

// Quality settings roughly match libjpeg qualities.
float distanceFromQuality(float quality) {
  float distance =
      quality >= 30 ? 0.1 + (100 - quality) * 0.09 : 6.4 + pow(2.5, (30 - quality) / 5.0f) / 6.25f;
  // The largest distance libjxl accepts.
  return std::min(distance, 25.0f);
}

val encode(std::string image, int width, int height, JXLOptions options) {
  auto encoder = JxlEncoderMake(/*memory_manager=*/nullptr);
  if (!encoder)
    return val::null();
  JxlEncoder* enc = encoder.get();

#ifdef __EMSCRIPTEN_PTHREADS__
  auto runner = JxlThreadParallelRunnerMake(/*memory_manager=*/nullptr,
                                            JxlThreadParallelRunnerDefaultNumWorkerThreads());
  if (JxlEncoderSetParallelRunner(enc, JxlThreadParallelRunner, runner.get()) != JXL_ENC_SUCCESS) {
    return val::null();
  }
#endif

  bool lossless = options.quality == 100;

  JxlBasicInfo info;
  JxlEncoderInitBasicInfo(&info);
  info.xsize = width;
  info.ysize = height;
  info.bits_per_sample = 8;
  info.num_extra_channels = 1;
  info.alpha_bits = 8;
  // Lossless keeps the original colours; lossy encodes in XYB.
  info.uses_original_profile = lossless ? JXL_TRUE : JXL_FALSE;
  if (JxlEncoderSetBasicInfo(enc, &info) != JXL_ENC_SUCCESS)
    return val::null();

  JxlColorEncoding color;
  JxlColorEncodingSetToSRGB(&color, /*is_gray=*/JXL_FALSE);
  if (JxlEncoderSetColorEncoding(enc, &color) != JXL_ENC_SUCCESS)
    return val::null();

  JxlEncoderFrameSettings* settings = JxlEncoderFrameSettingsCreate(enc, nullptr);
  auto set = [settings](JxlEncoderFrameSettingId id, int64_t value) {
    return JxlEncoderFrameSettingsSetOption(settings, id, value) == JXL_ENC_SUCCESS;
  };

  bool ok = set(JXL_ENC_FRAME_SETTING_EFFORT, options.effort) &&
            set(JXL_ENC_FRAME_SETTING_EPF, options.epf) &&
            set(JXL_ENC_FRAME_SETTING_DECODING_SPEED, options.decodingSpeedTier) &&
            JxlEncoderFrameSettingsSetFloatOption(settings, JXL_ENC_FRAME_SETTING_PHOTON_NOISE,
                                                  options.photonNoiseIso) == JXL_ENC_SUCCESS;

  bool modular = lossless || options.lossyModular;
  if (lossless) {
    ok = ok && JxlEncoderSetFrameLossless(settings, JXL_TRUE) == JXL_ENC_SUCCESS;
  } else {
    ok = ok && JxlEncoderSetFrameDistance(settings, distanceFromQuality(options.quality)) ==
                   JXL_ENC_SUCCESS;
  }
  ok = ok && set(JXL_ENC_FRAME_SETTING_MODULAR, modular ? 1 : 0);

  if (options.lossyPalette) {
    // Near-lossless palette, which assumes no responsive mode.
    ok = ok && set(JXL_ENC_FRAME_SETTING_LOSSY_PALETTE, 1) &&
         set(JXL_ENC_FRAME_SETTING_PALETTE_COLORS, 0) &&
         set(JXL_ENC_FRAME_SETTING_MODULAR_PREDICTOR, 0) &&
         set(JXL_ENC_FRAME_SETTING_RESPONSIVE, 0);
  }

  if (options.progressive) {
    ok = ok && set(JXL_ENC_FRAME_SETTING_QPROGRESSIVE_AC, 1) &&
         set(JXL_ENC_FRAME_SETTING_RESPONSIVE, 1);
    if (!modular) {
      ok = ok && set(JXL_ENC_FRAME_SETTING_PROGRESSIVE_DC, 1);
    }
  }
  if (!ok)
    return val::null();

  JxlPixelFormat format = {4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
  if (JxlEncoderAddImageFrame(settings, &format, image.data(), image.size()) != JXL_ENC_SUCCESS) {
    return val::null();
  }
  JxlEncoderCloseInput(enc);

  std::vector<uint8_t> output(64 * 1024);
  uint8_t* next_out = output.data();
  size_t avail_out = output.size();
  JxlEncoderStatus status;
  while ((status = JxlEncoderProcessOutput(enc, &next_out, &avail_out)) ==
         JXL_ENC_NEED_MORE_OUTPUT) {
    size_t offset = next_out - output.data();
    output.resize(output.size() * 2);
    next_out = output.data() + offset;
    avail_out = output.size() - offset;
  }
  if (status != JXL_ENC_SUCCESS)
    return val::null();
  output.resize(next_out - output.data());

  return Uint8Array.new_(typed_memory_view(output.size(), output.data()));
}

EMSCRIPTEN_BINDINGS(my_module) {
  value_object<JXLOptions>("JXLOptions")
      .field("effort", &JXLOptions::effort)
      .field("quality", &JXLOptions::quality)
      .field("progressive", &JXLOptions::progressive)
      .field("lossyPalette", &JXLOptions::lossyPalette)
      .field("decodingSpeedTier", &JXLOptions::decodingSpeedTier)
      .field("photonNoiseIso", &JXLOptions::photonNoiseIso)
      .field("lossyModular", &JXLOptions::lossyModular)
      .field("epf", &JXLOptions::epf);

  function("encode", &encode);
}
