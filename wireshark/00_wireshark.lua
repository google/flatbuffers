-- can be enabled when calling the root node parse function
FB_VERBOSE = false

function Register_Proto(protocol)
    DissectorTable.get("tcp.port"):add_for_decode_as(protocol)
    DissectorTable.get("udp.port"):add_for_decode_as(protocol)
end

local function Parse_Offset(buffer, offset, tree, field_name, field_type, indirect_offset)
    -- assume voffset is the only 2 byte wide variant
    local field_size = (field_type == fb_voffset_t) and 2 or 4

    local offset_buffer = buffer(offset, field_size)
    local offset_value = offset_buffer:le_uint()

    local subtree = tree
    if FB_VERBOSE then
        subtree = tree:add(field_type, offset_buffer, offset_value)
        subtree:prepend_text(field_type.name .. ": ")
        subtree:append_text(" [0x" .. NumberToHex(buffer:offset() + offset + offset_value, field_size) .. "]")
    end

    return {
        tree = subtree,
        value = offset_value
    }
end

local function Parse_UOffset(buffer, offset, tree, field_type)
    return Parse_Offset(buffer, offset, tree, field_type.name, fb_uoffset_t, 0)
end

local function Parse_VOffset(buffer, offset, tree, field_name, indirect_offset)
    return Parse_Offset(buffer, offset, tree, field_name, fb_voffset_t, indirect_offset)
end

function Parse_Root_Offset(buffer, offset, tree)
    local root_buffer = buffer(offset, 4)
    local root_offset = root_buffer:le_uint()

    if FB_VERBOSE then
        -- we never return this subtree
        local subtree = tree:add(fb_uoffset_t, root_buffer, root_offset)
        subtree:prepend_text("root offset: ")
        subtree:append_text(" [0x" .. NumberToHex(buffer:offset() + root_offset, 4) .. "]")
    end

    return {
        tree = tree,
        value = root_offset
    }
end

function Parse_Offset_Field(buffer, offset, tree, field_type, field_parser)
    local offset_data = Parse_UOffset(buffer, offset, tree, field_type)
    return field_parser(buffer, offset + offset_data.value, offset_data.tree, field_type)
end

function Parse_Array(buffer, offset, tree, field_type, object_type, item_size, count, ParseFunction)
    local array_buffer = buffer(offset, (count * item_size))
    local array_bytes = array_buffer:raw()
    local description = field_type.name .. ": array[" .. count .. "]"

    local subtree = tree:add(fb_array, array_buffer, array_bytes, description)

    for i = 1, count do
        local item_offset = offset + ((i - 1) * item_size)
        local tree_item = ParseFunction(buffer, item_offset, subtree, object_type)
        tree_item.tree:prepend_text((i - 1) .. ": ")
    end

    return {
        tree = subtree,
        value = array_bytes
    }
end

local function Parse_VectorInfo(buffer, offset, tree, field_type, object_size)
    local vector_count_buffer = buffer(offset, 4)
    local vector_count = vector_count_buffer:le_uint()

    local vector_buffer = buffer(offset, 4 + (vector_count * object_size))
    local vector_bytes = vector_buffer:raw()
    local description = field_type.name .. ": vector[" .. vector_count .. "]"

    local subtree = tree
    if FB_VERBOSE then
        subtree = tree:add(field_type, vector_buffer, vector_bytes, description)

        local count_buffer = buffer(offset, 4)
        local count_value = count_buffer:le_uint()
        subtree:add(fb_uint32, count_buffer, count_value)
    end

    return {
        tree = subtree,
        value = vector_count,
    }
end

-- TODO ParseVector64 or take an argument is_64
function Parse_Vector(buffer, offset, tree, is_indirect, field_type, object_type, object_size, Parse_Function)
    local offset_info = Parse_UOffset(buffer, offset, tree, field_type)
    local vector_object_size = is_indirect and 4 or object_size
    local vector_info = Parse_VectorInfo(buffer, offset + offset_info.value, offset_info.tree, field_type,
        vector_object_size)

    local Parse_FunctionObject = is_indirect and function(func_buffer, func_offset, func_tree, func_field_type)
        return Parse_Offset_Field(func_buffer, func_offset, func_tree, func_field_type, Parse_Function)
    end or Parse_Function

    return Parse_Array(buffer, offset + offset_info.value + 4, vector_info.tree, field_type, object_type,
        object_size, vector_info.value, Parse_FunctionObject)
end

function Parse_String(buffer, offset, tree, field_type)
    if offset == 0 then
        local subtree = tree:add(field_type)
        subtree:append_text("null [DEFAULT VALUE]")
        return {
            tree = subtree,
            value = nil
        }
    end

    local string_length_buffer = buffer(offset, 4)
    local string_length = string_length_buffer:le_uint() + 1 -- always null terminated, but not listed in length

    local string_struct_buffer = buffer(offset, 4 + string_length)
    local string_struct_bytes = string_struct_buffer:raw()
    local description = field_type.name .. ": string[" .. string_length .. "]"

    local subtree = tree

    if FB_VERBOSE then
        subtree = tree:add(fb_struct, string_struct_buffer, string_struct_bytes, description)

        local length_buffer = buffer(offset, 4)
        local length_value = length_buffer:le_uint()
        local len_tree = subtree:add(fb_uint32, length_buffer, length_value)
        len_tree:prepend_text("length: ")
        len_tree:append_text(" (excl \\0)")
    end

    local string_buffer = buffer(offset + 4, string_length)
    local string_value = string_buffer:string()
    subtree:add(field_type, string_buffer, string_value)

    return {
        tree = subtree,
        value = string_value
    }
