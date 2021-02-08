/// The underlying buffer that is used by a flexbuffer Reader. 
pub trait FlexBuffer: AsRef<[u8]> + Clone + Default {}

impl<B: AsRef<[u8]> + Clone + Default> FlexBuffer for B {}

