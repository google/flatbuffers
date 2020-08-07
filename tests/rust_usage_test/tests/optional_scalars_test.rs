#[allow(dead_code, unused_imports)]
#[path = "../../optional_scalars_generated.rs"]
mod optional_scalars_generated;
use crate::optional_scalars_generated::optional_scalars::*;

// There are 3 variants of scalars in tables - those specified with default=42,
// optional scalars, and those with nothing specified (implicitly default=0).
// This tests that you can read what you write.
macro_rules! make_test {
    (
        $test_name: ident,
        $just: ident, $default: ident, $maybe: ident,
        $five: expr, $zero: expr, $fortytwo: expr
    ) => {
        #[test]
        fn $test_name() {
            let mut builder = flatbuffers::FlatBufferBuilder::new();
            // Test five makes sense when specified.
            let ss = ScalarStuff::create(&mut builder, &ScalarStuffArgs {
                $just: $five,
                $default: $five,
                $maybe: Some($five),
                ..Default::default()
            });
            builder.finish(ss, None);

            let s = flatbuffers::get_root::<ScalarStuff>(builder.finished_data());
            assert_eq!(s.$just(), $five);
            assert_eq!(s.$default(), $five);
            assert_eq!(s.$maybe(), Some($five));

            // Test defaults are used when not specified.
            let s = flatbuffers::get_root::<ScalarStuff>(&[0; 8]);
            assert_eq!(s.$just(), $zero);
            assert_eq!(s.$default(), $fortytwo);
            assert_eq!(s.$maybe(), None);
        }

    };
}

make_test!(optional_i8, just_i8, default_i8, maybe_i8, 5, 0, 42);
make_test!(optional_u8, just_u8, default_u8, maybe_u8, 5, 0, 42);
make_test!(optional_i16, just_i16, default_i16, maybe_i16, 5, 0, 42);
make_test!(optional_u16, just_u16, default_u16, maybe_u16, 5, 0, 42);
make_test!(optional_i32, just_i32, default_i32, maybe_i32, 5, 0, 42);
make_test!(optional_u32, just_u32, default_u32, maybe_u32, 5, 0, 42);
make_test!(optional_i64, just_i64, default_i64, maybe_i64, 5, 0, 42);
make_test!(optional_u64, just_u64, default_u64, maybe_u64, 5, 0, 42);
make_test!(optional_f32, just_f32, default_f32, maybe_f32, 5.0, 0.0, 42.0);
make_test!(optional_f64, just_f64, default_f64, maybe_f64, 5.0, 0.0, 42.0);
make_test!(optional_bool, just_bool, default_bool, maybe_bool, true, false, true);
