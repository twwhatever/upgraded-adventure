use std::str;
use std::slice;
use std::ffi::CString;
use std::ffi::CStr;
use std::os::raw::c_char;
use core::alloc::Layout;
extern crate alloc;
use wasm_bindgen::prelude::wasm_bindgen;

#[link(wasm_import_module = "go_sample")]
extern "C" {
    fn hello(pbuf: *const u8, size: u32) -> *mut u8;
    fn echo(pbuf: *const u8, size: u32) -> *mut u8;
    fn fixed(pbuf: *const u8, size: u32) -> *mut u8;
    fn caller(pbuf: *const u8, size: u32, out_buf: *const u8, out_size: u32) -> u32;
}

#[link(wasm_import_module = "env")]
extern "C" {
    fn wasm_malloc(size: u32) -> u32;
}

#[wasm_bindgen]
pub extern "C" fn hi(pbuf: *const u8, size: u32) -> *mut c_char {
    println!("hi from rust: {:p} {}", pbuf, size);
    unsafe {
        //let pmsg =  fixed(pbuf, 1);
        let pmsg = wasm_malloc(1000);
        let written = caller(pbuf, size, pmsg as *mut u8, 1000);
        println!("hi from rust: message = {}, *message = {}", pmsg, *(pmsg as *const i8));
        let input_str = CStr::from_ptr((pmsg as *const i8));
        println!("hi from rust: input_str = {:?}", input_str);
        let output_str = format!("Hi, {} from Rust!", input_str.to_string_lossy());
        return CString::new(output_str).unwrap().into_raw();
        //return CString::new("hmm").unwrap().into_raw();

    }
}

