APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)
ARCH = x86
APPS_LIBS_ROOT ?= $(APPS_ROOT)/libs/$(ARCH)

TRILINOS_DIR=$(APPS_ROOT)/libs/src/trilinos
SRC_DIR=$(TRILINOS_DIR)/publicTrilinos
BUILD_DIR=$(TRILINOS_DIR)/build
INSTALL_DIR=$(TRILINOS_DIR)/install

#
# Configure kokkos
# FIXME?
# this actualy builds the libraries. I added the "install"
# to get everything into the install/lib and install/include directories
# However the REAL install is done here ... hum....
#
build: $(BUILD_DIR)/Makefile
	@echo Building $(BUILD_DIR)
	@cd $(BUILD_DIR); $(MAKE) -j$(MAKE_JOBS) install

CMAKE=cmake

CMAKE_ARGS=\
    -DCMAKE_BUILD_TYPE:STRING=DEBUG \
    -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
    -DTPL_ENABLE_Pthread=OFF \
    -DTrilinos_ENABLE_TESTS:BOOL=OFF \
    -DTrilinos_ENABLE_Kokkos=ON \
    -DTrilinos_ENABLE_Fortran=OFF \
    -DTrilinos_ASSERT_MISSING_PACKAGES=OFF \
    -DBUILD_SHARED_LIBS=OFF
#
#  Configure a build
#
$(BUILD_DIR)/Makefile:
	@[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)
	@echo Running cmake...
	( cd $(BUILD_DIR); \
	  $(CMAKE) $(CMAKE_ARGS) $(SRC_DIR) )
	@echo Cmake done

DOCONF_ARGS=-D Trilinos_ENABLE_Kokkos=ON

#
# Copy from the tmp install tree to the actual install tree
#
.PHONY: install
install:
	@echo Installing Kokkos to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)
	@cp -u -R $(INSTALL_DIR)/include $(APPS_LIBS_ROOT)/
	@cp -u -R $(INSTALL_DIR)/lib $(APPS_LIBS_ROOT)/
	@echo Installation complete

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(INSTALL_DIR)
