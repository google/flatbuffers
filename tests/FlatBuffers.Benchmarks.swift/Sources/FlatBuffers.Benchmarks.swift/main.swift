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
    document += "\(String(format: "|\t%@\t\t|\t\t%@\t\t|", "Name", "Scores"))\n"
    document += "\(separator)\n"
    for i in Benchmarks {
        document += "\(i.description) \n"
        document += "\(separator)\n"
    }
    return document
}

@inlinable func create10Strings() {
    var fb = FlatBufferBuilder(initialSize: 1<<20)
    for _ in 0..<10_000 {
        _ = fb.create(string: "foobarbaz")
    }
}

@inlinable func create100Strings(str: String) {
    var fb = FlatBufferBuilder(initialSize: 1<<20)
    for _ in 0..<10_000 {
        _ = fb.create(string: str)
    }
}

@inlinable func benchmarkFiveHundredAdds() {
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

@inlinable func benchmarkThreeMillionStructs() {
    let structCount = 3_000_000
    
    let rawSize = ((16 * 5) * structCount) / 1024
    
    var fb = FlatBufferBuilder(initialSize: Int32(rawSize * 1600))
    
    var offsets: [Offset<UOffset>] = []
    for _ in 0..<structCount {
        fb.startVectorOfStructs(count: 5, size: 16, alignment: 8)
        for _ in 0..<5 {
            fb.createStructOf(size: 16, alignment: 8)
            fb.reverseAdd(v: 2.4, postion: 0)
            fb.reverseAdd(v: 2.4, postion: 8)
            fb.endStruct()
        }
        let vector = fb.endVectorOfStructs(count: 5)
        let start = fb.startTable(with: 1)
        fb.add(offset: vector, at: 4)
        offsets.append(Offset<UOffset>(offset: fb.endTable(at: start)))
    }
    let vector = fb.createVector(ofOffsets: offsets)
    let start = fb.startTable(with: 1)
    fb.add(offset: vector, at: 4)
    let root = Offset<UOffset>(offset: fb.endTable(at: start))
    fb.finish(offset: root)
}

func benchmark(numberOfRuns runs: Int) {
    var benchmarks: [Benchmark] = []
    let str = (0...99).map { _ -> String in return "x" }.joined()
    benchmarks.append(run(name: "500_000", runs: runs, action: benchmarkFiveHundredAdds))
    benchmarks.append(run(name: "10 str", runs: runs, action: create10Strings))
    let hundredStr = run(name: "100 str", runs: runs) {
        create100Strings(str: str)
    }
    benchmarks.append(run(name: "3M strc", runs: 1, action: benchmarkThreeMillionStructs))
    benchmarks.append(hundredStr)
    print(createDocument(Benchmarks: benchmarks))
}

benchmark(numberOfRuns: 20)
