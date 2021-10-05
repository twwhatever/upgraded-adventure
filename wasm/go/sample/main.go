package main

import (
	"reflect"
	"unsafe"
)

//export hello
func hello(buf *uint8, size uint) *uint8 {
	// b := unsafe.Slice(buf, size)  // Note, requires Go 1.17 (tinygo 0.20)
	sh := &reflect.SliceHeader{
		Data: uintptr(unsafe.Pointer(buf)),
		Len:  uintptr(size),
		Cap:  uintptr(size),
	}
	b := *(*[]byte)(unsafe.Pointer(sh))
	s := "Hello, " + string(b) + " from Go!"
	s += string(rune(0)) // Note: explicit null terminator.
	rv := []byte(s)
	return &rv[0]
}
