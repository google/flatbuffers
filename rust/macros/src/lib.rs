#![crate_type = "dylib"]
#![feature(quote, plugin_registrar, rustc_private)]
extern crate rustc_plugin;
extern crate syntax;
extern crate regex;

use syntax::ast;
use syntax::codemap;
use syntax::print;
use syntax::ext::base::{ExtCtxt, MacResult, DummyResult};
use syntax::parse::token;
use std::ops::Deref;
use rustc_plugin::Registry;

mod table;
mod utils;
mod union;

use self::utils::*;

const EXPECTED_IDENT_TABLE_ETC: &'static str = "Expected one of 'Table', 'Struct', 'Enum' or 'Union'";
const EXPECTED_FAT_ARROW: &'static str = "Expected '=>'";
const EXPECTED_COLON: &'static str = "Expected ':'";
const EXPECTED_TABLE_FIELDS: &'static str = "Expected a list of fields";
const EXPECTED_FIELD_DEF: &'static str = "Expected field";
const INVALID_FIELD_DEF: &'static str = "Invalid field definition";
const EXPECTED_SLOT_INT: &'static str = "Slot must be an integer";
const UNKNOWN_ATTRIBUTE: &'static str = "Unknown attribute";
const EXPECTED_SIZE_INT: &'static str = "Size must be an integer";
const EXPECTED_ENUM_ITEMS: &'static str = "Expected Enum items";
const EXPECTED_UNION_ITEMS: &'static str = "Expected Union items";
const EXPECTED_ENUM_TYPE: &'static str = "Expected one of ubyte, byte, ushort, short, uint, int, ulong, or long";


#[plugin_registrar]
pub fn plugin_registrar(registry: &mut Registry) {
    registry.register_macro("flatbuffers_object", expand)
}

