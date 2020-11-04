use crate::follow::Follow;
use crate::{ForwardsUOffset, SOffsetT, UOffsetT, VOffsetT, Vector, SIZE_UOFFSET,
SafeSliceAccess};
use thiserror::Error;
use std::ops::Range;

/// Traces the location of data errors. Not populated for Dos detecting errors.
/// Useful for MissingRequiredField and Utf8Error in particular, though
/// the other errors should not be producible by correct flatbuffers implementations.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum ErrorTraceDetail {
    VectorElement {
        index: usize,
        position: usize,
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
#[derive(PartialEq, Eq, Default, Debug, Clone)]
pub struct ErrorTrace(Vec<ErrorTraceDetail>);
impl std::convert::AsRef<[ErrorTraceDetail]> for ErrorTrace {
    fn as_ref(&self) -> &[ErrorTraceDetail] {
        &self.0
    }
}

/// Describes how a flatuffer is invalid and, for data errors, roughly where. No extra tracing
/// information is given for DoS detecting errors since it will probably be a lot.
#[derive(Clone, Error, Debug, PartialEq, Eq)]
pub enum InvalidFlatbuffer {
    MissingRequiredField {
        required: &'static str,
        error_trace: ErrorTrace,
    },
    InconsistentUnion {
        field: &'static str,
        field_type: &'static str,
        error_trace: ErrorTrace,
    },
    BadString {
        utf8_error: Option<std::str::Utf8Error>,
        has_null_terminator: bool,
        range: Range<usize>,
        error_trace: ErrorTrace,
    },
    Unaligned {
        position: usize,
        unaligned_type: &'static str,
        error_trace: ErrorTrace,
    },
    RangeOutOfBounds {
        range: Range<usize>,
        error_trace: ErrorTrace,
    },
    SignedOffsetOutOfBounds {
        soffset: SOffsetT,
        position: usize,
        error_trace: ErrorTrace,
    },
    // Dos detecting errors. These do not get error traces since it will probably be very large.
    TooManyTables,
    ApparentSizeTooLarge,
    DepthLimitReached,
}

impl std::fmt::Display for ErrorTrace {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        use ErrorTraceDetail::*;
        for e in self.0.iter() {
            match e {
                VectorElement { index, position } => {
                    writeln!(
                        f,
                        "\twhile verifying vector element {:?} at position {:?}",
                        index, position
                    )?;
                }
                TableField {
                    field_name,
                    position,
                } => {
                    writeln!(
                        f,
                        "\twhile verifying table field `{}` at position {:?}",
                        field_name, position
                    )?;
                }
                UnionVariant { variant, position } => {
                    writeln!(
                        f,
                        "\t while verifying union variant `{}` at position {:?}",
                        variant, position
                    )?;
                }
            }
        }
        Ok(())
    }
}

impl std::fmt::Display for InvalidFlatbuffer {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "InvalidFlatbuffer: ")?;
        use InvalidFlatbuffer::*;
        match self {
            TooManyTables => write!(f, "Too many tables.")?,
            ApparentSizeTooLarge => write!(f, "Apparent size too large.")?,
            DepthLimitReached => {
                write!(f, "Nested table depth limit reached.")?
            }
            MissingRequiredField {
                required,
                error_trace,
            } => {
                write!(
                    f,
                    "Missing required field `{}`.\n{}",
                    required, error_trace
                )?;
            }
            InconsistentUnion {
                field,
                field_type,
                error_trace,
            } => {
                write!(
                    f,
                    "Union exactly one of union discriminant (`{}`) and value\
                     (`{}`) are present. .\n{}",
                    field_type, field, error_trace
                )?;
            }
            BadString {
                utf8_error,
                has_null_terminator,
                range,
                error_trace,
            } => {
                write!(f, "string at range [{}, {}) ", range.start, range.end)?;
                if !has_null_terminator {
                    write!(f, "is missing its null terminator")?;
                    if utf8_error.is_some() {
                        write!(f, "and ")?;
                    }
                }
                if let Some(e) = utf8_error {
                    write!(f, "is invalid utf8 ({})", e)?;
                }
                write!(f, ".\n{}", error_trace)?;
            }
            Unaligned {
                position,
                unaligned_type,
                error_trace,
            } => {
                write!(
                    f,
                    "Type `{}` at position {:?} is unaligned.\n{}",
                    unaligned_type, position, error_trace
                )?;
            }
            RangeOutOfBounds { range, error_trace } => {
                write!(
                    f,
                    "Range [{}, {}) is out of bounds.\n{}",
                    range.start, range.end, error_trace
                )?;
            }
            SignedOffsetOutOfBounds {
                soffset,
                position,
                error_trace,
            } => {
                write!(
                    f,
                    "Signed Offset at position {:?} has value {:?} \
                     which points out of bounds.\n{}",
                    position, soffset, error_trace
                )?;
            }
        }
        Ok(())
    }
}

