use crate::{ForwardsUOffset, VOffsetT, SOffsetT, UOffsetT, Vector, SIZE_UOFFSET};
use crate::follow::Follow;
use thiserror::Error;

/// Traces the location of data errors. Not populated for Dos detecting errors.
/// Useful for MissingRequiredField and Utf8Error in particular, though
/// the other errors should not be producible by correct flatbuffers implementations.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum ErrorTraceDetail {
    Element {
        index: usize,
        position: usize
    },
    TableField {
        field_name: &'static str,
        position: usize,
    },
    UnionVariant {
        variant: &'static str,
        position: usize,
    },
}

/// Describes how a flatuffer is invalid and, for data errors, roughly where. No extra tracing
/// information is given for DoS detecting errors since it will probably be a lot.
#[derive(Clone, Error, Debug, PartialEq, Eq)]
pub enum InvalidFlatbuffer {
    MissingRequiredField {
        required: &'static str,
        error_trace: Vec<ErrorTraceDetail>,
    },
    InconsistentUnion {
        field: &'static str,
        field_type: &'static str,
        error_trace: Vec<ErrorTraceDetail>,
    },
    Utf8Error {
        utf8_error: std::str::Utf8Error,
        position: usize,
        error_trace: Vec<ErrorTraceDetail>,
    },
    MissingNullTerminator,  // TODO(caspern): This needs tracing.
    Unaligned {
        position: usize,
        error_trace: Vec<ErrorTraceDetail>,
    },
    RangeOutOfBounds {
        range: (usize, usize),
        error_trace: Vec<ErrorTraceDetail>,
    },
    SignedOffsetOutOfBounds {
        soffset: SOffsetT,
        position: usize,
        error_trace: Vec<ErrorTraceDetail>,
    },
    // Dos detecting errors. These do not get error traces since it will probably be very large.
    TooManyTables,
    ApparentSizeTooLarge,
    DepthLimitReached,
}

impl std::fmt::Display for InvalidFlatbuffer {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:?}", self)  // CASPER: do pretty displaying.
    }
}

pub type Result<T> = core::prelude::v1::Result<T, InvalidFlatbuffer>;

impl InvalidFlatbuffer {
    fn new_range_oob<T>(lb: usize, ub: usize) -> Result<T> {
        Err(Self::RangeOutOfBounds {
            range: (lb, ub),
            error_trace: Vec::new(),
        })
    }
    fn new_inconsistent_union<T>(field: &'static str, field_type: &'static str) -> Result<T> {
        Err(Self::InconsistentUnion { field, field_type, error_trace: Vec::new() })
    }
    fn new_missing_required<T>(required: &'static str) -> Result<T> {
        Err(Self::MissingRequiredField { required, error_trace: Vec::new() })
    }
    fn new_utf8_error(utf8_error: std::str::Utf8Error, position: usize) -> Self {
        Self::Utf8Error { utf8_error, position, error_trace: Vec::new() }
    }
    pub fn error_trace(&self) -> Option<&[ErrorTraceDetail]> {
        use InvalidFlatbuffer::*;
        match self {
            MissingRequiredField { error_trace, .. }
            | Unaligned { error_trace, .. }
            | RangeOutOfBounds { error_trace, .. }
            | InconsistentUnion { error_trace, ..}
            | SignedOffsetOutOfBounds { error_trace, .. } => Some(error_trace),
            _ => None,
        }
    }
    fn error_trace_mut(&mut self) -> Option<&mut Vec<ErrorTraceDetail>> {
        use InvalidFlatbuffer::*;
        match self {
            MissingRequiredField { error_trace, .. }
            | Unaligned { error_trace, .. }
            | RangeOutOfBounds { error_trace, .. }
            | InconsistentUnion { error_trace, ..}
            | SignedOffsetOutOfBounds { error_trace, .. } => Some(error_trace),
            _ => None,
        }
    }
}


/// Records the path to the verifier detail if the error is a data error and not a DoS error.
fn append_trace<T>(res: Result<T>, d: ErrorTraceDetail) -> Result<T> {
    res.map_err(|mut e| {
        e.error_trace_mut().map(|t| t.push(d));
        e
    })
}

