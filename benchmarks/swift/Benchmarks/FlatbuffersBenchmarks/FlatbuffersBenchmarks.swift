/*
 * Copyright 2024 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import Benchmark
import FlatBuffers
import Foundation

@usableFromInline
struct AA: NativeStruct {
  public init(a: Double, b: Double) {
    self.a = a
    self.b = b
  }
  var a: Double
  var b: Double
}

let benchmarks = {
  let oneGB: Int32 = 1_024_000_000
  let data = {
    var array = [8888.88, 8888.88]
    var data = Data()
    array.withUnsafeBytes { ptr in
      data.append(contentsOf: ptr)
    }
    return data
  }()
  let ints: [Int] = Array(repeating: 42, count: 100)
  let bytes: [UInt8] = Array(repeating: 42, count: 100)
  let str10 = (0...9).map { _ -> String in "x" }.joined()
  let str100 = (0...99).map { _ -> String in "x" }.joined()
  let array: [AA] = [
    AA(a: 2.4, b: 2.4),
    AA(a: 2.4, b: 2.4),
    AA(a: 2.4, b: 2.4),
    AA(a: 2.4, b: 2.4),
    AA(a: 2.4, b: 2.4),
  ]

  let metrics: [BenchmarkMetric] = [
    .cpuTotal,
    .wallClock,
    .mallocCountTotal,
    .releaseCount,
    .peakMemoryResident,
  ]
  let maxIterations = 1_000_000
  let maxDuration: Duration = .seconds(3)
  let singleConfiguration: Benchmark.Configuration = .init(
    metrics: metrics,
    warmupIterations: 1,
    scalingFactor: .one,
    maxDuration: maxDuration,
    maxIterations: maxIterations)
  let kiloConfiguration: Benchmark.Configuration = .init(
    metrics: metrics,
    warmupIterations: 1,
    scalingFactor: .kilo,
    maxDuration: maxDuration,
    maxIterations: maxIterations)
  let megaConfiguration: Benchmark.Configuration = .init(
    metrics: metrics,
    warmupIterations: 1,
    scalingFactor: .mega,
    maxDuration: maxDuration,
    maxIterations: maxIterations)

  Benchmark.defaultConfiguration = megaConfiguration

  Benchmark("Allocating 1GB", configuration: singleConfiguration) { benchmark in
    for _ in benchmark.scaledIterations {
      blackHole(FlatBufferBuilder(initialSize: oneGB))
    }
  }

  Benchmark(
    "Allocating ByteBuffer 1GB",
    configuration: singleConfiguration
  ) { benchmark in
    let memory = UnsafeMutableRawPointer.allocate(
      byteCount: 1_024_000_000,
      alignment: 1)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(ByteBuffer(assumingMemoryBound: memory, capacity: Int(oneGB)))
    }
  }

  Benchmark("Clearing 1GB", configuration: singleConfiguration) { benchmark in
    var fb = FlatBufferBuilder(initialSize: oneGB)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.clear())
    }
  }

  Benchmark("Strings 10") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.create(string: str10))
    }
  }

  Benchmark("Strings 100") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.create(string: str100))
    }
  }

  Benchmark("Vector 1 Bytes") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.createVector(bytes: bytes))
    }
  }

  Benchmark("Vector 1 Ints") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.createVector(ints))
    }
  }

  Benchmark("Vector 100 Ints") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for i in benchmark.scaledIterations {
      blackHole(fb.createVector(ints))
    }
  }

  Benchmark("Vector 100 Bytes") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for i in benchmark.scaledIterations {
      blackHole(fb.createVector(bytes))
    }
  }

  Benchmark("Vector 100 ContiguousBytes") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for i in benchmark.scaledIterations {
      blackHole(fb.createVector(bytes: bytes))
    }
  }

  Benchmark(
    "FlatBufferBuilder Add",
    configuration: kiloConfiguration
  ) { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1024 * 1024 * 32)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let off = fb.create(string: "T")
      let s = fb.startTable(with: 4)
      fb.add(element: 3.2, def: 0, at: 2)
      fb.add(element: 4.2, def: 0, at: 4)
      fb.add(element: 5.2, def: 0, at: 6)
      fb.add(offset: off, at: 8)
      blackHole(fb.endTable(at: s))
    }
  }

  Benchmark(
    "FlatBufferBuilder Start table",
    configuration: kiloConfiguration
  ) { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1024 * 1024 * 32)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let s = fb.startTable(with: 4)
      blackHole(fb.endTable(at: s))
    }
  }

  Benchmark("Struct") { benchmark in
    var fb = FlatBufferBuilder(initialSize: 1024 * 1024 * 32)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(fb.create(struct: array.first!))
    }
  }

  Benchmark("Structs") { benchmark in
    let rawSize = ((16 * 5) * benchmark.scaledIterations.count) / 1024
    var fb = FlatBufferBuilder(initialSize: Int32(rawSize * 1600))
    var offsets: [Offset] = []

    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let vector = fb.createVector(
        ofStructs: array)
      let start = fb.startTable(with: 1)
      fb.add(offset: vector, at: 4)
      offsets.append(Offset(offset: fb.endTable(at: start)))
    }

    let vector = fb.createVector(ofOffsets: offsets)
    let start = fb.startTable(with: 1)
    fb.add(offset: vector, at: 4)
    let root = Offset(offset: fb.endTable(at: start))
    blackHole(fb.finish(offset: root))
  }

  Benchmark("Vector of Offsets") { benchmark in
    let rawSize = ((16 * 5) * benchmark.scaledIterations.count) / 1024
    var fb = FlatBufferBuilder(initialSize: Int32(rawSize * 1600))
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let offsets = [
        fb.create(string: "T"),
        fb.create(string: "2"),
        fb.create(string: "3"),
      ]
      let off = fb.createVector(ofOffsets: [
        fb.createVector(ofOffsets: offsets),
        fb.createVector(ofOffsets: offsets),
      ])
      let s = fb.startTable(with: 2)
      fb.add(offset: off, at: 2)
      blackHole(fb.endTable(at: s))
    }
  }

  Benchmark("Reading Doubles") { benchmark in
    let byteBuffer = ByteBuffer(data: data)
    for _ in benchmark.scaledIterations {
      blackHole(byteBuffer.read(def: Double.self, position: 0))
    }
  }
}
