GAME ?= all
ARGS ?=
SANITIZER ?=

PROFILE ?= OFF
TRACY_PORT ?= 9000
TRACY_OUTPUT ?= output.tracy
TRACY_CAPTURE_TIME ?= 300

all: build

build:
	cmake -S . -B build -DSANITIZER=$(SANITIZER) -DPROFILE=${PROFILE}
	@ln -sf build/compile_commands.json compile_commands.json
	@echo
	cmake --build build --target $(GAME) -- -j8

build-tracy-gui:
	cd vendor/tracy && cmake -B profiler/build -S profiler -DCMAKE_BUILD_TYPE=Release
	cd vendor/tracy && cmake --build profiler/build --config Release

build-tracy-capture:
	cd vendor/tracy && cmake -B capture/build -S capture -DCMAKE_BUILD_TYPE=Release
	cd vendor/tracy && cmake --build capture/build --config Release

open-tracy-gui:
	nohup ./vendor/tracy/profiler/build/tracy-profiler </dev/null >/dev/null 2>&1 &

clean:
	rm -rf build compile_commands.json .cache

play:
	@if [ "$(GAME)" = "all" ]; then \
		echo "Usage:"; \
		echo "make play GAME=<game> ARGS=<game_args> [ TRACY_PORT=<port> ]"; \
		echo; \
		echo "Where:"; \
		echo "<game> is one of:"; \
		cat .targetgames; \
		echo; \
		echo; \
		exit 1; \
	fi
	cd ./build/$(GAME) && TRACY_PORT=$(TRACY_PORT) ./$(GAME) $(ARGS)

play-profile:
	@if [ "$(GAME)" = "all" ]; then \
		echo "Usage:"; \
		echo "make play-profile GAME=<game> ARGS=<game_args> TRACY_PORT=<port> TRACY_OUTPUT=<file>.tracy TRACY_CAPTURE_TIME=<seconds>"; \
		echo; \
		echo "Where:"; \
		echo "<game> is one of:"; \
		cat .targetgames; \
		echo; \
		echo; \
		exit 1; \
	fi
	mkdir -p traces
	cd ./build/$(GAME) && TRACY_PORT=$(TRACY_PORT) ./$(GAME) $(ARGS) &
	./vendor/tracy/capture/build/tracy-capture -o ./traces/$(TRACY_OUTPUT) -p $(TRACY_PORT) -s $(TRACY_CAPTURE_TIME)

format:
	find src -name '*.[ch]pp' | xargs -P 8 -n 1 clang-format -i

check-format:
	find src -name '*.[ch]pp' | xargs -P 8 -n 1 clang-format --dry-run --Werror

check-tidy:
	find src/engine $(shell cat .targetgames | sed 's/^/src\/games\//') -name '*.[ch]pp' -print | xargs -P 8 -n 1 clang-tidy

.PHONY: all build build-tracy-gui build-tracy-capture open-tracy-gui clean play play-profile format check-format check-tidy
