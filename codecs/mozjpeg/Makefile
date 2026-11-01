CODEC_URL := https://github.com/mozilla/mozjpeg/archive/v4.1.5.tar.gz
CODEC_DIR := node_modules/mozjpeg
CODEC_BUILD_DIR := $(CODEC_DIR)/build
CODEC_OUT := $(CODEC_BUILD_DIR)/libjpeg.a $(CODEC_BUILD_DIR)/rdswitch.o
ENVIRONMENT = worker

OUT_JS := enc/mozjpeg_enc.js
OUT_WASM := $(OUT_JS:.js=.wasm)

.PHONY: all clean

all: $(OUT_JS)

$(filter enc/%,$(OUT_JS)): enc/mozjpeg_enc.cpp

%.js: $(CODEC_OUT)
	$(CXX) \
		-I $(CODEC_DIR) \
		-I $(CODEC_BUILD_DIR) \
		${CXXFLAGS} \
		${LDFLAGS} \
		--bind \
		-s ENVIRONMENT=$(ENVIRONMENT) \
		-s EXPORT_ES6=1 \
		-o $@ \
		$+

$(CODEC_BUILD_DIR)/libjpeg.a: $(CODEC_BUILD_DIR)/Makefile
	$(MAKE) -C $(CODEC_BUILD_DIR) jpeg-static

# set_quality_ratings() lives in cjpeg's rdswitch.c, which isn't part of the
# library, so it's compiled on its own.
$(CODEC_BUILD_DIR)/rdswitch.o: $(CODEC_BUILD_DIR)/Makefile
	$(CC) -c \
		$(CFLAGS) \
		-I $(CODEC_DIR) \
		-I $(CODEC_BUILD_DIR) \
		-o $@ \
		$(CODEC_DIR)/rdswitch.c

# BUILD is set so the binary doesn't embed the build date.
$(CODEC_BUILD_DIR)/Makefile: $(CODEC_DIR)/CMakeLists.txt
	emcmake cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD=squooshre \
		-DENABLE_SHARED=0 \
		-DWITH_SIMD=0 \
		-DWITH_TURBOJPEG=0 \
		-DPNG_SUPPORTED=0 \
		-DWITH_ARITH_ENC=0 \
		-DWITH_ARITH_DEC=0 \
		-B $(@D) \
		$(<D)

$(CODEC_DIR)/CMakeLists.txt:
	mkdir -p $(@D)
	curl -sL $(CODEC_URL) | tar xz --strip 1 -C $(@D)

clean:
	$(RM) $(OUT_JS) $(OUT_WASM)
	$(MAKE) -C $(CODEC_BUILD_DIR) clean
