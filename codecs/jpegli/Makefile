CODEC_URL = https://github.com/google/jpegli.git
# jpegli has no releases yet; this is main of 2026-06-01.
CODEC_VERSION = 031a0077f5799a6041004267fc12b956c1f52a20
CODEC_DIR = node_modules/jpegli
CODEC_BUILD_DIR := $(CODEC_DIR)/build
ENVIRONMENT = worker

OUT_JS = enc/jpegli_enc.js
OUT_WASM = $(OUT_JS:.js=.wasm)

CODEC_LIBS := \
	$(CODEC_BUILD_DIR)/lib/libjpegli-static.a \
	$(CODEC_BUILD_DIR)/third_party/highway/libhwy.a

.PHONY: all clean

all: $(OUT_JS)

# Compile and link with SIMD, so link-time optimization keeps it.
$(OUT_JS): CXXFLAGS+=-msimd128

$(OUT_JS): enc/jpegli_enc.cpp $(CODEC_LIBS)
	$(CXX) \
		$(CXXFLAGS) \
		$(LDFLAGS) \
		-I $(CODEC_DIR) \
		-I $(CODEC_BUILD_DIR)/lib/include/jpegli \
		--bind \
		-s ENVIRONMENT=$(ENVIRONMENT) \
		-s EXPORT_ES6=1 \
		-o $@ \
		$+

$(CODEC_LIBS) &: $(CODEC_BUILD_DIR)/Makefile
	$(MAKE) -C $(CODEC_BUILD_DIR) jpegli-static hwy

$(CODEC_BUILD_DIR)/Makefile: CXXFLAGS+=-msimd128
$(CODEC_BUILD_DIR)/Makefile: CFLAGS+=-msimd128

# Only the encoder library is built; the encoder doesn't use threads.
$(CODEC_BUILD_DIR)/Makefile: $(CODEC_DIR)/CMakeLists.txt
	emcmake cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=0 \
		-DBUILD_TESTING=0 \
		-DJPEGLI_ENABLE_TOOLS=0 \
		-DJPEGLI_ENABLE_DEVTOOLS=0 \
		-DJPEGLI_ENABLE_FUZZERS=0 \
		-DJPEGLI_ENABLE_BENCHMARK=0 \
		-DJPEGLI_ENABLE_DOXYGEN=0 \
		-DJPEGLI_ENABLE_MANPAGES=0 \
		-DJPEGLI_ENABLE_JNI=0 \
		-DJPEGLI_ENABLE_SJPEG=0 \
		-DJPEGLI_ENABLE_OPENEXR=0 \
		-DJPEGLI_ENABLE_JPEGLI_LIBJPEG=0 \
		-DJPEGLI_ENABLE_SKCMS=1 \
		-DJPEGLI_BUNDLE_LIBPNG=0 \
		-DJPEGLI_ENABLE_WASM_THREADS=0 \
		-DCMAKE_CROSSCOMPILING_EMULATOR=node \
		-B $(@D) \
		$(<D)

$(CODEC_DIR)/CMakeLists.txt:
	$(RM) -r $(@D)
	git init $(@D)
	git -C $(@D) fetch $(CODEC_URL) $(CODEC_VERSION) --depth 1
	git -C $(@D) checkout FETCH_HEAD
	git -C $(@D) submodule update --init --depth 1 --jobs `nproc` third_party/highway third_party/skcms third_party/libjpeg-turbo

clean:
	$(RM) $(OUT_JS) $(OUT_WASM)
	$(MAKE) -C $(CODEC_BUILD_DIR) clean
