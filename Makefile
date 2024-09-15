GAME ?= all
ARGS ?=

all: build

build:
	cmake -S . -B build
	@ln -sf build/compile_commands.json compile_commands.json
	@echo
	cmake --build build --target $(GAME)

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
	cd ./build/$(GAME) && ./$(GAME) $(ARGS)

format:
	find src -name '*.[ch]pp' | xargs clang-format -i

check-format:
	find src -name '*.[ch]pp' | xargs clang-format --dry-run --Werror

check-tidy:
	find src -name '*.[ch]pp' | xargs clang-tidy

.PHONY: all build clean play format check-format check-tidy
