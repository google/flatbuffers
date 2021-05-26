/*
 * Copyright 2021 Google Inc. All rights reserved.
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

import CoreFoundation
import FlatBuffers

struct Benchmark {
  var name: String
  var value: Double

  var description: String { "\(String(format: "|\t%@\t\t|\t\t%fs\t|", name, value))"}
}

func run(name: String, runs: Int, action: () -> Void) -> Benchmark {
  action()
  let start = CFAbsoluteTimeGetCurrent()
  for _ in 0..<runs {
    action()
  }
  let ends = CFAbsoluteTimeGetCurrent()
  let value = Double(ends - start) / Double(runs)
  print("done \(name): in \(value)")
  return Benchmark(name: name, value: value)
}


func createDocument(Benchmarks: [Benchmark]) -> String {
  let separator = "-------------------------------------"
  var document = "\(separator)\n"
  document += "\(String(format: "|\t%@\t\t  |\t\t%@\t\t|", "Name", "Scores"))\n"
  document += "\(separator)\n"
  for i in Benchmarks {
    document += "\(i.description) \n"
    document += "\(separator)\n"
  }
  return document
}

@inlinable
func create10Strings() {
  var fb = FlatBufferBuilder(initialSize: 1<<20)
  for _ in 0..<10_000 {
    _ = fb.create(string: "foobarbaz")
  }
}

@inlinable
func create100Strings(str: String) {
  var fb = FlatBufferBuilder(initialSize: 1<<20)
  for _ in 0..<10_000 {
    _ = fb.create(string: str)
  }
}

@inlinable
func benchmarkFiveHundredAdds() {
  var fb = FlatBufferBuilder(initialSize: 1024 * 1024 * 32)
  for _ in 0..<500_000 {
    let off = fb.create(string: "T")
    let s = fb.startTable(with: 4)
    fb.add(element: 3.2, def: 0, at: 2)
    fb.add(element: 4.2, def: 0, at: 4)
    fb.add(element: 5.2, def: 0, at: 6)
    fb.add(offset: off, at: 8)
    _ = fb.endTable(at: s)
  }
}

@inlinable
func benchmarkThreeMillionStructs() {
  let structCount = 3_000_000

  let rawSize = ((16 * 5) * structCount) / 1024

  var fb = FlatBufferBuilder(initialSize: Int32(rawSize * 1600))

  var offsets: [Offset] = []
  for _ in 0..<structCount {
    fb.startVector(
      5 * MemoryLayout<AA>.size,
      elementSize: MemoryLayout<AA>.alignment)
    for _ in 0..<5 {
      _ = fb.create(struct: AA(a: 2.4, b: 2.4))
    }
    let vector = fb.endVector(len: 5)
    let start = fb.startTable(with: 1)
    fb.add(offset: vector, at: 4)
    offsets.append(Offset(offset: fb.endTable(at: start)))
  }
  let vector = fb.createVector(ofOffsets: offsets)
  let start = fb.startTable(with: 1)
  fb.add(offset: vector, at: 4)
  let root = Offset(offset: fb.endTable(at: start))
  fb.finish(offset: root)
}

@usableFromInline
struct AA: NativeStruct {
  public init(a: Double, b: Double) {
    self.a = a
    self.b = b
  }
  var a: Double
  var b: Double

}

func benchmark(numberOfRuns runs: Int) {
  var benchmarks: [Benchmark] = []
  let str = (0...99).map { _ -> String in "x" }.joined()
  benchmarks.append(run(
    name: "500_000",
    runs: runs,
    action: benchmarkFiveHundredAdds))
  benchmarks.append(run(name: "10 str", runs: runs, action: create10Strings))
  let hundredStr = run(name: "100 str", runs: runs) {
    create100Strings(str: str)
  }
  benchmarks.append(run(
    name: "3M strc",
    runs: 1,
    action: benchmarkThreeMillionStructs))
  benchmarks.append(hundredStr)
  print(createDocument(Benchmarks: benchmarks))
}

benchmark(numberOfRuns: 20)
