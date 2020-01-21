// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

extern crate flexbuffers;
use flexbuffers::*;
use std::time;

/// Make some example data
fn make_monster(mut monster: MapBuilder) {
    monster.push("type", "great orc");
    monster.push("age", 100u8);
    monster.push("name", "Mr. Orc");
    monster.push("coins", &[1, 25, 50, 100, 250]);
    monster.push("color", &[255u8, 0, 0, 0]);
    {
        let mut weapons = monster.start_vector("weapons");
        {
            let mut hammer = weapons.start_map();
            hammer.push("name", "hammer");
            hammer.push("damage type", "crush");
            hammer.push("damage", 20);
        }
        {
            let mut axe = weapons.start_map();
            axe.push("name", "Great Axe");
            axe.push("damage type", "slash");
            axe.push("damage", 30);
        }
    }
    {
        let mut sounds = monster.start_vector("sounds");
        sounds.push("grr");
        sounds.push("rawr");
        sounds.push("muahaha");
    }
    // TODO(cneo): Directly pushing string slices has alloc.
}

fn validate_monster(r: MapReader) {
    assert_eq!(r.idx("type").as_str(), "great orc");
    assert_eq!(r.idx("age").as_u8(), 100);
    assert_eq!(r.idx("name").as_str(), "Mr. Orc");

    let coins = r.idx("coins").get_slice::<i16>().unwrap();
    assert_eq!(coins, [1, 25, 50, 100, 250]);

    let color = r.idx("color").get_slice::<u8>().unwrap();
    assert_eq!(color, [255, 0, 0, 0]);

    let weapons = r.idx("weapons").as_vector();
    assert_eq!(weapons.len(), 2);

    let hammer = weapons.idx(0).as_map();
    assert_eq!(hammer.idx("name").as_str(), "hammer");
    assert_eq!(hammer.idx("damage type").as_str(), "crush");
    assert_eq!(hammer.idx("damage").as_u64(), 20);

    let axe = weapons.idx(1).as_map();
    assert_eq!(axe.idx("name").as_str(), "Great Axe");
    assert_eq!(axe.idx("damage type").as_str(), "slash");
    assert_eq!(axe.idx("damage").as_u64(), 30);

    let sounds = r.idx("sounds").as_vector();
    for (i, &s) in ["grr", "rawr", "muahaha"].iter().enumerate() {
        assert_eq!(sounds.idx(i).as_str(), s);
    }
}

fn bench(name: &str, n: usize, mut go: impl (FnMut() -> ())) {
    let mut times = Vec::with_capacity(n);
    // go(); // warmup
    for _ in 0..10_000 {
        let start = time::Instant::now();
        go();
        let stop = time::Instant::now();
        times.push((stop - start).as_nanos());
    }
    times.sort();
    println!(
        "{} nanos:\t{:>6?}   {:>6?}   {:>6?}   {:>6?}   {:>6?}",
        name, times[100], times[2500], times[5000], times[7500], times[9900]
    );
}

// This seems to give slower results than the version in benches...
// Its probably a higher fidelity profile though.
fn main() {
    let mut b = Builder::default();
    let ser = || {
        let mut v = b.start_vector();
        for _ in 0..100 {
            make_monster(v.start_map());
        }
    };
    bench("build", 10_000, ser);

    let fxb = b.view().to_vec();

    let ver = || {
        let r = Reader::get_root(&fxb).unwrap().as_vector();

        assert_eq!(r.len(), 100);
        for i in 0..100 {
            validate_monster(r.idx(i).as_map());
        }
    };

    bench("read", 10_000, ver);
}
