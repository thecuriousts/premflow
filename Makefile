BUILD_DIR ?= build
ELOMAXZ_SOURCE_DIR ?=
CMAKE_FLAGS := -B $(BUILD_DIR)
ifneq ($(ELOMAXZ_SOURCE_DIR),)
CMAKE_FLAGS += -DELOMAXZ_SOURCE_DIR=$(ELOMAXZ_SOURCE_DIR)
endif

SRC := main.c app.c app.h effects.c core.c ui.c test.c premflow.h

.PHONY: all clean test install uninstall configure format format-check

format:
	clang-format -i $(SRC)

format-check:
	@clang-format --dry-run -Werror $(SRC)

configure:
	cmake $(CMAKE_FLAGS)

all: configure
	cmake --build $(BUILD_DIR)

test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

install: all
	cmake --install $(BUILD_DIR) --prefix $(or $(PREFIX),$(HOME)/.local)

uninstall:
	rm -f $(or $(PREFIX),$(HOME)/.local)/bin/premflow
	@echo "✅ premflow uninstalled"

run: all
	./$(BUILD_DIR)/premflow
