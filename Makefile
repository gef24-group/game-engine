GAME ?= all

all: build

build:
	cmake -S . -B build
	@echo
	cmake --build build --target $(GAME)
	@echo
	@ln -sf build/compile_commands.json compile_commands.json

clean:
	rm -rf build compile_commands.json

play:
	@if [ "$(GAME)" = "all" ]; then \
		echo "Usage: make play GAME=<game>"; \
		echo "Where: <game> is one of:"; \
		ls -1 src/games/; \
		echo; \
		exit 1; \
	fi
	cd ./build/$(GAME) && ./$(GAME)

format:
	find src -name '*.[ch]pp' | xargs clang-format -i

.PHONY: all build clean play format
