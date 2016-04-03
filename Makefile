OBLIVCC = $(OBLIVC_PATH)/bin/oblivcc
OBLIVCH = $(OBLIVC_PATH)/src/ext/oblivc
OBLIVCA = $(OBLIVC_PATH)/_build/libobliv.a

# Generate build directories
BUILD_SUBDIRS=$(patsubst %,build/%,src test bench)
$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

.PHONY: builddirs
builddirs: $(BUILD_SUBDIRS)

.PHONY: clean
clean:
	rm -rf build

# Include generated dependencies
-include $(patsubst %.oo,%.od,$(OBJS:.o=.d))

build/%.o: %.c builddirs
	gcc -c $(CFLAGS) $*.c -o $@ -I $(OBLIVCH)
	cpp -MM $(CFLAGS) $*.c -I $(OBLIVCH) -MT $@ > build/$*.d

build/%.oo: %.oc builddirs
	$(OBLIVCC) -c $(CFLAGS) $*.oc -o $@
	cpp -MM $(CFLAGS) $*.oc -MT $@ > build/$*.od

