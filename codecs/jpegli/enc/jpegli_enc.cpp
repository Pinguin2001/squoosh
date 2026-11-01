#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <setjmp.h>
#include <cstdlib>

#include "lib/jpegli/encode.h"

using namespace emscripten;

thread_local const val Uint8Array = val::global("Uint8Array");

struct JpegliOptions {
  int quality;
  // 444, 440, 422 or 420.
  int chroma_subsample;
  // 0 is baseline (sequential), 2 is jpegli's default progression.
  int progressive_level;
  bool adaptive_quantization;
  bool std_quant_tables;
};

struct ErrorManager {
  jpeg_error_mgr pub;
  jmp_buf jump;
};

void OnError(j_common_ptr cinfo) {
  longjmp(reinterpret_cast<ErrorManager*>(cinfo->err)->jump, 1);
}

// The same settings as jpegli's cjpegli tool.
val encode(std::string image, int width, int height, JpegliOptions options) {
  jpeg_compress_struct cinfo;
  ErrorManager jerr;
  cinfo.err = jpegli_std_error(&jerr.pub);
  jerr.pub.error_exit = OnError;

  uint8_t* output = nullptr;
  unsigned long output_size = 0;

  if (setjmp(jerr.jump)) {
    jpegli_destroy_compress(&cinfo);
    free(output);
    return val::null();
  }

  jpegli_create_compress(&cinfo);
  jpegli_mem_dest(&cinfo, &output, &output_size);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 4;
  cinfo.in_color_space = JCS_EXT_RGBA;
  if (options.std_quant_tables) {
    jpegli_use_standard_quant_tables(&cinfo);
  }
  jpegli_set_defaults(&cinfo);

  // Luma sampling factors; chroma stays at 1x1.
  int h = options.chroma_subsample == 422 || options.chroma_subsample == 420 ? 2 : 1;
  int v = options.chroma_subsample == 440 || options.chroma_subsample == 420 ? 2 : 1;
  cinfo.comp_info[0].h_samp_factor = h;
  cinfo.comp_info[0].v_samp_factor = v;
  for (int i = 1; i < cinfo.num_components; i++) {
    cinfo.comp_info[i].h_samp_factor = 1;
    cinfo.comp_info[i].v_samp_factor = 1;
  }

  jpegli_enable_adaptive_quantization(&cinfo, options.adaptive_quantization);
  jpegli_set_distance(&cinfo, jpegli_quality_to_distance(options.quality),
                      /*force_baseline=*/TRUE);
  jpegli_set_progressive_level(&cinfo, options.progressive_level);
  cinfo.optimize_coding = TRUE;

  jpegli_start_compress(&cinfo, TRUE);
  const int row_stride = width * 4;
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW row = reinterpret_cast<JSAMPROW>(&image[cinfo.next_scanline * row_stride]);
    jpegli_write_scanlines(&cinfo, &row, 1);
  }
  jpegli_finish_compress(&cinfo);
  jpegli_destroy_compress(&cinfo);

  auto result = Uint8Array.new_(typed_memory_view(output_size, output));
  free(output);
  return result;
}

EMSCRIPTEN_BINDINGS(my_module) {
  value_object<JpegliOptions>("JpegliOptions")
      .field("quality", &JpegliOptions::quality)
      .field("chroma_subsample", &JpegliOptions::chroma_subsample)
      .field("progressive_level", &JpegliOptions::progressive_level)
      .field("adaptive_quantization", &JpegliOptions::adaptive_quantization)
      .field("std_quant_tables", &JpegliOptions::std_quant_tables);

  function("encode", &encode);
}