/// Adds a TableField trace detail if `res` is a data error.
fn trace_field<T>(res: Result<T>, field_name: &'static str, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::TableField { field_name, position })
}
/// Adds a TableField trace detail if `res` is a data error.
fn trace_elem<T>(res: Result<T>, index: usize, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::Element { index, position })
}

pub struct VerifierOptions {
    /// Maximum depth of nested tables allowed in a valid flatbuffer.
    pub max_depth: usize,
    /// Maximum number of tables allowed in a valid flatbuffer.
    pub max_tables: usize,
    /// Maximum "apparent" size of the message if the Flatbuffer object DAG is expanded into a
    /// tree.
    pub max_apparent_size: usize,
    /// Ignore errors where a string is missing its null terminator.
    /// This is mostly a problem if the message will be sent to a client using old c-strings.
    pub ignore_missing_null_terminator: bool,
    // probably want an option to ignore utf8 errors since strings come from c++
    // options to error un-recognized enums and unions? possible footgun.
    // Ignore nested flatbuffers, etc?
}
impl Default for VerifierOptions {
    fn default() -> Self {
        Self {
            max_depth: 64,
            max_tables: 1_000_000,
            // size_ might do something different.
            max_apparent_size: 1 << 31,
            ignore_missing_null_terminator: false,
        }
    }
}

/// Carries the verification state. Should not be reused between tables.
pub struct Verifier<'opts, 'buf> {
    opts: &'opts VerifierOptions,
    buffer: &'buf [u8],
    depth: usize,
    num_tables: usize,
    apparent_size: usize, // TODO(casper): Actually track this...
}
impl<'opts, 'buf> Verifier<'opts, 'buf> {
    pub fn new(opts: &'opts VerifierOptions, buffer: &'buf [u8]) -> Self {
        Self {
            opts,
            buffer,
            depth: 0,
            num_tables: 0,
            apparent_size: 0,
        }
    }
    /// Resets verifier internal state.
    #[inline]
    pub fn reset(&mut self) {
        self.depth = 0;
        self.num_tables = 0;
        self.num_tables = 0;
    }
    /// Check that there really is a T in there.
    #[inline]
    fn is_aligned<T>(&self, pos: usize) -> Result<()> {
        // Safe because we're not dereferencing.
        let p = unsafe { self.buffer.as_ptr().add(pos) };
        if (p as usize) % std::mem::align_of::<T>() == 0 {
            Ok(())
        } else {
            Err(InvalidFlatbuffer::Unaligned {
                position: pos,
                error_trace: Vec::new(),
            })
        }
    }
    #[inline]
    fn range_in_buffer(&mut self, pos: usize, size: usize) -> Result<()> {
        if pos + size > self.buffer.len() {
            return InvalidFlatbuffer::new_range_oob(pos, pos + size);
        }
        self.apparent_size += size;
        if self.apparent_size > self.opts.max_apparent_size {
            return Err(InvalidFlatbuffer::ApparentSizeTooLarge);
        }
        Ok(())
    }
    #[inline]
    pub fn in_buffer<T>(&mut self, pos: usize) -> Result<()> {
        self.is_aligned::<T>(pos)?;
        self.range_in_buffer(pos, std::mem::size_of::<T>())
    }
    #[inline]
    fn get_u16(&mut self, pos: usize) -> Result<u16> {
        self.in_buffer::<u16>(pos)?;
        Ok(u16::from_le_bytes([self.buffer[pos], self.buffer[pos + 1]]))
    }
    #[inline]
    fn get_uoffset(&mut self, pos: usize) -> Result<UOffsetT> {
        self.in_buffer::<u32>(pos)?;
        Ok(u32::from_le_bytes([
            self.buffer[pos],
            self.buffer[pos + 1],
            self.buffer[pos + 2],
            self.buffer[pos + 3],
        ]))
    }
    #[inline]
    fn deref_soffset(&mut self, pos: usize) -> Result<usize> {
        self.in_buffer::<SOffsetT>(pos)?;
        let offset = SOffsetT::from_le_bytes([self.buffer[pos], self.buffer[pos + 1],
                               self.buffer[pos + 2], self.buffer[pos + 3]]);

        // signed offsets are subtracted.
        let derefed = if offset > 0 {
            pos.checked_sub(offset.abs() as usize)
        } else {
            pos.checked_add(offset.abs() as usize)
        };
        if let Some(x) = derefed {
            if x < self.buffer.len() {
                return Ok(x);
            }
        }
        Err(InvalidFlatbuffer::SignedOffsetOutOfBounds {
            soffset: offset,
            position: pos,
            error_trace: Vec::new(),
        })
    }
    #[inline]
    pub fn visit_table<'ver>(
        &'ver mut self,
        table_pos: usize,
    ) -> Result<TableVerifier<'ver, 'opts, 'buf>> {
        let vtable_pos = self.deref_soffset(table_pos)?;
        let vtable_len = self.get_u16(vtable_pos)? as usize;
        self.is_aligned::<VOffsetT>(vtable_pos + vtable_len)?;  // i.e. vtable_len is even.
        self.range_in_buffer(vtable_pos, vtable_len)?;
        // Check bounds.
        self.num_tables += 1;
        if self.num_tables > self.opts.max_tables {
            return Err(InvalidFlatbuffer::TooManyTables);
        }
        self.depth += 1;
        if self.depth > self.opts.max_depth {
            return Err(InvalidFlatbuffer::DepthLimitReached);
        }
        Ok(TableVerifier {
            pos: table_pos,
            vtable: vtable_pos,
            vtable_len,
            verifier: self,
        })
    }

    /// Runs the union variant's type's verifier assuming the variant is at the given position,
    /// tracing the error.
    pub fn verify_union_variant<T: Verifiable>(
        &mut self, variant: &'static str, position: usize
    ) -> Result<()> {
        let res = T::run_verifier(self, position);
        append_trace(res, ErrorTraceDetail::UnionVariant { variant, position })
    }
}

