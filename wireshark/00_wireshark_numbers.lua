function GetNumberBuffer(buffer, offset, proto_field)
    local field_size = {
        [fb_bool] = 1,
        [fb_uint8] = 1,
        [fb_int8] = 1,
        [fb_uint16] = 2,
        [fb_int16] = 2,
        [fb_uint32] = 4,
        [fb_int32] = 4,
        [fb_float32] = 4,
        [fb_uint64] = 8,
        [fb_int64] = 8,
        [fb_float64] = 8
    }

    return buffer(offset, field_size[proto_field])
end

local function AddFieldToTree(buffer, offset, tree, field_type, scalar_type, proto_field, default_value)
    if offset == 0 then
        local subtree = tree:add(field_type, default_value)
        subtree:append_text(" [DEFAULT VALUE]")
        return {
            tree = subtree,
            value = nil
        }
    end

    local field_buffer = GetNumberBuffer(buffer, offset, proto_field)
    local field_value = field_buffer["le_" .. scalar_type](field_buffer)
    return {
        tree = tree:add(field_type, field_buffer, field_value),
        value = field_value
    }
end

function Parse_Uint8(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        fb_uint8, default_value)
end

function Parse_Uint16(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        fb_uint16, default_value)
end

function Parse_Uint32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        fb_uint32, default_value)
end

function Parse_Uint64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint64",
        fb_uint64, default_value)
end

function Parse_Int8(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type,
        "int", fb_int8, default_value)
end

function Parse_Int16(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int",
        fb_int16, default_value)
end

function Parse_Int32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int",
        fb_int32, default_value)
end

function Parse_Int64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int64",
        fb_int64, default_value)
end

function Parse_Float32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "float",
        fb_float32, default_value)
end

function Parse_Float64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "double",
        fb_float64, default_value)
end

function Parse_Bool(buffer, offset, tree, field_type, default_value)
    if offset == 0 then
        local subtree = tree:add(field_type, default_value)
        subtree:append_text(" [DEFAULT VALUE]")
        return {
            tree = subtree,
            value = default_value
        }
    end

    local bool_buffer = GetNumberBuffer(buffer, offset, fb_bool)
    local bool_value = bool_buffer:le_uint()
    return {
        tree = tree:add(field_type, bool_buffer, bool_value),
        value = bool_value
    }
end
