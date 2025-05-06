CC=clang
NAME=mes
OUT=bin/$(NAME)
CFLAGS=-Wall -Wextra -Werror -pedantic -Wunreachable-code \
	   # -fsanitize=address \
	   # -fsanitize=alignment \
	   # -fsanitize=bool \
	   # -fsanitize=bounds \
	   # -fsanitize=bounds-strict \
	   # -fsanitize=enum \
	   # -fsanitize=float-cast-overflow \
	   # -fsanitize=float-divide-by-zero \
	   # -fsanitize=integer-divide-by-zero \
	   # -fsanitize=leak \
	   # -fsanitize=null \
	   # -fsanitize=object-size \
	   # -fsanitize=return \
	   # -fsanitize=shift \
	   # -fsanitize=signed-integer-overflow \
	   # -fsanitize=undefined \
	   # -fsanitize=unreachable \
	   # -fsanitize=vla-bound \


# -fprofile-arcs
#  -> -ftest-coverage	(--coverage)
#  -> -fbranch-probabilities

CFLAGS +=-O3 -flto -g -pthread

sources=main.c bitboard.c chess.c test.c magic.c engine.c uci.c hash.c globals.c

.PHONY: clean run build

build: $(OUT)

clean:
	-rm -Rf bin obj

tests: $(OUT)
	time ./$(OUT) --test

run: $(OUT)
	time ./$(OUT)

ifeq (,$(filter $(MAKECMDGOALS), clean count test))
include $(addprefix dep/, $(sourcecs:.c=.d))
endif

dep/%.d: src/%.c | dep
	$(CC) $(CFLAGS) -M $^ > $@

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

obj dep bin:
	mkdir -p $@

$(OUT): $(addprefix obj/, $(sources:.c=.o)) | bin
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS)
