// In this example, a build.rs file generates the code and then copies it into generated/
extern crate flatbuffers;
#[allow(unused_imports, dead_code)]
mod generated;
use generated::my_game::sample::{Monster, MonsterArgs};

fn main() {
    let mut fbb = flatbuffers::FlatBufferBuilder::new();
    let name = Some(fbb.create_string("bob"));
    let m = Monster::create(&mut fbb, &MonsterArgs { hp: 1, mana: 2, name, ..Default::default() });
    fbb.finish(m, None);
    let mon = flatbuffers::root::<Monster>(fbb.finished_data()).unwrap();
    assert_eq!(mon.hp(), 1);
    assert_eq!(mon.mana(), 2);
    assert_eq!(mon.name().unwrap(), "bob");
}

#[test]
fn test_main() {
    main()
}

#[cfg(test)]
mod float_key_tests {
    include!(concat!(env!("OUT_DIR"), "/float_keys_generated.rs"));

    use self::float_keys::{DoubleKey, DoubleKeyArgs, FloatKey};
    use std::cmp::Ordering::{Equal, Greater, Less};

    #[test]
    fn float_struct_keys() {
        // Signed zeros share a rank. All NaNs sort after the numeric keys.
        let values = [
            (f32::NEG_INFINITY, 0), (-1.0, 1), (-0.0, 2), (0.0, 2),
            (f32::INFINITY, 3), (f32::NAN, 4), (-f32::NAN, 4),
        ];
        for &(left, left_rank) in &values {
            for &(right, right_rank) in &values {
                let expected = left_rank.cmp(&right_rank);
                let key = FloatKey::new(left);
                assert_eq!(key.key_compare_with_value(right), expected);
                assert_eq!(key.key_compare_less_than(&FloatKey::new(right)), expected == Less);
            }
        }
    }

    #[test]
    fn double_table_key() {
        let mut builder = flatbuffers::FlatBufferBuilder::new();
        let table = DoubleKey::create(&mut builder, &DoubleKeyArgs { value: f64::NAN });
        builder.finish(table, None);
        let key = flatbuffers::root::<DoubleKey>(builder.finished_data()).unwrap();
        assert_eq!(key.key_compare_with_value(f64::INFINITY), Greater);
        assert_eq!(key.key_compare_with_value(f64::NAN), Equal);
        assert!(!key.key_compare_less_than(&key));
    }
}
