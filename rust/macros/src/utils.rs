use syntax::ast;
use syntax::codemap;
use syntax::ext::base::ExtCtxt;
use syntax::parse::token;

#[derive(Debug, PartialEq, Eq)]
pub enum ObjectType {
    Table,
    Struct,
    Enum,
    Union,
}

#[derive(Debug)]
pub struct ObjAttribute {
    pub name: String,
    pub value: String,
}

#[derive(Debug)]
pub struct FieldDef {
    pub name: String,
    pub ty: FieldType,
    pub slot: String,
    pub default: String,
    pub comments: Vec<String>,
    pub padding: Option<String>
}

#[derive(Debug)]
pub enum FieldType {
    Scalar(String, u8),
    Enum(String, Box<FieldType>),
    Union(String),
    Table(String),
    Vector(Box<FieldType>),
}

#[derive(Debug)]
pub struct EnumItem {
    pub name: String,
    pub value: String,
}

pub fn map_ty(other: String) -> Option<FieldType> {
    let ft = match &*other {
        "byte" => FieldType::Scalar("i8".to_string(), 1),
        "ubyte" => FieldType::Scalar("u8".to_string(), 1),
        "short" => FieldType::Scalar("i16".to_string(), 2),
        "ushort" => FieldType::Scalar("u16".to_string(), 2),
        "int" => FieldType::Scalar("i32".to_string(), 4),
        "uint" => FieldType::Scalar("u32".to_string(), 4),
        "long" => FieldType::Scalar("i64".to_string(), 8),
        "ulong" => FieldType::Scalar("u64".to_string(), 8),
        "float" => FieldType::Scalar("f32".to_string(), 4),
        "double" => FieldType::Scalar("f64".to_string(), 8),
        "bool" => FieldType::Scalar("bool".to_string(), 1),
        "string" => FieldType::Scalar("&str".to_string(), 4),
        _ => {
            let otherc = other.clone();
            let mut chars = otherc.chars();
            if chars.next() == Some('[') && chars.next_back() == Some(']') {
                // vector
                let ty = chars.as_str()
                              .to_string();
                let ty = map_ty(ty);
                if ty.is_none() {
                    return None;
                }
                FieldType::Vector(Box::new(ty.unwrap()))
            } else if other.starts_with("union ") {
                let mut iter = other.split_whitespace();
                let md = iter.nth(1);
                if md.is_none() {
                    return None;
                }
                FieldType::Union(md.unwrap().to_string())
            } else if other.starts_with("enum ") {
                let mut iter = other.split_whitespace();
                let _ = iter.next();
                let md = iter.next();
                let ty = iter.next();
                if md.is_none() || ty.is_none() {
                    return None;
                }
                let ty = map_ty(ty.unwrap().to_string());
                if ty.is_none() {
                    return None;
                }
                FieldType::Enum(md.unwrap().to_string(), Box::new(ty.unwrap()))
            } else {
                FieldType::Table(other)
            }
        }
    };
    Some(ft)
}

impl Default for FieldType {
    fn default() -> FieldType {
        FieldType::Scalar("I8".to_string(), 1)
    }
}

impl FieldType {
    pub fn get_table_accessor(&self, slot: &str, default: &str) -> String {
        match *self {
            FieldType::Scalar(ref ty, _) => scalar_accessor(ty, slot, default),
            FieldType::Enum(ref md, ref ty) => enum_accessor(md, &ty, slot, default),
            FieldType::Union(ref md) => union_accessor(md, slot),
            FieldType::Table(_) => table_accessor(slot),
            FieldType::Vector(ref ty) => vector_accessor(&ty, slot),
        }
    }

    pub fn get_struct_accessor(&self, slot: &str) -> String {
        match *self {
            FieldType::Scalar(ref ty, _) => struct_scalar_accessor(ty, slot),
            FieldType::Enum(ref md, ref ty) => struct_enum_accessor(md, &ty, slot),
            FieldType::Table(_) => struct_table_accessor(slot),
            _ => panic!("Structs do not have Union or Vector fields"),
        }
    }


    pub fn base_type(&self) -> String {
        match *self {
            FieldType::Scalar(ref ty, _) => ty.to_string(),
            FieldType::Table(ref ty) => format!("{}<&[u8]>", ty.to_string()),
            FieldType::Vector(ref ty) => format!("Iter<'a,{}>", ty.base_type()),
            FieldType::Union(ref ty) => ty.to_string(),
            FieldType::Enum(ref md, _) => format!("Option<{}>", md.to_string()),
        }
    }

    pub fn is_scalar(&self) -> bool {
        if let FieldType::Scalar(_,_) = *self {
            true
        } else {
            false
        } 
    }
}

fn enum_accessor(md: &str, ty: &FieldType, slot: &str, default: &str) -> String {
    let fun = ty.get_table_accessor(slot, default);
    format!("let v = {}; {}::from(v)", fun, md)
}

fn struct_enum_accessor(md: &str, ty: &FieldType, slot: &str) -> String {
    let fun = ty.get_struct_accessor(slot);
    format!("let v = {}; {}::from(v)", fun, md)
}

