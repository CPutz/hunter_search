MOD=gnu
BUILD_DIR=$(CURDIR)/build

GMP_ARCHIVE=gmp-6.1.1.tar.xz
GMP_BUILD_DIR=$(BUILD_DIR)/gmp
GMP_DIR=$(BUILD_DIR)/$(GMP_ARCHIVE:%.tar.xz=%)

default:
	@echo "make install-all to install gmp library into share/$(MOD)"
	@echo "make build-all to only build gmp library into $(BUILD_DIR)"

install-all: build-all install-gmp

install-gmp: build-gmp install-env
	cp -av $(GMP_BUILD_DIR)/* $(MOD)/

build-all: build-gmp

build-gmp: build-env
	tar -xJvf $(GMP_ARCHIVE) -C $(BUILD_DIR)
	cd $(GMP_DIR) && ./configure --prefix=$(GMP_BUILD_DIR) --exec-prefix=$(GMP_BUILD_DIR)
	$(MAKE) -C $(GMP_DIR) install

install-env:
	mkdir -vp $(MOD)

build-env:
	mkdir -vp $(GMP_BUILD_DIR) $(PARI_BUILD_DIR)

clean:
	rm -rv $(BUILD_DIR)
