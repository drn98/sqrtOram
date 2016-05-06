ORAM library for Obliv-C
========================

This is a library extension for [Obliv-C](http://oblivc.org/), a language for
collaborative cryptographic computation on private data. Typically Obliv-C does
not allow array access such that the element location depends on private data.
This library provides data structures for overcoming this limitation, at least
for honest-but-curious protocols.

**Status:** The library itself is ready for use, but the test/benchmarking code is still
being posted as we clean it up. In the meantime, we are happy to help out if you
contact us at sza4uq@virginia.edu.


Installation
------------

First, install Obliv-C if you have not done so already. Follow the instructions
from this [repository](https://github.com/samee/obliv-c). After that, perform
the following actions:

  1. Add this to your `.bashrc`, since it is used by the Makefile. This should
allow you to invoke the compiler as `$OBLIV_CPATH/bin/oblivcc`.

    ```bash
    export OBLIVC_PATH=/path/to/oblivc
    ```

  2. Clone the repository: `git clone https://github.com/samee/sqrtOram.git`
  3. Perform `make`

This will build a `build/liboram.a` file in the repository. Now you can use it
in any project as follows:

    OBLIVCC=$OBLIVC_PATH/bin/oblivcc
    LIBORAM=path/to/liboram/repo
    $OBLIVCC yourproject.oc -I $LIBORAM/oram -L $LIBORAM/build -loram

As always, questions always welcome at sza4uq@virginia.edu
