use crate::{ForwardsUOffset, VOffsetT, Vector, SIZE_U32};
use crate::follow::Follow; // CASPER;
use thiserror::Error;

/// Traces the location of data errors. Not populated for Dos detecting errors.
/// Useful for MissingRequiredField in particular, the other errors should not be producible
/// be a correct flatbuffers implementation.
#[derive(Clone, Debug)]
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

// Derive error, impl error trait
#[derive(Clone, Error, Debug)]
pub enum InvalidFlatbuffer {
    MissingRequiredField {
        required: &'static str,
        error_trace: Vec<ErrorTraceDetail>,
    },
    Unaligned {
        position: usize,
        error_trace: Vec<ErrorTraceDetail>,
    },
    OutOfBounds {
        range: (usize, usize),
        error_trace: Vec<ErrorTraceDetail>,
    },
    // CASPER: this really should have an error trace too.
    #[error(transparent)]
    Utf8Error(#[from] std::str::Utf8Error),
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


impl InvalidFlatbuffer {
    fn new_oob<T>(lb: usize, ub: usize) -> Result<T> {
        Err(Self::OutOfBounds {
            range: (lb, ub),
            error_trace: Vec::new(),
        })
    }
}

pub type Result<T> = core::prelude::v1::Result<T, InvalidFlatbuffer>;

/// Records the path to the verifier detail if the error is a data error and not a DoS error.
pub fn append_trace<T>(mut res: Result<T>, d: ErrorTraceDetail) -> Result<T> {
    // TODO(cneo): Unlikely branch hint?
    if let Err(err) = res.as_mut() {
        use InvalidFlatbuffer::*;
        match err {
            MissingRequiredField { error_trace, .. }
            | Unaligned { error_trace, .. }
            | OutOfBounds { error_trace, .. } => error_trace.push(d),
            _ => (),
        };
    }
    res
}

/// Adds a TableField trace detail if `res` is a data error.
pub fn trace_field<T>(res: Result<T>, field_name: &'static str, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::TableField { field_name, position })
}
/// Adds a TableField trace detail if `res` is a data error.
pub fn trace_union_variant<T>(res: Result<T>, variant: &'static str, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::UnionVariant { variant, position })
}
/// Adds a TableField trace detail if `res` is a data error.
pub fn trace_elem<T>(res: Result<T>, index: usize, position: usize) -> Result<T> {
    append_trace(res, ErrorTraceDetail::Element { index, position })
}

pub struct VerifierOptions {
    max_depth: usize,
    max_tables: usize,
    max_apparent_size: usize,
    // probably want an option to ignore utf8 errors since strings come from c++
}
impl Default for VerifierOptions {
    fn default() -> Self {
        Self {
            max_depth: 64,
            max_tables: 1_000_000,
            // size_ might do something different.
            max_apparent_size: 1 << 31,
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
    pub fn is_aligned<T>(&self, pos: usize) -> Result<()> {
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
    pub fn range_in_buffer(&self, pos: usize, size: usize) -> Result<()> {
        if pos + size < self.buffer.len() {
            Ok(())
        } else {
            InvalidFlatbuffer::new_oob(pos, pos + size)
        }
    }
    #[inline]
    pub fn in_buffer<T>(&self, pos: usize) -> Result<()> {
        self.is_aligned::<T>(pos)?;
        self.range_in_buffer(pos, std::mem::size_of::<T>())
    }
    #[inline]
    fn get_u16(&self, pos: usize) -> Result<u16> {
        self.in_buffer::<u16>(pos)?;
        Ok(u16::from_le_bytes([self.buffer[pos], self.buffer[pos + 1]]))
    }
    #[inline]
    fn get_i16(&self, pos: usize) -> Result<i16> {
        self.in_buffer::<i16>(pos)?;
        Ok(i16::from_le_bytes([self.buffer[pos], self.buffer[pos + 1]]))
    }
    #[inline]
    fn get_u32(&self, pos: usize) -> Result<u32> {
        self.in_buffer::<u32>(pos)?;
        Ok(u32::from_le_bytes([
            self.buffer[pos],
            self.buffer[pos + 1],
            self.buffer[pos + 2],
            self.buffer[pos + 3],
        ]))
    }
    pub fn deref_uoffset(&self, pos: usize) -> Result<usize> {
        // CASPER: UOffset parameterizable?
        let offset = self.get_u32(pos)?;
        let new_pos = pos + offset as usize;
        self.in_buffer::<u8>(new_pos)?;
        Ok(new_pos)
    }
    pub fn deref_soffset(&self, pos: usize) -> Result<usize> {
        let offset = self.get_i16(pos)?;
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
        // CASPER: this is wrong.
        dbg!("SOFFSET");
        InvalidFlatbuffer::new_oob(pos, pos + 1)
    }
    #[inline]
    pub fn visit_table<'ver>(
        &'ver mut self,
        table_pos: usize,
    ) -> Result<TableVerifier<'ver, 'opts, 'buf>> {
        let vtable_pos = self.deref_soffset(table_pos)?;
        let vtable_len = self.get_u16(vtable_pos)? as usize;
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
    #[inline]
    pub fn visit_field<T: Verifiable>(
        &mut self,
        field_name: &'static str,
        field: VOffsetT,
        required: bool,
    ) -> Result<&mut Self> {
        let field = field as usize;
        if field + std::mem::size_of::<VOffsetT>() < self.vtable_len {
            let field_offset = self.verifier.get_u16(self.vtable + field)?;
            if field_offset > 0 {
                // Field is present.
                let field_pos = self.pos + field_offset as usize;
                trace_field(T::run_verifier(self.verifier, field_pos), field_name, field_pos)?;
                return Ok(self);
            }
        }
        if required {
            Err(InvalidFlatbuffer::MissingRequiredField {
                required: field_name,
                error_trace: Vec::new(),
            })
        } else {
            Ok(self)
        }
    }
}
impl<'ver, 'opts, 'buf> std::ops::Drop for TableVerifier<'ver, 'opts, 'buf> {
    #[inline]
    fn drop(&mut self) {
        self.verifier.depth -= 1;
    }
}

// Needs to be implemented for Tables and maybe structs.
// Unions need some special treatment.
pub trait Verifiable {
    /// Runs the verifier for this type, assuming its at position `pos` in the verifier's buffer.
    /// Should not need to be called directly.
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()>;
}


pub fn get_root<'buf, T: Follow<'buf> + Verifiable>(data: &'buf [u8]) -> Result<T::Inner> {
    let opts = VerifierOptions::default();
    let mut v = Verifier::new(&opts, data);
    <ForwardsUOffset<T>>::run_verifier(&mut v, 0)?;
    Ok(<ForwardsUOffset<T>>::follow(data, 0))
}

// Verify the uoffset and then pass verifier to the type being pointed to.
impl<T: Verifiable> Verifiable for ForwardsUOffset<T> {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let offset = v.get_u32(pos)? as usize;
        let next_pos = offset + pos;
        dbg!(("Forward", pos, offset, next_pos, std::any::type_name::<T>()));
        v.in_buffer::<u8>(pos)?;
        T::run_verifier(v, next_pos)
    }
}

// CASPER: FIXME: this is a dumb hack so I don't need to refactor idl_gen_rust.
impl<T: Verifiable> Verifiable for Option<T> {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        T::run_verifier(v, pos)
    }
}

// CASPER: FIXME: this is a dumb hack so I don't need to refactor idl_gen_rust.
impl<T: Verifiable> Verifiable for &T {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        T::run_verifier(v, pos)
    }
}

// CASPER: DELETE ME actually need to handle unions correctly.
impl Verifiable for crate::Table<'_> {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        v.visit_table(pos)?;
        Ok(())
    }
}

