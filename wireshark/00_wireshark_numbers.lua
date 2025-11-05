local function AddFieldToTree(buffer, offset, tree, field_type, scalar_type, field_width, default_value)
    if offset == 0 then
        local subtree = tree:add(field_type, default_value)
        subtree:append_text(" [DEFAULT VALUE]")
        return {
            tree = subtree,
            value = nil
        }
    end

    local field_buffer = buffer(offset, field_width)
    local field_value = field_buffer["le_" .. scalar_type](field_buffer)
    return {
        tree = tree:add(field_type, field_buffer, field_value),
        value = field_value
    }
end

function Parse_Uint8(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        1, default_value)
end

function Parse_Uint16(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        2, default_value)
end

function Parse_Uint32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint",
        4, default_value)
end

function Parse_Uint64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "uint64",
        8, default_value)
end

function Parse_Int8(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type,
        "int", 1, default_value)
end

function Parse_Int16(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int",
        2, default_value)
end

function Parse_Int32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int",
        4, default_value)
end

function Parse_Int64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "int64",
        8, default_value)
end

function Parse_Float32(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "float",
        4, default_value)
end

function Parse_Float64(buffer, offset, tree, field_type, default_value)
    return AddFieldToTree(buffer, offset, tree, field_type, "double",
        8, default_value)
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

    local bool_buffer = buffer(offset, 1)
    local bool_value = bool_buffer:le_uint()
    return {
        tree = tree:add(field_type, bool_buffer, bool_value),
        value = bool_value
    }
end
