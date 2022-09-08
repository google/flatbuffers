#![no_std]
#![no_main]
#![feature(alloc_error_handler)]
#![feature(core_c_str)]
#![feature(core_ffi_c)]

/// Minimal no_std binary used to test that flatbuffers compiles in a no_std
/// enviroment. Based on: https://blog.dbrgn.ch/2019/12/24/testing-for-no-std-compatibility/
use core::panic::PanicInfo;
use linked_list_allocator::LockedHeap;

#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();

// Include flatbuffers purely to check that it compiles in in a no_std binary
#[allow(unused_imports)]
use flatbuffers;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[alloc_error_handler]
fn out_of_memory(layout: ::core::alloc::Layout) -> ! {
    panic!("Error allocating memory: {:#?}", layout);
}

#[no_mangle]
pub extern "C" fn _start() -> ! {
    loop {}
}
