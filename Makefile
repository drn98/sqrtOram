# Usage instructions
# ==================
#
# Ensure $OBLIVC_PATH environment variable is set to
# point to the Obliv-C source repository. Running `make` builds build/liboram.a.
# Compile your own programs by running Obliv-C compiler as follows
#
#   oblivcc myprog.c myprog.oc -lobliv -L../path/for/libobliv
#
# Try to keep a separate folder for your own programs.
#
#
# Instructions for modifying liboram
# ==================================
#
# Adding new ORAM constructions
# -----------------------------
#
# Pretty much anything you place in the folder oram/ will get packaged 
# up into build/liboram.a on `make`. So use that for any new code that
# you want to add to liboram.
#
# Adding new test cases
# ---------------------
#
# Use the test/ folder. Right now, there is no makefile target that makes
# builds all test executable (i.e. no automated regression testing). We really
# need one. But till then, any file named test/foo.c will allow you to do
# 'make build/test/foo' which will be linked to liboram. If there is a matching
# test/foo.oc file as well, that will also be linked in to the test file.
#
#
# Adding new test/benchmark helpers
# ---------------------------------
#
# Sometimes you will want extra code shared between many test executables.
# You can't put them in oram/ (or it will get linked into libobliv), and you
# can't put it in test/ (or test/bar.c will create non-sensical rules for 
# build/test/bar). So you can just create a new folder at the repository root
# (like oram/, util/). In fact, util/util.c is indeed shared by test cases
# a lot. If you do, remember to add it to SRC_DIRS = ... below, so that the
# folder is searched for include files and a corresponding folder is created in
# build/.

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
	$(OBLIVCC) -o $@ $(filter %.o %.oo,$^) -loram -Lbuild -lm
