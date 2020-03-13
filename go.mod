module github.com/google/flatbuffers

go 1.14

// Adding prefixes that point to the real code in master (hosted on github)
// could make it easy to accidentally use the wrong code in testing. So, for
// tests we use an obviously fake module prefix that just points to local root.
replace fake.flatbuffers.moduleroot => ./

require fake.flatbuffers.moduleroot v0.0.0 // indirect
