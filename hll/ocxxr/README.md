ocxxr: C++ bindings for OCR
===========================

This is a header-only library providing a C++ interface
for the Open Community Runtime API (OCR v1.1.0).

Requirements
------------

### C++ Compiler

Since ocxxr uses several C++14 features,
you will need a fairly recent compiler to build ocxxr applications.
Clang 3.4+ or GCC 5+ should work correctly.

However, we also include some support for backward-compatibility with C++11.
All of the external API should function correctly in this mode,
but some internal sanity checks may be missing.
We have tested the C++11 support with GCC 4.8.

### OCR

This version of ocxxr is built against the OCR v1.1.0 API.

Setup
-----

### Installation

Since this is a header-only library, installation is simple!
Just copy the code somewhere, and add the `include` directory
to your include path in your makefile.

### Environment Variables

If you want to use the provided makefiles, then you should set `XSTG_ROOT`
to point to the directory containing both your OCR and OCR-Apps trees.
You might also want to set `OCXXR_LIB_ROOT` to point to
the directory containing this README (the ocxxr project root)
for easier use in your makefiles.

Testing
-------

    $ cd tests
    $ ./test-all.sh
    ...
    > All 24 tests passed!


Support
-------

For more documentation, please visit the ocxxr wiki:

https://github.com/DaoWen/ocxxr/wiki

To report bugs or ask for other support, please open an issue on GitHub:

https://github.com/DaoWen/ocxxr/issues
