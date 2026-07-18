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
import FlexBuffers
import Foundation

let benchmarks = {
  let data = {
    var array = [8888.88, 8888.88]
    var data = Data()
    array.withUnsafeBytes { ptr in
      data.append(contentsOf: ptr)
    }
    return data
  }()
  let ints: [Int32] = Array(repeating: 42, count: 100)
  let str10 = (0...9).map { _ -> String in "x" }.joined()
  let str100 = (0...99).map { _ -> String in "x" }.joined()

  // A representative map: 50 keyed scalars, a keyed string and a keyed vector
  // of 100 scalars. Used for the realistic decode benchmarks.
  let mapBuffer: ByteBuffer = {
    var fbx = FlexBuffersWriter(initialSize: 1 << 16)
    fbx.map {
      for i in 0..<50 { $0.add(int: i, key: "i\(i)") }
      $0.add(string: "hello world", key: "s")
      $0.vector(key: "v") { v in
        for x in 0..<100 { v.add(int: x) }
      }
    }
    fbx.finish()
    return fbx.sizedByteBuffer
  }()

  let metrics: [BenchmarkMetric] = [
    .cpuTotal,
    .wallClock,
    .mallocCountTotal,
    .releaseCount,
    .peakMemoryResident,
  ]
  let maxIterations = 1_000_000
  let maxDuration: Duration = .seconds(3)
  let megaConfiguration: Benchmark.Configuration = .init(
    metrics: metrics,
    warmupIterations: 1,
    scalingFactor: .mega,
    maxDuration: maxDuration,
    maxIterations: maxIterations)

  Benchmark.defaultConfiguration = megaConfiguration

  // Decode (read path)

  // Raw scalar read: isolates `read<T: BitwiseCopyable>` and the `let` blob.
  Benchmark("Reading Doubles") { benchmark in
    let byteBuffer = ByteBuffer(data: data)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      blackHole(byteBuffer.read(def: Double.self, position: 0))
    }
    benchmark.stopMeasurement()
  }

  // Realistic decode: resolve root map and read a keyed scalar.
  Benchmark("Decode Map Scalar") { benchmark in
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let map = try! getRoot(buffer: mapBuffer)!.map!
      blackHole(map["i25"]?.int)
    }
    benchmark.stopMeasurement()
  }

  // Realistic decode: resolve root map and read a keyed string.
  Benchmark("Decode Map String") { benchmark in
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let map = try! getRoot(buffer: mapBuffer)!.map!
      blackHole(map["s"]?.string())
    }
    benchmark.stopMeasurement()
  }

  // Realistic decode: resolve a nested vector and sum its scalars.
  Benchmark("Decode Vector") { benchmark in
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      let map = try! getRoot(buffer: mapBuffer)!.map!
      let vector = map["v"]!.vector!
      var sum: Int64 = 0
      for i in 0..<vector.count {
        sum &+= vector[i]?.int ?? 0
      }
      blackHole(sum)
    }
    benchmark.stopMeasurement()
  }

  // Encode (write path)
  // Writers are reused with `reset(keepingCapacity:)` so per-iteration
  // allocation does not dominate and mask the write-path cost (which is what
  // the `@exclusivity(unchecked)` storage pointer affects).

  Benchmark("Strings 10") { benchmark in
    var fbx = FlexBuffersWriter(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      fbx.add(string: str10)
      fbx.finish()
      blackHole(fbx.sizedByteBuffer)
      fbx.reset(keepingCapacity: true)
    }
    benchmark.stopMeasurement()
  }

  Benchmark("Strings 100") { benchmark in
    var fbx = FlexBuffersWriter(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      fbx.add(string: str100)
      fbx.finish()
      blackHole(fbx.sizedByteBuffer)
      fbx.reset(keepingCapacity: true)
    }
    benchmark.stopMeasurement()
  }

  Benchmark("Encoding Vector Of Ints") { benchmark in
    var fbx = FlexBuffersWriter(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      fbx.vector {
        $0.create(vector: ints)
      }
      fbx.finish()
      blackHole(fbx.sizedByteBuffer)
      fbx.reset(keepingCapacity: true)
    }
    benchmark.stopMeasurement()
  }

  Benchmark("Encoding Map") { benchmark in
    var fbx = FlexBuffersWriter(initialSize: 1 << 20)
    benchmark.startMeasurement()
    for _ in benchmark.scaledIterations {
      fbx.map {
        for i in 0..<50 { $0.add(int: i, key: "i\(i)") }
        $0.add(string: "hello world", key: "s")
        $0.vector(key: "v") { v in
          for x in 0..<100 { v.add(int: x) }
        }
      }
      fbx.finish()
      blackHole(fbx.sizedByteBuffer)
      fbx.reset(keepingCapacity: true)
    }
    benchmark.stopMeasurement()
  }
}
