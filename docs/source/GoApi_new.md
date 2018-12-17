Go API - new and improved
=========================

## Scope of the document

This document outlines proposed changes to the Go API and operation of the
`flatc` Go compiler.

## Data type changes

Enums are always explicitly typed.

The flatbuffer's `string` type is mapped to Go `string` type. Unfortunately,
Go has no "abstract" string like type and thus all of the string related
functionality provided by Go standard library and third party code expect
a concrete `string` typed values.

This implies that treating strings as byte slices by the flatbuffer library
is unhelpful, as application code will be forced to construct strings out of
those byte slices in vast majority of cases to be able to use most
string related functionality provoded by Go and application runtime environment.

## New message builder API

The proposed API features a proper builder object:

``````````````````````````````````````````````````{.go}
	sb := example.BuildStat(flatbuffers.NewBuilder(0))
	str := sb.CreateString("MyStat")
	sb.Start()
	sb.AddId(str)
	sb.AddVal(12345678)
	sb.AddCount(12345)
	stat_end := sb.End()
	sb.Finish(stat_end)
``````````````````````````````````````````````````

Compare to the current version of API:

``````````````````````````````````````````````````{.go}
	b := flatbuffers.NewBuilder(0)
	str := b.CreateString("MyStat")
	example.StatStart(b)
	example.StatAddId(b, str)
	example.StatAddVal(b, 12345678)
	example.StatAddCount(b, 12345)
	stat_end := example.StatEnd(b)
	b.Finish(stat_end)
``````````````````````````````````````````````````

The adavntages of the new API are as following:

* Less typing
* Much less IDE code completion confusion, especialy when there are multiple
  similarly named types defined (`MonsterFish`, `MonsterBear`, etc.)
* Improved type safety when building complex messages with multiple subroutines
* More in line with other language implementations

## `InitAsRoot` initializer

Message initialization function `GetRootAs<Message>` insists on allocating a
new `<Message>` table anchor on a heap. On the other hand, existing
initalization method `Init` is not suitable for root tables, causing an
unexpected breakage when used inadvertently.

Therefore, `InitAsRoot` method is now generated to allow for initialization
of existing (on stack or pooled) table anchors with root tables.

## `Create<Message>` is now generated for tables, not only structs

`Create<Message>` constructors are handy when constructing relatively simple
messages "in-line":

``````````````````````````````````````````````````{.go}
	b := flatbuffers.NewBuilder(0)
	b.Finish(CreateStat(
		b,
		b.CreateString("MyStat"),
		12345678,
		12345,
	))
``````````````````````````````````````````````````

Due to limitations of Go type system, `Create<Message>` as not as type safe as
could be desired but may still be of use.

## Object API is now supported

When supplied with `--gen-object-api` command line flag, the `flatc` compiler
will now generate a Go native data type together with marshaling and
unmarshaling code (the names "marshal" and "unmarshal", rather than "pack"
and "unpack" are more idiomatic in Go).

``````````````````````````````````````````````````{.go}
	type StatT struct {
		Id string
		Val int64
		Count uint16
	}

	type Stat struct {
		_tab flatbuffers.Table
	}

	func (rcv *StatT) Marshal(builder *flatbuffers.Builder) flatbuffers.UOffsetT {
		// omitted for brevity
	}

	func (rcv *Stat) Unmarshal(obj *StatT) *StatT {
		// omitted for brevity
	}
``````````````````````````````````````````````````

The marshaling code currently tries to keep the message size down by explicitly
skipping zero values in the source struct. This comes at the expense of the
multiple conditionals emitted into the marshaling code. It is possible to
omit the checks, ending up with larger messages on the wire in cases where
original table has many fields but only sparsely populated, scuh as a reference
`Monster` data type. In the future a command line flag may be provided to allow
for this.

Note, that current marshaling implementation may still produces somewhat larger
message size than those achievable by manual packing due to the way flatbuffer's
`struct` data is marshaled (that is, all `struct` fields are mashaled
unconditionally).

Unmarshaling code makes an effort to reuse as many pointers and slices as
possible. Thus, client code wishing to preserve an ownership of a part of
an unmarshaled table must swap a pointer to that part with `nil` to prevent
reachability.

Extant shortcomings: byte arrays must be handled in a more efficient manner.

## `--go-namespace` command line flag is treated differently by `flatc`

The existing functionality of `--go-namespace` flag appears to be copied from
elsewhere and makes little sense in Go. The new implementation treats the
value of this flag as import path prefix, aka global namespace identifier.

An import path in Go consists of 2 parts: a global namespace identifier,
possibly including a package "origin" url and a local namespace, which is
simply a file system path between the location of innermost `go.mod` file (or
location pointed at by `$GOPATH/src` in older setups). `flatc` handles the
local namespaces automatically, while global namespace can not be reliably
determined automatically in all cases and is easier to simply supply as the
command line parameter.

## Single file mode is removed

In "single file mode" all Go code produced by `flatc` is exected to end up in
a singly `.go` file.

As mentioned above, in Go namespaces strictly map to the directory hierarchy (
each namespace must reside in its own folder). Thus, while "single file mode"
may be useful in certain simple cases, it is not clear how to handle it in
general case.

Options include:

* Produce single file per namespace with proper namespace directories generated.
  Such approach appears to negate the goals of "single file mode".

* Silently erase namespace information from generated objects. Opens a door
  to subtle and hard to find runtime errors due to induced schema
  incompatibilities.

* Erase namespace information as long as there's only one namespace involved.
  If multiple namespaces are detected in user schema fail with error.

As none of those options were to my personal liking I decided not to offer
a "single file mode" for a time being.
