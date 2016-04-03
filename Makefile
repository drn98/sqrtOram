OBLIVCC = $(OBLIVC_PATH)/bin/oblivcc
OBLIVCH = $(OBLIVC_PATH)/src/ext/oblivc
OBLIVCA = $(OBLIVC_PATH)/_build/libobliv.a

.PHONY: all clean
all: build/liboram.a

clean:
	rm -rf build

# Gather up all files in src/ and pack them into liboram.a
LIBORAM_OBJS=$(addprefix build/,\
	       $(patsubst %.c,%.o,$(wildcard src/*.c)) \
	       $(patsubst %.oc,%.oo,$(wildcard src/*.oc)))
build/liboram.a: $(LIBORAM_OBJS)
	ar -rc $@ $^

# Create directories
.PHONY: builddirs
BUILD_SUBDIRS=$(addprefix build/,src test bench)
$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

builddirs: $(BUILD_SUBDIRS)

# Include generated dependencies
-include $(patsubst %.oo,%.od,$(OBJS:.o=.d))

build/%.o: %.c builddirs
	gcc -c $(CFLAGS) $*.c -o $@ -I $(OBLIVCH) -I src
	cpp -MM $(CFLAGS) $*.c -I $(OBLIVCH) -MT $@ > build/$*.d

build/%.oo: %.oc builddirs
	$(OBLIVCC) -c $(CFLAGS) $*.oc -I src -o $@
	cpp -MM $(CFLAGS) $*.oc -MT $@ > build/$*.od

