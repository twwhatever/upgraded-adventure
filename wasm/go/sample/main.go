package main

import (
	"fmt"
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

//export echo
func echo(buf *uint8, size uint) *uint8 {
	s := fmt.Sprintf("in go: %p %d", buf, size)
	s += string(rune(0))
	rv := []byte(s)
	return &rv[0]
}

//export fixed
func fixed(buf *uint8, size uint) uintptr {
	fmt.Printf("fixed from go: %p %d\n", buf, size)
	s := "something in go"
	s += string(rune(0))
	s_slice := []byte(s)
	return uintptr(unsafe.Pointer(&s_slice[0]))
}

//export caller
func caller(buf *uint8, size uint, out_buf *uint8, out_size uint) uint {
	fmt.Printf("caller from go: %p %d %p %d\n", buf, size, out_buf, out_size)
	s := "something in go"
	s_slice := []byte(s)
	s_size := uint(len(s_slice))
	if s_size > out_size {
		return s_size
	}
	pout := uintptr(unsafe.Pointer(out_buf))
	fmt.Printf("caller from go: setting %p", pout)
	for i := uint(0); i < s_size; i++ {
		*(*uint8)(unsafe.Pointer(pout)) = s_slice[i]
		pout++
		fmt.Printf("caller from go: %d\n", i)
	}
	fmt.Printf("caller from go: %p %c\n", out_buf, int8(*out_buf))
	return s_size
}
