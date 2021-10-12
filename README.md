# Go plugins via WAMR

This project is for illustrates building plugins for a C++ executable in Go.

## Instructions

Sorry, no makefile type thing yet.  You'll need to install TinyGo and WAMR.

Then

```
$ tinygo build -o go_sample.wasm
$ c++ -I ../wasm-micro-runtime/core/iwasm/include/ runtime/runtime.cpp -L ../wasm-micro-runtime/product-mini/platforms/darwin/build -lvmlib
$ ./a.out
```
