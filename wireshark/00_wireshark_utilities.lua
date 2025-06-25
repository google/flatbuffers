function Construct_Fields(...)
    local fields = {}
    local args = { ... }

    local function concat_tables(t1, t2)
        local t3 = {}
        -- Copy all elements from t1 into t3
        for i = 1, #t1 do
            t3[#t3 + 1] = t1[i]
        end
        -- Append all values from t2 into t3
        for _, value in pairs(t2) do
            t3[#t3 + 1] = value
        end
        return t3
    end

    for i, v in ipairs(args) do
        fields = concat_tables(fields, v)
    end

    return fields
end

function NumberToHex(value, num_bytes)
    return string.format("%0" .. (num_bytes * 2) .. "x", value)
end
