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
    let fb = FlatBufferBuilder(initialSize: 1<<20)
    for _ in 0..<10_000 {
        _ = fb.create(string: "foobarbaz")
    }
}

@inlinable func create100Strings(str: String) {
    let fb = FlatBufferBuilder(initialSize: 1<<20)
    for _ in 0..<10_000 {
        _ = fb.create(string: str)
    }
}

@inlinable func benchmarkFiveHundredAdds() {
    let fb = FlatBufferBuilder(initialSize: 1024 * 1024 * 32)
    for _ in 0..<500_000 {
        let off = fb.create(string: "T")
        let s = fb.startTable(with: 4)
        fb.add(element: 3.2, def: 0, at: 0)
        fb.add(element: 4.2, def: 0, at: 1)
        fb.add(element: 5.2, def: 0, at: 2)
        fb.add(offset: off, at: 3)
        _ = fb.endTable(at: s)
    }
}

func benchmark(numberOfRuns runs: Int) {
    var benchmarks: [Benchmark] = []
    let str = (0...99).map { _ -> String in return "x" }.joined()
    benchmarks.append(run(name: "500_000", runs: runs, action: benchmarkFiveHundredAdds))
    benchmarks.append(run(name: "10 str", runs: runs, action: create10Strings))
    let hundredStr = run(name: "100 str", runs: runs) {
        create100Strings(str: str)
    }
    benchmarks.append(hundredStr)
    print(createDocument(Benchmarks: benchmarks))
}

benchmark(numberOfRuns: 20)
