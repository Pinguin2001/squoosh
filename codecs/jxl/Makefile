CODEC_URL = https://github.com/libjxl/libjxl.git
CODEC_VERSION = v0.12.0
CODEC_DIR = node_modules/jxl
# Every supported browser has Wasm SIMD, so there is one SIMD build of the
# libraries. They are compiled with threads enabled; the single-threaded
# outputs just don't use them nor link them in.
CODEC_BUILD_DIR := $(CODEC_DIR)/build/mt-simd
ENVIRONMENT = worker

OUT_JS = enc/jxl_enc.js enc/jxl_enc_mt.js dec/jxl_dec.js
OUT_WASM = $(OUT_JS:.js=.wasm)

LIB_DIR := $(CODEC_BUILD_DIR)/lib
THIRD_PARTY_LIBS := \
	$(CODEC_BUILD_DIR)/third_party/brotli/libbrotlidec.a \
	$(CODEC_BUILD_DIR)/third_party/brotli/libbrotlienc.a \
	$(CODEC_BUILD_DIR)/third_party/brotli/libbrotlicommon.a \
	$(CODEC_BUILD_DIR)/third_party/highway/libhwy.a

.PHONY: all clean

all: $(OUT_JS)

# Define dependencies for all variations of build artifacts.
$(filter enc/%,$(OUT_JS)): enc/jxl_enc.cpp
$(filter dec/%,$(OUT_JS)): dec/jxl_dec.cpp

enc/jxl_enc.js: $(LIB_DIR)/libjxl.a $(LIB_DIR)/libjxl_cms.a
enc/jxl_enc_mt.js: $(LIB_DIR)/libjxl.a $(LIB_DIR)/libjxl_cms.a $(LIB_DIR)/libjxl_threads.a
dec/jxl_dec.js: $(LIB_DIR)/libjxl_dec.a $(LIB_DIR)/libjxl_cms.a

# Compile multithreaded wrappers with -pthread.
enc/jxl_enc_mt.js: CXXFLAGS+=-pthread
# Compile and link everything with SIMD, so link-time optimization keeps it.
$(OUT_JS): CXXFLAGS+=-msimd128

$(OUT_JS):
	$(CXX) \
		$(CXXFLAGS) \
		$(LDFLAGS) \
		-I $(CODEC_DIR)/lib/include \
		-I $(LIB_DIR)/include \
		--bind \
		-s ENVIRONMENT=$(ENVIRONMENT) \
		-s EXPORT_ES6=1 \
		-o $@ \
		$+ \
		$(THIRD_PARTY_LIBS) \
		$$(find $(CODEC_BUILD_DIR) -name 'libskcms*.a')

# All libraries come from one CMake build; make them in one go.
$(LIB_DIR)/libjxl.a $(LIB_DIR)/libjxl_dec.a $(LIB_DIR)/libjxl_cms.a $(LIB_DIR)/libjxl_threads.a &: $(CODEC_BUILD_DIR)/Makefile
	$(MAKE) -C $(CODEC_BUILD_DIR) jxl jxl_dec jxl_cms jxl_threads

$(CODEC_BUILD_DIR)/Makefile: CXXFLAGS+=-msimd128
$(CODEC_BUILD_DIR)/Makefile: CFLAGS+=-msimd128

$(CODEC_BUILD_DIR)/Makefile: $(CODEC_DIR)/CMakeLists.txt
	emcmake cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=0 \
	-DBUILD_TESTING=0 \
	-DJPEGXL_ENABLE_TOOLS=0 \
	-DJPEGXL_ENABLE_BENCHMARK=0 \
	-DJPEGXL_ENABLE_EXAMPLES=0 \
	-DJPEGXL_ENABLE_DOXYGEN=0 \
	-DJPEGXL_ENABLE_MANPAGES=0 \
	-DJPEGXL_ENABLE_JNI=0 \
	-DJPEGXL_ENABLE_SJPEG=0 \
	-DJPEGXL_ENABLE_OPENEXR=0 \
	-DJPEGXL_ENABLE_JPEGLI=0 \
	-DJPEGXL_ENABLE_TRANSCODE_JPEG=0 \
	-DJPEGXL_ENABLE_SKCMS=1 \
	-DJPEGXL_BUNDLE_LIBPNG=0 \
	-DJPEGXL_FORCE_SYSTEM_BROTLI=0 \
	-DJPEGXL_FORCE_SYSTEM_HWY=0 \
	-DCMAKE_CROSSCOMPILING_EMULATOR=node \
	-B $(@D) \
	$(<D)

$(CODEC_DIR)/CMakeLists.txt:
	$(RM) -r $(@D)
	git init $(@D)
	git -C $(@D) fetch $(CODEC_URL) refs/tags/$(CODEC_VERSION) --depth 1
	git -C $(@D) checkout FETCH_HEAD
	git -C $(@D) submodule update --init --depth 1 --jobs `nproc` third_party/brotli third_party/highway third_party/skcms

clean:
	$(RM) $(OUT_JS) $(OUT_WASM)
	$(MAKE) -C $(CODEC_BUILD_DIR) clean