pub type Result<T> = core::prelude::v1::Result<T, InvalidFlatbuffer>;

impl InvalidFlatbuffer {
    fn new_range_oob<T>(start: usize, end: usize) -> Result<T> {
        Err(Self::RangeOutOfBounds {
            range: Range { start, end },
            error_trace: Default::default(),
        })
    }
    fn new_inconsistent_union<T>(field: &'static str, field_type: &'static str) -> Result<T> {
        Err(Self::InconsistentUnion {
            field,
            field_type,
            error_trace: Default::default(),
        })
    }
    fn new_missing_required<T>(required: &'static str) -> Result<T> {
        Err(Self::MissingRequiredField {
            required,
            error_trace: Default::default(),
        })
    }
}

/// Records the path to the verifier detail if the error is a data error and not a DoS error.
fn append_trace<T>(mut res: Result<T>, d: ErrorTraceDetail) -> Result<T> {
    if let Err(e) = res.as_mut() {
        use InvalidFlatbuffer::*;
        if let MissingRequiredField { error_trace, .. }
        | Unaligned { error_trace, .. }
        | RangeOutOfBounds { error_trace, .. }
        | InconsistentUnion { error_trace, .. }
        | BadString { error_trace, .. }
        | SignedOffsetOutOfBounds { error_trace, .. } = e
        {
            error_trace.0.push(d)
        }
    }
    res
}

/// Adds a TableField trace detail if `res` is a data error.
fn trace_field<T>(res: Result<T>, field_name: &'static str, position: usize) -> Result<T> {
    append_trace(
        res,
        ErrorTraceDetail::TableField {
            field_name,
            position,
        },
    )
}
/// Adds a TableField trace detail if `res` is a data error.
fn trace_elem<T>(res: Result<T>, index: usize, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::VectorElement { index, position })
}

#[derive(Debug, Clone, PartialEq, Eq)]
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
#[derive(Debug)]
pub struct Verifier<'opts, 'buf> {
    buffer: &'buf [u8],
    opts: &'opts VerifierOptions,
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
                unaligned_type: std::any::type_name::<T>(),
                position: pos,
                error_trace: Default::default(),
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
        let offset = SOffsetT::from_le_bytes([
            self.buffer[pos],
            self.buffer[pos + 1],
            self.buffer[pos + 2],
            self.buffer[pos + 3],
        ]);

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
            error_trace: Default::default(),
        })
    }
    #[inline]
    pub fn visit_table<'ver>(
        &'ver mut self,
        table_pos: usize,
    ) -> Result<TableVerifier<'ver, 'opts, 'buf>> {
        let vtable_pos = self.deref_soffset(table_pos)?;
        let vtable_len = self.get_u16(vtable_pos)? as usize;
        self.is_aligned::<VOffsetT>(vtable_pos + vtable_len)?; // i.e. vtable_len is even.
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
        &mut self,
        variant: &'static str,
        position: usize,
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
            trace_field(
                T::run_verifier(self.verifier, field_pos),
                field_name,
                field_pos,
            )?;
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
        verify_union: UnionVerifier,
    ) -> Result<Self>
    where
        Key: Follow<'buf> + Verifiable,
        UnionVerifier:
            (std::ops::FnOnce(<Key as Follow<'buf>>::Inner, &mut Verifier, usize) -> Result<()>), // NOTE: <Key as Follow<'buf>>::Inner == Key
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
            }
            (Some(k), Some(v)) => {
                trace_field(Key::run_verifier(self.verifier, k), key_field_name, k)?;
                let discriminant = Key::follow(self.verifier.buffer, k);
                trace_field(
                    verify_union(discriminant, self.verifier, v),
                    val_field_name,
                    v,
                )?;
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
    opts: &'opts VerifierOptions,
    data: &'buf [u8],
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
        v.in_buffer::<u8>(pos)?;  // CASPER: is this needed?
        T::run_verifier(v, next_pos)
    }
}


