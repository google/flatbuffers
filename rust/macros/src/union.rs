use syntax::ext::base::{ExtCtxt, MacResult, MacEager};
use syntax::parse::token;
use syntax::util::small_vector::SmallVector;
use syntax::ext::quote::rt::ExtParseUtils;

use utils::*;

pub fn build_simple_enum(cx: &mut ExtCtxt,
                         name: token::InternedString,
                         ty: FieldType,
                         items: Vec<EnumItem>)
                         -> Result<Box<MacResult>, ()> {
    let enum_def = cx.parse_item(build_enum_def(&name, &ty.base_type(), &items));
    let from_def = cx.parse_item(build_from_def(&name, &ty.base_type(), &items));
    let vtype = try!(build_enum_vector_type(&name, &ty));
    let vtype_def = cx.parse_item(vtype);
    Ok(MacEager::items(SmallVector::many(vec![enum_def, from_def, vtype_def])))
}

pub fn build_union(cx: &mut ExtCtxt,
                   name: token::InternedString,
                   items: Vec<EnumItem>)
                   -> Result<Box<MacResult>, ()> {
    let union_def = cx.parse_item(build_union_enum_def(&name, &items));
    let type_name = format!("{}Type", &name);
    let union_new_def = cx.parse_item(build_union_new_def(&name, &type_name, &items));
    let enum_def = cx.parse_item(build_enum_def(&type_name, "u8", &items));
    let from_def = cx.parse_item(build_from_def(&type_name, "u8", &items));
    Ok(MacEager::items(SmallVector::many(vec![union_def, union_new_def, enum_def, from_def])))
}

fn build_union_enum_def(name: &str, items: &[EnumItem]) -> String {
    let mut str1 = format!("#[derive(Debug)] \
                            pub enum {}<T: AsRef<[u8]>> {{\n",
                           name);
    for item in items {
        str1 = format!("{} {}({}<T>),\n", str1, item.name, item.name);
    }
    format!("{} }}", str1)
}

fn build_union_new_def(name: &str, type_name: &str, items: &[EnumItem]) -> String {
    let mut str1 = format!("impl<T: AsRef<[u8]>> {}<T> {{ \
                            pub fn new(table: flatbuffers::Table<T>, utype: Option<{}>) -> Option<{}<T>> {{
                match utype {{", name, type_name, name);
    for item in items {
        str1 = format!("{} Some({}::{}) => Some( {}::{}(table.into() ) ), ", str1, type_name,
                       item.name, name, item.name);
    }
    format!("{} _ => None  }} }}  }}", str1)
}


fn build_enum_def(name: &str, ty: &str, items: &[EnumItem]) -> String {
    let mut str1 = format!("#[derive(PartialEq, Eq, Clone, Copy, Debug, Hash)]\n \
        #[repr({})]\n \
        pub enum {} {{\n",
                           ty,
                           name);
    for item in items {
        str1 = format!("{} {} = {},\n", str1, item.name, item.value);
    }
    format!("{} }}", str1)
}

fn build_from_def(name: &str, ty: &str, items: &[EnumItem]) -> String {
    let mut str1 =
        format!("impl {} {{ pub fn from(value: {}) -> Option<{}> {{ match value {{ ",
                name,
                ty,
                name);
    for item in items {
        str1 = format!("{} {} => Some({}::{}),\n",
                       str1,
                       item.value,
                       name,
                       item.name);
    }
    format!("{} _ => None }}}}}}", str1)
}

fn build_enum_vector_type(name: &token::InternedString, ty: &FieldType) -> Result<String, ()> {
    let (base_ty, size) = match *ty {
        FieldType::Scalar(ref base_ty, size) => (base_ty.clone(), size),
        _ => return Err(()),
    };
    let str1 =
        format!("impl<'a> flatbuffers::VectorType<'a> for {} {{ type Item = \
                 Option<{}>;  
             fn inline_size() -> usize {{ {} }} fn \
                 read_next(buffer:&'a [u8]) -> Self::Item {{ \
                   let i = {}::read_next(buffer);
                   {}::from(i) \
                   }} }}",
                name,
                name,
                size,
                base_ty,
                name);
    Ok(str1)
}