end

function Parse_Struct(buffer, offset, tree, field_type, struct_size, member_list)
    local struct_buffer = buffer(offset, struct_size)
    local struct_bytes = struct_buffer:raw()

    local subtree = tree:add(field_type, struct_buffer, struct_bytes, field_type.name)

    for i = 1, #member_list do
        local data_item_offset = member_list[i][1]
        -- call this members' parser
        member_list[i][3](buffer, offset + data_item_offset, subtree, member_list[i][1])
    end

    return {
        tree = subtree,
        value = struct_bytes
    }
end

-- I can't figure out how to get width out of field size
function Parse_Enum(buffer, offset, tree, field_width, field_type, default_value)
    if offset == 0 then
        local subtree = tree
        subtree = tree:add(field_type, default_value)
        subtree:append_text(" [DEFAULT VALUE]")
        return {
            tree = subtree,
            value = default_value
        }
    end

    local enum_buffer = buffer(offset, field_width)
    local enum_value = enum_buffer:le_uint()
    local subtree = tree:add(field_type, enum_buffer, enum_value)

    return {
        tree = subtree,
        value = enum_value
    }
end

function Parse_Union(buffer, offset, tree, field_type, parse_function)
    if offset == 0 then
        local subtree = tree:add(field_type)
        return {
            tree = subtree,
            value = nil
        }
    end

    local union_data = Parse_Offset_Field(buffer, offset, tree, field_type, parse_function)

    union_data.tree:prepend_text(field_type.name .. ": ")

    return union_data
end

local function Parse_VTable(buffer)
    local vtable_size_buffer = buffer(0, 2)
    local vtable_size = vtable_size_buffer:le_uint()

    local table_size_buffer = buffer(2, 2)
    local table_size = table_size_buffer:le_uint()

    local entries = {}

    for i = 1, ((vtable_size - 4) / 2) do
        local entry_buffer = buffer(4 + ((i - 1) * 2), 2)
        local entry = entry_buffer:le_uint()
        entries[i] = entry
    end

    return {
        vtable_size = vtable_size,
        vtable_size_buffer = vtable_size_buffer,
        table_size = table_size,
        table_size_buffer = table_size_buffer,
        entries = entries
    }
end

local function Parse_TableInfo(buffer, offset, tree, field_type, member_list)
    local vtable_offset_buffer = buffer(offset, 4)
    local vtable_offset = vtable_offset_buffer:le_int()

    local vtable_data = Parse_VTable(buffer(offset - vtable_offset))

    -- create table subtree
    local table_buffer = buffer(offset, vtable_data.table_size)
    local table_bytes = table_buffer:raw()
    local subtree = tree:add(field_type, table_buffer, table_bytes, field_type.name)

    -- add vtable info to subtree
    if FB_VERBOSE then
        -- add vtable offset to tree
        local vtable_tree = subtree:add(fb_soffset_t, vtable_offset_buffer, -vtable_offset)
        vtable_tree:prepend_text("vtable offset: ")
        vtable_tree:append_text(" [0x" .. NumberToHex(buffer:offset() + offset - vtable_offset, 4) .. "]")

        local vtable_buffer = buffer(offset - vtable_offset, vtable_data.vtable_size)
        local vtable_bytes = vtable_buffer:raw()
        local vtable_description = "vtable[" .. #vtable_data.entries .. "]"
        local vtable_subtree = vtable_tree:add(fb_vtable, vtable_buffer, vtable_bytes, vtable_description)
        -- add vtable metadata
        local vtable_size_buffer = buffer(vtable_data.vtable_size_buffer:offset(), 2)
        local vtable_size_value = vtable_size_buffer:le_uint()
        local vtable_size_subtree = vtable_subtree:add(fb_uint16, vtable_size_buffer, vtable_size_value)
        vtable_size_subtree:prepend_text("VTable Size: ")

        local table_size_buffer = buffer(vtable_data.table_size_buffer:offset(), 2)
        local table_size_value = table_size_buffer:le_uint()
        local table_size_subtree = vtable_subtree:add(fb_uint16, table_size_buffer, table_size_value)
        table_size_subtree:prepend_text("Table Size: ")

        -- add vtable entries
        -- skip over vtable size and table size
        local entry_start = offset - vtable_offset + 4
        if (#vtable_data.entries ~= 0) then
            for i = 1, #vtable_data.entries do
                local entry_offset = entry_start + ((i - 1) * 2)
                Parse_VOffset(buffer, entry_offset, vtable_subtree, member_list[i][1], buffer:offset() + offset)
            end
        end
    end

    -- give the generated parser the subtree and vtable entries
    return {
        tree = subtree,
        value = vtable_data.entries
    }
end

function Parse_Table(buffer, offset, tree, field_type, member_list)
    -- we know its a table, so parse the table info to get the offsets
    local entry_list = Parse_TableInfo(buffer, offset, tree, field_type, member_list)

    for i = 1, #entry_list.value do
        local table_tree = entry_list.tree

        -- parse the data item here
        local data_item_offset = (entry_list.value[i] == 0) and 0 or (offset + entry_list.value[i])
        -- call this members' parser
        member_list[i][2](buffer, data_item_offset, table_tree, member_list[i][1])
    end

    return {
        tree = entry_list.tree,
        value = nil
    }
end
