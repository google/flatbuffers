use rustc_version::{Channel, version_meta};

fn main() {
    let version_meta = version_meta().unwrap();

    // To use nightly features we declare this and then we can use
    // #[cfg(nightly)]
    // for nightly only features
    println!("cargo:rustc-check-cfg=cfg(nightly)");
    if version_meta.channel == Channel::Nightly {
        println!("cargo:rustc-cfg=nightly")
    }
}
