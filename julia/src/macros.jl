export @UNION, @DEFAULT, @ALIGN, @STRUCT, @with_kw

const __module__ = 0

function indexof(needle, haystack)
	for (i, v) in enumerate(haystack)
		v == needle && return i-1
	end
	return -1
end

macro UNION(T, TT)
	typeof(T) == Symbol || throw(ArgumentError("1st argument must be a symbol to represent a Union type"))
	TT.head == :tuple || throw(ArgumentError("2nd argument must be a tuple of types like `(T1,T2,...)`"))
	return esc(quote
		const $T = $(Expr(:curly, :Union, TT.args...))
		FlatBuffers.typeorder(::Type{$T}, ::Type{TT}) where {TT} = FlatBuffers.indexof(TT, $TT)
		FlatBuffers.typeorder(::Type{$T}, i::Integer) = ($TT)[i+1]
		FlatBuffers.isunionwithnothing(::Type{$T}) = false
	end)
end

macro ALIGN(T, sz)
	return esc(quote
		FlatBuffers.alignment(::Type{$T}) = $sz
	end)
end

macro enumtype(T, typ)
	return esc(quote
		FlatBuffers.enumtype(::Type{$T}) = $typ
	end)
end

# recursively finds largest field of a STRUCT
fbsizeof(::Type{T}) where {T<:Enum} = sizeof(enumtype(T))
fbsizeof(::Type{T}) where {T} = sizeof(T)

maxsizeof(::Type{T}) where {T<:Enum} = sizeof(enumtype(T))
maxsizeof(::Type{T}) where {T} = isbitstype(T) ? sizeof(T) : maximum(map(x->maxsizeof(x), T.types))

nextsizeof(::Type{T}) where {T} = isbitstype(T) ? sizeof(T) : nextsizeof(T.types[1])

function fieldlayout(mod, typ, exprs...)
	fields = Expr[]
	values = []
	largest_field = maximum(map(x->maxsizeof(Core.eval(mod, x.args[2])), exprs))
	sz = cur_sz = 0
	x = 0
	for (i,expr) in enumerate(exprs)
		T = Core.eval(mod, expr.args[2])
		if !isbitstype(T)
			exprs2 = [Expr(:(::), nm, typ) for (nm,typ) in zip(fieldnames(T),T.types)]
			fields2, values2 = fieldlayout(mod, T, exprs2...)
			append!(fields, map(x->Expr(:(::), Symbol(string(expr.args[1],'_',x.args[1])), x.args[2]), fields2))
			append!(values, map(x->x == 0 ? 0 : Expr(:call, :getfield, expr.args[1], QuoteNode(x)), values2))
		else
			push!(fields, expr)
			push!(values, expr.args[1])
		end
		sz += cur_sz = fbsizeof(T)
		if sz % largest_field == 0
			sz = cur_sz = 0
			continue
		end
		nextsz = i == length(exprs) ? 0 : nextsizeof(Core.eval(mod, exprs[i+1].args[2]))
		if i == length(exprs) || cur_sz < nextsz || (sz + nextsz) > largest_field
			# this is the last field and we're not `sz % largest_field`
			# potential diffs = 7, 6, 5, 4, 3, 2, 1
			sym = expr.args[1]
			diff = cur_sz < nextsz ? nextsz - cur_sz : largest_field - sz
			if diff == 7
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt8)); x += 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt16)); x += 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt32)); x += 1
				push!(values, 0); push!(values, 0); push!(values, 0)
			elseif diff == 6
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt16)); x += 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt32)); x += 1
				push!(values, 0); push!(values, 0)
			elseif diff == 5
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt8)); x += 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt32)); x += 1
				push!(values, 0); push!(values, 0)
			elseif diff == 4
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt32)); x += 1
				push!(values, 0)
			elseif diff == 3
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt8)); x += 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt16)); x += 1
				push!(values, 0); push!(values, 0)
			elseif diff == 2
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt16)); x += 1
				push!(values, 0)
			elseif diff == 1
				push!(fields, Expr(:(::), Symbol("_pad_$(sym)_$(typ)_$x"), :UInt8)); x += 1
				push!(values, 0)
			end
			sz = (sz + diff) % largest_field == 0 ? 0 : (cur_sz < nextsz ? sz + diff : 0)
			cur_sz = 0
		end
	end
	return fields, values
