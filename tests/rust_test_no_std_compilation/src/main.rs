#![no_std]
#![no_main]

extern crate wee_alloc;

// Include flatbuffers purely to check that it compiles in a no_std binary
#[allow(unused_imports)]
use flatbuffers;

#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern "C" fn _start() -> ! {
    loop {}
}
