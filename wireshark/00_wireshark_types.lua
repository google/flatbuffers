-- internal primitives
fb_uoffset_t = ProtoField.uint32("uoffset_t", "unsigned offset", base.DEC, nil, nil, "Generic Offset Type")
fb_soffset_t = ProtoField.int32("soffset_t", "signed offset", base.DEC, nil, nil, "Generic Signed Offset Type")
fb_voffset_t = ProtoField.uint16("voffset_t", "vtable offset", base.DEC, nil, nil, "Generic VTable Offset Type")

-- primitives
-- bool
fb_bool = ProtoField.bool("bool", "bool", base.NONE, nil, nil, "Generic Boolean Type")

-- unsigned
fb_uint8 = ProtoField.uint8("uint8", "uint8", base.DEC, nil, nil, "Generic 8-bit Unsigned Integer")
fb_uint16 = ProtoField.uint16("uint16", "uint16", base.DEC, nil, nil, "Generic 16-bit Unsigned Integer")
fb_uint32 = ProtoField.uint32("uint32", "uint32", base.DEC, nil, nil, "Generic 32-bit Unsigned Integer")
fb_uint64 = ProtoField.uint64("uint64", "uint64", base.DEC, nil, nil, "Generic 64-bit Unsigned Integer")

-- signed
fb_int8 = ProtoField.int8("int8", "int8", base.DEC, nil, nil, "Generic 8-bit Signed Integer")
fb_int16 = ProtoField.int16("int16", "int16", base.DEC, nil, nil, "Generic 16-bit Signed Integer")
fb_int32 = ProtoField.int32("int32", "int32", base.DEC, nil, nil, "Generic 32-bit Signed Integer")
fb_int64 = ProtoField.int64("int64", "int64", base.DEC, nil, nil, "Generic 64-bit Signed Integer")

-- floating point
fb_float32 = ProtoField.float("float32", "float32", base.DEC, "Generic 32-bit Float")
fb_float64 = ProtoField.double("float64", "float64", base.DEC, "Generic 64-bit Float")
fb_union = ProtoField.bytes("union", "union", base.NONE, "Generic Union Type ([uint8_t, u_offset_t])")

fb_struct = ProtoField.bytes("struct", "struct", base.NONE, "Generic Struct Type")
fb_array = ProtoField.bytes("array", "array", base.NONE, "Generic Array Type")
fb_vector = ProtoField.bytes("vector", "vector", base.NONE, "Generic Vector Type")
fb_table = ProtoField.bytes("table", "table", base.NONE, "Generic Table Type")
fb_vtable = ProtoField.bytes("vtable", "vtable", base.NONE, "Generic VTable Type")
fb_string = ProtoField.string("string", "string", base.UNICODE, "Generic String Type")
fb_file_ident = ProtoField.string("file_identifier", "file identifier", base.UNICODE, "File Identifier")

fb_basic_fields = {
    fb_uoffset_t, fb_soffset_t, fb_voffset_t,
    fb_bool,
    fb_uint8, fb_uint16, fb_uint32, fb_uint64,
    fb_int8, fb_int16, fb_int32, fb_int64,
    fb_float32, fb_float64, fb_string, fb_file_ident,
    fb_union, fb_struct, fb_array, fb_vector, fb_table, fb_vtable
}