fn expand(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Box<MacResult> {
    if let Ok(res) = expand_impl(cx, sp, ast) {
        res
    } else {
        DummyResult::any(sp)        
    }
}

fn expand_impl(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Box<MacResult>, ()> {
    if ast.len() == 0 {
        cx.span_err(sp, EXPECTED_IDENT_TABLE_ETC);
        return Err(());
    }
    let ident = try!(expect_ident(cx, sp, &ast[0], &["Table", "Struct", "Enum", "Union"], EXPECTED_IDENT_TABLE_ETC));
    if ast.len() < 2 {
        cx.span_err(sp, EXPECTED_FAT_ARROW);
        return Err(());
    }
    try!(consume_fat_arrow(cx, sp, &ast[1],  EXPECTED_FAT_ARROW));
    let ast = &ast[2..];
    match try!(object_type(ident)) {
        ObjectType::Table  =>  expand_table_object(cx, sp, ast),
        ObjectType::Struct =>  expand_struct_object(cx, sp, ast),
        ObjectType::Enum   =>  expand_enum_object(cx, sp, ast),
        ObjectType::Union  =>  expand_union_object(cx, sp, ast),
    }
}


fn object_type(ident: ast::Ident) -> Result<ObjectType,()> {
    match ident.name.as_str().deref() {
        "Table"  => Ok(ObjectType::Table),
        "Struct" => Ok(ObjectType::Struct),
        "Enum"   => Ok(ObjectType::Enum),
        "Union"  => Ok(ObjectType::Union),
        _ => Err(())
    } 
}

fn expand_table_object(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Box<MacResult>, ()> {
    let name = try!(get_name(cx, sp, ast, &format!("Expected a name for the {}", "Table")));
    let attributes = try!(get_attributes(cx, sp, &ast[1..]));
    let ast = if attributes.len() > 0 {
        &ast[2..]
    } else {
        &ast[1..]
    };
    let fields =  try!(get_obj_fields(cx, sp, ast, EXPECTED_TABLE_FIELDS, ObjectType::Table));
    table::build_table_items(cx, name, fields, attributes)
}

fn expand_struct_object(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Box<MacResult>, ()> {
    let name = try!(get_name(cx, sp, ast, &format!("Expected a name for the {}", "Struct")));
    let attributes = try!(get_attributes(cx, sp, &ast[1..]));
    let ast = if attributes.len() > 0 {
        &ast[2..]
    } else {
        &ast[1..]
    };
    let fields =  try!(get_obj_fields(cx, sp, ast, EXPECTED_TABLE_FIELDS, ObjectType::Struct));
    table::build_struct_items(cx, name, fields, attributes)
}

fn expand_enum_object(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Box<MacResult>, ()> {
    let name = try!(get_name(cx, sp, ast, &format!("Expected a name for the {}", "Enum")));
    let items = try!(get_enum_items(cx, sp, &ast[1], EXPECTED_ENUM_ITEMS));
    let ty = try!(get_enum_repr(cx, &ast[3], EXPECTED_ENUM_TYPE));
    union::build_simple_enum(cx, name, ty, items)
}

fn expand_union_object(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Box<MacResult>, ()> {
    let name = try!(get_name(cx, sp, ast, &format!("Expected a name for the {}", "Union")));
    let items = try!(get_enum_items(cx, sp, &ast[1], EXPECTED_UNION_ITEMS));
    union::build_union(cx, name, items)
}

fn get_name(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree], msg: &str) -> Result<token::InternedString, ()> {
    if ast.len() > 0 {
        match ast[0] {
            ast::TokenTree::Token(_, token::Token::Ident(ident)) => {
                return Ok(ident.name.as_str())
            }
            _ => {}
        }
    }
    cx.span_err(sp, msg);
    Err(())
}

fn get_attributes(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree]) -> Result<Vec<ObjAttribute>, ()> {
    if ast.len() == 0 {
        return Ok(vec![])
    }
    match ast[0] {
        ast::TokenTree::Delimited(_, ref delemented) => {
            if delemented.delim == token::DelimToken::Paren {
                return get_obj_attributes(cx, sp, delemented.as_ref())
            }
        }
        _ => {}
    }
    return Ok(vec![])
}

fn get_obj_attributes(cx: &mut ExtCtxt, sp: codemap::Span, ast: &ast::Delimited) -> Result<Vec<ObjAttribute>, ()> {
    let tts = &ast.tts;
    let mut res = Vec::new();
    if tts.len() == 0 {
        return Ok(res);
    }
    let mut i = 0;
    loop {
        if tts.len() < i + 3 {
            cx.span_err(ast.open_span, INVALID_FIELD_DEF);
            return Err(())
        }
        let attribute = try!(expect_ident(cx, tts[i].get_span(), &tts[i], &["size"], EXPECTED_FIELD_DEF));
        let attribute = attribute.name.as_str().deref().to_string();
        try!(consume_colon(cx, sp, &tts[i+1],  EXPECTED_COLON));
        match &*attribute {
            "size" => {
                let lit = try!(get_lit(cx, tts[i+2].get_span(), &tts[i+2], EXPECTED_SIZE_INT));
                if let token::Lit::Integer(size) = lit {
                    res.push( ObjAttribute{
                        name: attribute,
                        value: size.as_str().deref().to_string()
                    } )
                } else {
                    cx.span_err(tts[i+2].get_span(), EXPECTED_SIZE_INT);
                    return Err(())  
                }
            }
            _ => {
                cx.span_warn(ast.open_span, UNKNOWN_ATTRIBUTE);
            }
        }
        if tts.len() >= i + 4 && maybe_comma(&tts[i+3]) {
            i += 4;
        } else {
            i += 3;
        }
        if tts.len() <= i {
            break;
        }
    }
    Ok(res)
}

fn get_obj_fields(cx: &mut ExtCtxt, sp: codemap::Span, ast: &[ast::TokenTree], msg: &str, objty: ObjectType) -> Result<Vec<FieldDef>, ()> {
    if ast.len() == 0 {
        cx.span_warn(sp, msg);
        return Err(())
    }
    match ast[0] {
        ast::TokenTree::Delimited(_, ref delemented) => {
            return get_obj_fields_impl(cx, sp, delemented.as_ref(), objty);
        }
        _ => {}
    }
    cx.span_err(sp, msg);
    Err(())
}

fn get_obj_fields_impl(cx: &mut ExtCtxt, sp: codemap::Span, ast: &ast::Delimited, objty: ObjectType) -> Result<Vec<FieldDef>, ()> {
    let tts = &ast.tts;
    let mut res = Vec::new();
    if tts.len() == 0 {
        return Ok(res);
    }
    let mut i = 0;
    loop {
        if tts.len() < i + 3 {
            cx.span_err(ast.open_span, INVALID_FIELD_DEF);
            return Err(())
        }
        try!(expect_ident(cx, tts[i].get_span(), &tts[i], &["field"], EXPECTED_FIELD_DEF));
        try!(consume_fat_arrow(cx, sp, &tts[i+1],  EXPECTED_FAT_ARROW));
        let def = try!(get_field_def(cx, &tts[i+2], &objty));
        res.push(def);
        if tts.len() >= i + 4 && maybe_comma(&tts[i+3]) {
            i += 4;
        } else {
            i += 3;
        }
        if tts.len() <= i {
            break;
        }
    }
    Ok(res)
}

fn get_field_def(cx: &mut ExtCtxt, ast: &ast::TokenTree, objty: &ObjectType) -> Result<FieldDef, ()> {
    if let &ast::TokenTree::Delimited(_, ref delemented) = ast {
        let fieldef_str = print::pprust::tts_to_string(&delemented.tts);
        return parse_field_names(cx, ast.get_span(), fieldef_str, objty)
    }
    cx.span_err(ast.get_span(), INVALID_FIELD_DEF);
    Err(())
}

fn parse_field_names(cx: &mut ExtCtxt, sp: codemap::Span, ast: String, objty: &ObjectType) -> Result<FieldDef, ()> {
    if ast.len() == 0 {
        cx.span_err(sp, INVALID_FIELD_DEF);
        return Err(())
    }
    let def = FieldDef {
        name: "".to_string(),
        ty: Default::default(),
        slot: "".to_string(),
        default: "".to_string(),
        comments: Vec::new(),
        padding: None
    };
    let mut required = if *objty == ObjectType::Table {
        vec!["default", "name", "slot", "typeOf"] 
    } else {
        vec!["name", "slot", "typeOf"]
    };
    let list = required.clone();
    let iter = ast.split(',');
    let mut error = false;
    let def = iter.fold(def, |mut acc, attr| {
        if error { return acc }
        let mut iter = attr.split('=');
        match iter.next().unwrap().trim() {
            "name" => {
                acc.name = iter.next().unwrap().trim().to_string();
                remove_required(&mut required, "name");
            }
            "typeOf" => {
                let ty = map_ty(iter.next().unwrap().trim().to_string());
                if ty.is_none() {
                    cx.span_err(sp, INVALID_FIELD_DEF);
                    error = true;
                    return acc;
                }
                remove_required(&mut required, "typeOf");
                acc.ty = ty.unwrap();
            }
            "default" => {
                acc.default = iter.next().unwrap().trim().to_string();
                remove_required(&mut required, "default");
            }
            "slot" => {
                let slot = iter.next().unwrap().trim().to_string();
                if slot.parse::<u8>().is_err() {
                    cx.span_err(sp, EXPECTED_SLOT_INT);
                    error = true;
                    return acc
                } 
                acc.slot = slot;
                remove_required(&mut required, "slot");
            }
            "comment" => {
                let comment = iter.next().unwrap().trim().to_string();
                acc.comments.push(comment);
            }
            "padding" => {
                let padding = iter.next().unwrap().trim().to_string();
                acc.padding = Some(padding);
            }
            a => {
                error = true;
                cx.span_err(sp, &format!("{}: {}", UNKNOWN_ATTRIBUTE, a));
            }
        }
        acc
    });
    if error {
        return Err(())
    }
    if required.len() > 0 {
        fn tokens_to_string(tokens: &[&str]) -> String {
            let mut i = tokens.iter();
            let b = i.next()
                     .map_or("".to_string(), |t| t.to_string());
            i.enumerate().fold(b, |mut b, (i, ref a)| {
                if tokens.len() > 2 && i == tokens.len() - 2 {
                    b.push_str(", and ");
                } else if tokens.len() == 2 && i == tokens.len() - 2 {
                    b.push_str(" and ");
                } else {
                    b.push_str(", ");
                }
                b.push_str(&a.to_string());
                b
            })
        }
        let msg = format!("Expected {}, missing {}",
                          tokens_to_string(&list),
                          tokens_to_string(&required));
        cx.span_err(sp, &msg);
        return Err(())
    }
    Ok(def)
}


fn remove_required(required: &mut Vec<&str>, found: &str) {
    if let Ok(i) = required.binary_search(&&found) {
        required.remove(i);
    }
}

fn get_enum_items(cx: &mut ExtCtxt, sp: codemap::Span, ast: &ast::TokenTree, msg: &str) -> Result<Vec<EnumItem>,()> {
    if let &ast::TokenTree::Delimited(_, ref delemented) = ast {
        let fieldef_str = print::pprust::tts_to_string(&delemented.tts);
        return get_enum_items_impl(cx, ast.get_span(), fieldef_str, msg)
    }
    cx.span_err(sp, INVALID_FIELD_DEF);
    Err(())
}

fn get_enum_items_impl(cx: &mut ExtCtxt, sp: codemap::Span, ast: String, msg: &str) -> Result<Vec<EnumItem> ,()> {
    if ast.len() == 0 {
        cx.span_err(sp, msg);
        return Err(())
    }
    let items = Vec::new();
    let iter = ast.split(',');
    let items = iter.fold(items, |mut acc, attr| {
        let mut iter = attr.split('=');
        let enum_name = iter.next().unwrap().trim();
        let enum_value = iter.next().unwrap().trim();
        let item = EnumItem {
            name: enum_name.to_string(),
            value: enum_value.to_string()
        };
        acc.push(item);
        acc
    });
    Ok(items)
}

fn get_enum_repr(cx: &mut ExtCtxt, ast: &ast::TokenTree, msg: &str) -> Result<FieldType, ()> {
    if let &ast::TokenTree::Token(_, token::Token::Ident(ident))= ast {
        let ty = ident.name.as_str();
        if let Some(ty) = map_ty(ty.to_string()) {
            if ty.is_scalar() {
                return Ok(ty)
            }
        }
    }
    cx.span_err(ast.get_span(), msg);
    Err(())
}
