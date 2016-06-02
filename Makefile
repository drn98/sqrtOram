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
# 'make build/test/foo' which will be linked to liboram. This makefile will
# recursively follow any included header files, find their matching .c or .oc
# source files, and link those in as well. So try not to include files you 
# don't actually use. The bench/ folder works the same way, but used mostly
# for measurement code.
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
CFLAGS=-O3

SRC_DIRS = oram util
TEST_DIRS = test bench
SRC_TEST_DIRS = $(SRC_DIRS) $(TEST_DIRS)
TEST_BINS=$(patsubst %.c,build/%, \
	  $(patsubst %.oc,%,$(shell grep -l '^int main\>' $(TEST_DIRS:=/*))))
ALL_C_FILES = $(wildcard $(addsuffix /*.c,$(SRC_TEST_DIRS)))
ALL_OC_FILES = $(wildcard $(addsuffix /*.oc,$(SRC_TEST_DIRS)))
INCLUDE_FLAGS = $(addprefix -I ,$(SRC_DIRS) $(OBLIVCH))

.PHONY: all test clean
.SECONDARY:
all: build/liboram.a

test: $(TEST_BINS)

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
BUILD_SUBDIRS=$(addprefix build/,$(SRC_TEST_DIRS))
$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

builddirs: $(BUILD_SUBDIRS)

# Include generated dependencies
ifneq (clean,$(MAKECMDGOALS))
  -include $(addprefix build/,$(ALL_C_FILES:.c=.d) $(ALL_OC_FILES:.oc=.od))
endif

localincs=$(filter %.h %.oh,$(filter-out /%,$(filter-out %:,$(shell cat $1))))
srcofh=$(wildcard $(patsubst %.oh,%.oc,$(patsubst %.h,%.c,$1) \
                                       $(patsubst %.h,%.oc,$1)))
objofh=$(patsubst %.c,build/%.o, \
       $(patsubst %.oc,build/%.oo,$(call srcofh,$1)))
dofobj=$(patsubst %.oo,%.od,$(patsubst %.o,%.d,$1))
objdeps=$(call objofh,$(filter-out oram/%,\
	                           $(call localincs,$(call dofobj,$1))))

build/%.d: %.c | builddirs
	cpp -MM $(CFLAGS) $*.c $(INCLUDE_FLAGS) -MT $*.o > $@

build/%.o: %.c build/%.d
	gcc -Wall -std=gnu99 -c $(CFLAGS) $*.c -o $@ $(INCLUDE_FLAGS)

build/%.od: %.oc | builddirs
	cpp -MM $(CFLAGS) $*.oc $(INCLUDE_FLAGS) -MT $*.oo > $@

build/%.oo: %.oc build/%.od
	$(OBLIVCC) -c $(CFLAGS) $*.oc $(INCLUDE_FLAGS) -o $@

# Build test executables
build/test/%: build/test/%.o build/liboram.a
	$(OBLIVCC) -o $@ $(filter %.o %.oo,$^) -loram -Lbuild -lm

build/bench/%: build/bench/%.o build/liboram.a
	$(OBLIVCC) -o $@ $(filter %.o %.oo,$^) -loram -Lbuild -lm

# Create link-time dependencies for binaries
ifneq (clean,$(MAKECMDGOALS))
  -include $(TEST_BINS:=.exec_d)
endif

dfsobj=$(sort $(foreach x,$1,$(call dfsobj_aux,$x,)))
in_list=$(filter $1,$2)

dfsobj_aux = \
  $(if $(call in_list,$1,$2),,\
  $(sort $1 $(foreach nxt,$(call objdeps,$1),$(call dfsobj_aux,$(nxt),$1 $2))))


# Depend on .o instead of .c, so that .d gets built with it
# Test binaries need main() in .c, not .oc
build/%.exec_d: build/%.o
	@echo Starting with build/$*.o
	echo $@ build/$*: $(call dfsobj,$^) > $@