// Cache table metadata in usize so we don't have to cast types or jump around so much.
// We will visit every field anyway.
pub struct TableVerifier<'ver, 'opts, 'buf> {
    // Absolute position of table in buffer
    pos: usize,
    // Absolute position of vtable in buffer.
    vtable: usize,
    // Length of vtable.
    vtable_len: usize,
    // Verifier struct which holds the surrounding state and options.
    verifier: &'ver mut Verifier<'opts, 'buf>,
}
impl<'ver, 'opts, 'buf> TableVerifier<'ver, 'opts, 'buf> {

    fn deref(&mut self, field: VOffsetT) -> Result<Option<usize>> {
        let field = field as usize;
        if field < self.vtable_len {
            let field_offset = self.verifier.get_u16(self.vtable + field)?;
            if field_offset > 0 {
                // Field is present.
                let field_pos = self.pos + field_offset as usize;
                return Ok(Some(field_pos));
            }
        }
        Ok(None)
    }

    #[inline]
    pub fn visit_field<T: Verifiable>(
        mut self,
        field_name: &'static str,
        field: VOffsetT,
        required: bool,
    ) -> Result<Self> {
        if let Some(field_pos) = self.deref(field)? {
            trace_field(T::run_verifier(self.verifier, field_pos), field_name, field_pos)?;
            return Ok(self);
        }
        if required {
            InvalidFlatbuffer::new_missing_required(field_name)
        } else {
            Ok(self)
        }
    }
    #[inline]
    /// Union verification is complicated. The schemas passes this function the metadata of the
    /// union's key (discriminant) and value fields, and a callback. The function verifies and
    /// reads the key, then invokes the callback to perform data-dependent verification.
    pub fn visit_union<Key, UnionVerifier>(
        mut self,
        key_field_name: &'static str,
        key_field_voff: VOffsetT,
        val_field_name: &'static str,
        val_field_voff: VOffsetT,
        required: bool,
        verify_union: UnionVerifier
    ) -> Result<Self>
    where
        Key: Follow<'buf> + Verifiable,
        UnionVerifier: (std::ops::FnOnce(
            <Key as Follow<'buf>>::Inner, &mut Verifier, usize) -> Result<()>)
        // NOTE: <Key as Follow<'buf>>::Inner == Key
    {
        // TODO(caspern): how to trace vtable errors?
        let val_pos = self.deref(val_field_voff)?;
        let key_pos = self.deref(key_field_voff)?;
        match (key_pos, val_pos) {
            (None, None) => {
                if required {
                    InvalidFlatbuffer::new_missing_required(val_field_name)
                } else {
                    Ok(self)
                }
            },
            (Some(k), Some(v)) => {
                trace_field(Key::run_verifier(self.verifier, k), key_field_name, k)?;
                let discriminant = Key::follow(self.verifier.buffer, k);
                trace_field(verify_union(discriminant, self.verifier, v), val_field_name, v)?;
                Ok(self)
            }
            _ => InvalidFlatbuffer::new_inconsistent_union(key_field_name, val_field_name),
        }
    }
    pub fn finish(self) -> &'ver mut Verifier<'opts, 'buf> {
        self.verifier.depth -= 1;
        self.verifier
    }
}

// Needs to be implemented for Tables and maybe structs.
// Unions need some special treatment.
pub trait Verifiable {
    /// Runs the verifier for this type, assuming its at position `pos` in the verifier's buffer.
    /// Should not need to be called directly.
    // CASPER: Combine this with follow.
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()>;
}

#[inline]
/// Gets the root of the Flatbuffer, verifying it first with default options.
pub fn get_root<'buf, T: Follow<'buf> + Verifiable>(data: &'buf [u8]) -> Result<T::Inner> {
    let opts = VerifierOptions::default();
    get_root_with::<T>(&opts, data)
}
#[inline]
/// Gets the root of the Flatbuffer, verifying it first with default options.
pub fn get_root_with<'opts, 'buf, T: Follow<'buf> + Verifiable>(
    opts: &'opts VerifierOptions, data: &'buf [u8]
) -> Result<T::Inner> {
    let mut v = Verifier::new(&opts, data);
    <ForwardsUOffset<T>>::run_verifier(&mut v, 0)?;
    Ok(<ForwardsUOffset<T>>::follow(data, 0))
}