fn union_accessor(md: &str, slot: &str) -> String {
    let fun = format!("self.{}_type();", md.to_lowercase());
    let table = format!("let table =  ($i.0).get_slot_table({});", slot);
    format!("{} {} {}::new(table, ty)", fun, table, md.to_lowercase())
}

fn table_accessor(slot: &str) -> String {
    let fun = format!("let t = (self.0).get_slot_table({})", slot);
    format!("{} if t.is_some() {{ return t.unwrap().into(); }} None",
            fun)
}

fn struct_table_accessor(slot: &str) -> String {
    format!("(self.0).get_struct({})", slot)
}

fn vector_accessor(ty: &FieldType, slot: &str) -> String {
    let ty = ty.base_type();
    format!("(self.0).get_slot_vector::<{}>({})", ty, slot)
}

fn scalar_accessor(ty: &str, slot: &str, default: &str) -> String {
    match &*ty {
        "i8" => format!("(self.0).get_slot_i8({},{})", slot, default),
        "u8" => format!("(self.0).get_slot_u8({},{})", slot, default),
        "i16" => format!("(self.0).get_slot_i16({},{})", slot, default),
        "u16" => format!("(self.0).get_slot_u16({},{})", slot, default),
        "i32" => format!("(self.0).get_slot_i32({},{})", slot, default),
        "u32" => format!("(self.0).get_slot_u32({},{})", slot, default),
        "i64" => format!("(self.0).get_slot_i64({},{})", slot, default),
        "u64" => format!("(self.0).get_slot_u64({},{})", slot, default),
        "f32" => format!("(self.0).get_slot_f32({},{})", slot, default),
        "f64" => format!("(self.0).get_slot_f64({},{})", slot, default),
        "bool" => format!("(self.0).get_slot_bool({},{})", slot, default),
        "&str" => format!("(self.0).get_slot_str({},{})", slot, default),
        otherwise => panic!("Unknow scalar type {}", otherwise),
    }
}

fn struct_scalar_accessor(ty: &str, slot: &str) -> String {
    match &*ty {
        "i8" => format!("(self.0).get_i8({})", slot),
        "u8" => format!("(self.0).get_u8({})", slot),
        "i16" => format!("(self.0).get_i16({})", slot),
        "u16" => format!("(self.0).get_u16({})", slot),
        "i32" => format!("(self.0).get_i32({})", slot),
        "u32" => format!("(self.0).get_u32({})", slot),
        "i64" => format!("(self.0).get_i64({})", slot),
        "u64" => format!("(self.0).get_u64({})", slot),
        "f32" => format!("(self.0).get_f32({})", slot),
        "f64" => format!("(self.0).get_f64({})", slot),
        "bool" => format!("(self.0).get_bool({})", slot),
        "&str" => format!("(self.0).get_str({})", slot),
        otherwise => panic!("Unknow scalar type {}", otherwise),
    }
}

pub fn find_attribute(name: &str, attributes: &[ObjAttribute]) -> Option<String> {
    for attr in attributes {
        if attr.name == name {
            return Some(attr.value.clone());
        }
    }
    None
}

pub fn consume_fat_arrow(cx: &mut ExtCtxt,
                         sp: codemap::Span,
                         ast: &ast::TokenTree,
                         msg: &str)
                         -> Result<(), ()> {
    match *ast {
        ast::TokenTree::Token(_, token::Token::FatArrow) => return Ok(()),
        _ => {}
    }
    cx.span_err(sp, msg);
    Err(())
}

pub fn consume_colon(cx: &mut ExtCtxt,
                     sp: codemap::Span,
                     ast: &ast::TokenTree,
                     msg: &str)
                     -> Result<(), ()> {
    match *ast {
        ast::TokenTree::Token(_, token::Token::Colon) => return Ok(()),
        _ => {}
    }
    cx.span_err(sp, msg);
    Err(())
}


pub fn maybe_comma(ast: &ast::TokenTree) -> bool {
    match *ast {
        ast::TokenTree::Token(_, token::Token::Comma) => return true,
        _ => {}
    }
    false
}

pub fn get_lit(cx: &mut ExtCtxt,
               sp: codemap::Span,
               ast: &ast::TokenTree,
               msg: &str)
               -> Result<token::Lit, ()> {
    match ast {
        &ast::TokenTree::Token(_, token::Token::Literal(lit, _)) => return Ok(lit),
        _ => {
            cx.span_err(sp, msg);
            return Err(());
        }
    }
}

pub fn expect_ident(cx: &mut ExtCtxt,
                    sp: codemap::Span,
                    ast: &ast::TokenTree,
                    name: &[&str],
                    msg: &str)
                    -> Result<ast::Ident, ()> {
    match ast {
        &ast::TokenTree::Token(_, token::Token::Ident(ident)) => {
            let res = name.iter().any(|x| *x == ident.name.as_str());
            if res {
                return Ok(ident);
            }
        }
        _ => {}
    }
    cx.span_err(sp, msg);
    Err(())
}
