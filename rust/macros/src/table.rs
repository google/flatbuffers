use syntax::ext::base::{ExtCtxt, MacResult, MacEager};
use syntax::parse::token;
use syntax::util::small_vector::SmallVector;
use syntax::ext::quote::rt::ExtParseUtils;

use utils::*;

pub fn build_table_items(cx: &mut ExtCtxt,
                         name: token::InternedString,
                         fields: Vec<FieldDef>,
                         attributes: Vec<ObjAttribute>)
                         -> Result<Box<MacResult>, ()> {
    let struct_def = cx.parse_item(build_struct_def(&name));
    let build_impl = cx.parse_item(build_impl(&name, &fields));
    let from_impl = cx.parse_item(build_from(&name));
    let vtype = cx.parse_item(build_vector_type(&name, &attributes));
    Ok(MacEager::items(SmallVector::many(vec![struct_def, build_impl, from_impl, vtype])))
}

pub fn build_struct_items(cx: &mut ExtCtxt,
                          name: token::InternedString,
                          fields: Vec<FieldDef>,
                          attributes: Vec<ObjAttribute>)
                          -> Result<Box<MacResult>, ()> {
    let struct_def = cx.parse_item(build_struct_def(&name));
    let build_impl = cx.parse_item(build_struct_impl(&name, &fields));
    let from_impl = cx.parse_item(build_from(&name));
    let vtype = cx.parse_item(build_vector_type(&name, &attributes));
    Ok(MacEager::items(SmallVector::many(vec![struct_def, build_impl, from_impl, vtype])))
}

fn build_struct_def(name: &token::InternedString) -> String {
    format!("#[derive(Debug, Clone, PartialEq, Eq)] pub struct {}<T: AsRef<[u8]>>(flatbuffers::Table<T>);",
            name)
}

fn build_impl(name: &token::InternedString, fields: &Vec<FieldDef>) -> String {
    let mut str1 = format!("impl<T: AsRef<[u8]>> {}<T> {{\n", name);
    for field in fields {
        str1 = format!("{} fn {}(&self) -> {} {{ {} }}",
                       str1,
                       name,
                       field.ty.base_type(),
                       field.ty.get_table_accessor(&field.slot, &field.default));
    }
    format!("{} }}", str1)
}

fn build_struct_impl(name: &token::InternedString, fields: &Vec<FieldDef>) -> String {
    let mut str1 = format!("impl<T: AsRef<[u8]>> {}<T> {{ \n", name);
    for field in fields {
        str1 = format!("{} fn {}(&self) -> {} {{ {} }}",
                       str1,
                       field.name.to_lowercase(),
                       field.ty.base_type(),
                       field.ty.get_struct_accessor(&field.slot));
    }
    format!("{} }}", str1)
}

fn build_from(name: &token::InternedString) -> String {
    format!("impl<T: AsRef<[u8]>> From<flatbuffers::Table<T>> for {}<T> {{ fn from(table: \
             flatbuffers::Table<T>) -> {}<T> {{ {}(table) }}}}",
            name,
            name,
            name)

}

fn build_vector_type(name: &token::InternedString, attributes: &[ObjAttribute]) -> String {
    let size = find_attribute("size", attributes).unwrap_or("4".to_string());
    let size = size.parse::<u16>().unwrap();
    format!("impl<'a, T: AsRef<[u8]>> flatbuffers::VectorType<'a> for {}<T> {{ \
             type Item = {}<&'a [u8]>;
             fn inline_size() -> usize {{ \
                {} \
            }} \
            fn read_next(buffer:&'a [u8]) -> Self::Item {{ \
               let table = flatbuffers::Table::get_indirect_root(buffer, 0); \
               table.into() \
               }} }}",
            name,
            name,
            size)
}
