# flexbuffers

[Flexbuffers](https://google.github.io/flatbuffers/flexbuffers.html) is a
schema-less binary format developed at Google. FlexBuffers can be accessed
without parsing, copying, or allocation. This is a huge win for efficiency,
memory friendly-ness, and allows for unique use cases such as mmap-ing large
amounts of free-form data.

FlexBuffers' design and implementation allows for a very compact encoding,
with automatic sizing of containers to their smallest possible representation
(8/16/32/64 bits). Many values and offsets can be encoded in just 8 bits.

Flexbuffers is the schema-less cousin of
[Flatbuffers](https://google.github.io/flatbuffers/).