impl<T: Verifiable> Verifiable for &[T] {
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_u32(pos)? as usize;
        let data = pos + std::mem::size_of::<u32>();  // CASPER: use the common constant.
        v.is_aligned::<T>(data)?;
        let size = std::mem::size_of::<T>();
        v.range_in_buffer(data, len * size)?;
        Ok(())
        // CASPER: share code between slice, vector, string
    }
}

impl<T: Verifiable> Verifiable for Vector<'_, T> {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_u32(pos)? as usize;
        let data = pos + std::mem::size_of::<u32>();
        // CASPER: what happens when its a vector of struct that's 8byte aligned? Hopefully
        // `data` is 8byte aligned and `len` is 4 byte aligned.
        v.is_aligned::<T>(data)?;
        let size = std::mem::size_of::<T>();
        v.range_in_buffer(data, len * size)?;
        dbg!(("Vector", pos, len, size, std::any::type_name::<T>()));
        for i in 0..len {
            let element_pos = data + i * size;
            dbg!(i);
            trace_elem(T::run_verifier(v, element_pos), i, element_pos)?;
        }
        Ok(())
    }
}

impl<'a> Verifiable for &'a str {
    #[inline]
    fn run_verifier<'opts, 'buf>(v: &mut Verifier<'opts, 'buf>, pos: usize) -> Result<()> {
        let len = v.get_u32(pos)? as usize;
        v.range_in_buffer(pos + SIZE_U32, len)?;
        let s = &v.buffer[pos + SIZE_U32..pos + SIZE_U32 + len];
        std::str::from_utf8(&s)?;
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

/* CASPER: ideas for traits for common flatbuffers stuff. There is no need for this and it would
make users have to import these traits probably. It really only serves as documentation and type
safety that we'd test anyway.

/// Tables with this trait may be unpacked into a more Rust native
/// representation. This trades speed for mutability anod ergonomics.
trait<'buf> GeneratedTableWithObject<'buf> : GeneratedTable<'buf> {
    // CASPER: what about structs, enums, and nontables?
    type Object: GeneratedObject;
    fn unpack(&self) -> Self::Object;
}

/// Types implementing this trait are the native froms of Flatbuffers data.
/// They may be serialized into a FlatBufferBuilder.
trait<'buf> GeneratedObject {
    type GTable: GeneratedTableWithObject<'buf>;
    fn pack(
        &self, fbb: &mut FlatBufferBuilder<'buf>
    ) -> WIPOffset<Self::GTable<'buf>>;
}


/// Type implementing this trait can be read from and write to
/// Flatbuffer binary data.
trait<'args, 'bldr, 'buf> GeneratedTable<'buf>: Follow<'buf> {
    type Args<'args>: Default;
    type Builder: GeneratedTableBuilder<'buf, 'bldr>;

    /// This constructor does not verify that a table is a valid
    /// Flatbuffer. Callers must trust the accessed fields will pass
    /// the table's verifier or accessors may panic or worse.
    unsafe fn init_from_table(table: Table<'a>) -> Self;

    /// Constructs a table inside the FlatBufferBuilder.
    fn create<'bldr>(
        fbb: &mut FlatBufferBuilder<'bldr>,
        args: &Self::Args<'args>,
    ) -> flatbuffers::WIPOffset<Self<'bldr>>;
}

/// Types implementing this trait provide a generated API
/// to build `Self::GTable` more easily.
trait<'bldr, 'buf> GeneratedTableBuilder<'buf, 'bldr>
where 'buf: 'bldr
{
    type GTable: GeneratedTable;
    fn new(fbb: &'bldr FlatBufferBuilder<'buf>) -> Self<'buf, 'bldr>;
    fn finish(self) -> WIPOffset<Self::GTable<'buf>>;
}

*/
