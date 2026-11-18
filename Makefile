CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -O1 -g
CORE := $(wildcard shuttleref/core/src/*.c)
INC := -Ishuttleref/core/include -Ishuttleref/tests
TESTS := score_engine match_session ring_protocol gesture_detector phone_command harness_main
BINS := $(addprefix build/test_,$(TESTS))
.PHONY: test core-tests
core-tests: $(BINS)
	@for binary in $(BINS); do ./$$binary || exit 1; done
build/test_%: shuttleref/tests/test_%.c $(CORE) | build
	$(CC) $(CFLAGS) $(INC) $< $(CORE) -o $@
build:
	mkdir -p build
test: core-tests