end

linefilter = x->typeof(x) != LineNumberNode

macro STRUCT(expr)
	!expr.args[1] || throw(ArgumentError("@STRUCT is only applicable for immutable types"))
	exprs = filter(linefilter, expr.args[3].args)
	fields, values = FlatBuffers.fieldlayout(__module__, expr.args[2], exprs...)
	expr.args[3].args = fields
	# generate convenience outer constructors if necessary
	# if there are nested structs or padding:
	# build an outer constructor that takes all direct, original fields
	# recursively flatten/splat all nested structs into one big args tuple
	# adding zeros for padded arguments
	# pass big, flat, args tuple to inner constructor
	T = expr.args[2]
	if any(x->!FlatBuffers.isbitstype(Core.eval(__module__, x.args[2])), exprs) ||
		length(fields) > length(exprs)
		exprs2 = map(x->FlatBuffers.isbitstype(Core.eval(__module__, x.args[2])) ? x.args[1] : x, exprs)
		sig = Expr(:call, T, exprs2...)
		body = Expr(:call, T, values...)
		outer = Expr(:function, sig, body)
	else
		outer = :(nothing)
	end
	return esc(quote
		$expr
		$outer
	end)
end

macro DEFAULT(T, kwargs...)
	ifblock = quote end
	if length(kwargs) > 0 && isa(kwargs[1], Expr) && length(kwargs[1].args) > 1
		for kw in kwargs
			push!(ifblock.args, :(if sym == $(QuoteNode(kw.args[1]))
				return $(kw.args[2])
			end))
		end
	end
	esc(quote
		if $T <: Enum
			FlatBuffers.default(::Type{$T}) = FlatBuffers.enumtype($T)($(kwargs[1]))
		else
			function FlatBuffers.default(::Type{$T}, TT, sym)
				$ifblock
				return FlatBuffers.default(TT)
			end
		end
	end)
end

import Parameters

function getdef(typedef::Expr)
	isstructexpr(x) = x isa Expr && x.head == :struct
	i = findfirst(x -> x isa Expr && any(isstructexpr.(x.args)), typedef.args)
	if i == nothing
		throw(ArgumentError("malformed @with_kw expression"))
	end
	wrapper = typedef.args[i]
	i = findfirst(isstructexpr, wrapper.args)
	def = wrapper.args[i]
	return def
end

function getfielddefs(typedef::Expr)
	[a for a in getdef(typedef).args[end].args if a isa Expr && a.head == :(::)]
end

function getconstructor(typedef::Expr)
	cons = getdef(typedef).args[end].args[end-1].args[1]
	typevars = []
	if :head in propertynames(cons) && cons.head == :where
		typevars = cons.args[2:end]
		cons = cons.args[1]
	end
	cons, typevars
end

function getkwtype(defs, name)
	for d in defs
		if :args in propertynames(d) && d.args[1] == name
			return d.args[end]
		end
	end
	return nothing
end

function createdefaultfns(typedef::Expr)
	cons, typevars = getconstructor(typedef)
	T = cons.args[1]
	params = cons.args[2]

	@assert params.head == :parameters

	kwargs = []
	defs = getfielddefs(typedef)
	kwdict = Dict{Any, Any}()
	for p in params.args
		name = p.args[1]
		t = getkwtype(defs, name)
		if t == nothing
			continue
		end
		value = p.args[end]
		ifblock = get(kwdict, t, quote end)
		push!(ifblock.args, :(if sym == $(QuoteNode(name))
			return convert($t, $value)
		end))
		kwdict[t] = ifblock
	end

	[:(function FlatBuffers.default(::Type{$T}, ::Type{$TT}, sym) where {$(typevars...)}
		$(kwdict[TT])
		return FlatBuffers.default($TT)
	end) for TT in keys(kwdict)]
end

macro with_kw(typedef)
	body = Parameters.with_kw(typedef, __module__, true)
	defaults = createdefaultfns(body)
	defaultsblock = Expr(:block, body, defaults...)
	esc(defaultsblock)
end

#TODO:
# handle id?
# nested_flatbuffer