/// Checks and returns the range containing the flatbuffers vector.
fn verify_vector_range<'opts, 'buf, T>(
    v: &mut Verifier<'opts, 'buf>,
    pos:usize
) -> Result<std::ops::Range<usize>> {
    let len = v.get_uoffset(pos)? as usize;
    let data = pos + SIZE_UOFFSET;
    v.is_aligned::<T>(data)?;
    let size = std::mem::size_of::<T>();
    v.range_in_buffer(data, len * size)?;
    Ok(std::ops::Range {
        start: data,
        end: data + len * size,
    })
}

// This is slightly more efficient than Vector<_, T> since it assumes the T are inline types
// i.e. structs and scalars, so checking bounds and alignment is sufficient.
impl<T: Verifiable> Verifiable for &[T] {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        verify_vector_range::<T>(v, pos)?;
        Ok(())
    }
}


pub trait SimpleToVerify {}
impl<T: SafeSliceAccess> SimpleToVerify for T {}
// impl SimpleToVerify for i16 {}  // CASPER: Get rid of SafeSliceAccess
// impl SimpleToVerify for u16 {}
// impl SimpleToVerify for i32 {}
// impl SimpleToVerify for u32 {}
// impl SimpleToVerify for f32 {}
// impl SimpleToVerify for i64 {}
// impl SimpleToVerify for u64 {}
// impl SimpleToVerify for f64 {}



impl<T: SimpleToVerify> Verifiable for Vector<'_, T> {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        verify_vector_range::<T>(v, pos)?;
        Ok(())
    }

}

impl<T: Verifiable> Verifiable for Vector<'_, ForwardsUOffset<T>> {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let range = verify_vector_range::<ForwardsUOffset<T>>(v, pos)?;
        let size = std::mem::size_of::<ForwardsUOffset<T>>();
        for (i, element_pos) in range.step_by(size).enumerate() {
            trace_elem(<ForwardsUOffset<T>>::run_verifier(v, element_pos), i, element_pos)?;
        }
        Ok(())
    }
}

impl<'a> Verifiable for &'a str {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let range = verify_vector_range::<u8>(v, pos)?;
        let has_null_terminator = v.buffer.get(range.end).map(|&b| b == 0).unwrap_or(false);
        let s = std::str::from_utf8(&v.buffer[range.clone()]);
        if s.is_err() || !v.opts.ignore_missing_null_terminator && !has_null_terminator {
            return Err(InvalidFlatbuffer::BadString {
                utf8_error: s.err(),
                has_null_terminator,
                range,
                error_trace: Default::default(),
            });
        }
        Ok(())
    }
}

// Verify VectorOfTables, Unions, Arrays, Structs...
macro_rules! impl_verifiable_for {
    ($T: ty) => {
        impl Verifiable for $T {
            #[inline]
            fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
                v.in_buffer::<$T>(pos)
            }
        }
    };
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