// Verify the uoffset and then pass verifier to the type being pointed to.
impl<T: Verifiable> Verifiable for ForwardsUOffset<T> {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let offset = v.get_uoffset(pos)? as usize;
        let next_pos = offset + pos;
        v.in_buffer::<u8>(pos)?;
        T::run_verifier(v, next_pos)
    }
}

// This is slightly more efficient than Vector<_, T> since it assumes the T are inline types
// i.e. structs and scalars, so checking bounds and alignment is sufficient.
impl<T: Verifiable> Verifiable for &[T] {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_uoffset(pos)? as usize;
        let data = pos + SIZE_UOFFSET;
        v.is_aligned::<T>(data)?;
        let size = std::mem::size_of::<T>();
        v.range_in_buffer(data, len * size)?;
        Ok(())
    }
}

impl<T: Verifiable> Verifiable for Vector<'_, T> {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_uoffset(pos)? as usize;
        let data = pos + std::mem::size_of::<u32>();
        v.is_aligned::<T>(data)?;
        let size = std::mem::size_of::<T>();
        v.range_in_buffer(data, len * size)?;
        for i in 0..len {
            let element_pos = data + i * size;
            trace_elem(T::run_verifier(v, element_pos), i, element_pos)?;
        }
        Ok(())
    }
}

impl<'a> Verifiable for &'a str {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_uoffset(pos)? as usize;
        let data = pos + SIZE_UOFFSET;
        v.range_in_buffer(data, len + 1)?;
        let s = &v.buffer[data..data + len];
        if !v.opts.ignore_missing_null_terminator && v.buffer[data + len] != 0 {
            return Err(InvalidFlatbuffer::MissingNullTerminator);
        }
        std::str::from_utf8(&s).map_err(|e| InvalidFlatbuffer::new_utf8_error(e, pos))?;
        Ok(())
    }
}

// Verify VectorOfTables, Unions, Arrays, Structs...
macro_rules! impl_verifiable_for {
    ($T: ty) => {
        impl Verifiable for $T {
            #[inline]
            fn run_verifier<'opts, 'buf>(
                v: &mut Verifier<'opts, 'buf>, pos: usize
            ) -> Result<()> {
                v.in_buffer::<$T>(pos)
            }
        }
    }
}
impl_verifiable_for!(bool);
impl_verifiable_for!(u8);
impl_verifiable_for!(i8);
impl_verifiable_for!(u16);
impl_verifiable_for!(i16);
impl_verifiable_for!(u32);
impl_verifiable_for!(i32);
impl_verifiable_for!(f32);
impl_verifiable_for!(u64);
impl_verifiable_for!(i64);
impl_verifiable_for!(f64);
