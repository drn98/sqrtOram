SHELL = /bin/bash
OBLIVCC = $(OBLIVC_PATH)/bin/oblivcc
OBLIVCH = $(OBLIVC_PATH)/src/ext/oblivc
OBLIVCA = $(OBLIVC_PATH)/_build/libobliv.a

SRC_DIRS = oram util
INCLUDE_FLAGS = $(addprefix -I ,$(SRC_DIRS) $(OBLIVCH))

.PHONY: all clean
all: build/liboram.a

clean:
	rm -rf build

# Gather up all files in oram/ and pack them into liboram.a
LIBORAM_OBJS=$(addprefix build/,\
	       $(patsubst %.c,%.o,$(wildcard oram/*.c)) \
	       $(patsubst %.oc,%.oo,$(wildcard oram/*.oc)))
build/liboram.a: $(LIBORAM_OBJS)
	ar -rc $@ $^

# Create directories
.PHONY: builddirs
BUILD_SUBDIRS=$(addprefix build/,$(SRC_DIRS) test bench)
$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

builddirs: $(BUILD_SUBDIRS)

# Include generated dependencies
-include $(patsubst %.oo,%.od,$(OBJS:.o=.d))

build/%.o: %.c | builddirs
	gcc -c $(CFLAGS) $*.c -o $@ $(INCLUDE_FLAGS)
	cpp -MM $(CFLAGS) $*.c $(INCLUDE_FLAGS) -MT $@ > build/$*.d

build/%.oo: %.oc | builddirs
	$(OBLIVCC) -c $(CFLAGS) $*.oc $(INCLUDE_FLAGS) -o $@
	cpp -MM $(CFLAGS) $*.oc $(INCLUDE_FLAGS) -MT $@ > build/$*.od

# Create dependencies
# Some test/ files have only .c files, others have .c and .oc with the same name
build/Makefile-testdeps: $(wildcard test/*.c test/*.oc) | builddirs
	> $@
	for f in `ls test/*.c`; do \
	  if [ -a $${f/%.c/.oc} ]; then \
	    echo build/$${f/%.c/}: build/$${f/%.c/.oo} >> $@; \
	  fi; \
	done

-include build/Makefile-testdeps
# Build test executables
build/test/%: build/test/%.o build/util/util.o build/liboram.a
	$(OBLIVCC) -o $@ $(filter %.o %.oo,$^) -loram -Lbuild