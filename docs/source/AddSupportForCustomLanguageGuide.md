# Adding support for a custom language

We will illustrate the steps necessary to add support for a custom language, with snipets for the Kotlin language

## src/flatc.cpp

1) in the generators array, add a Generator for Kotlin

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
const Generator generators[] = {
...  
{ flatbuffers::GenerateKotlin,  "-k", "Kotlin",
    flatbuffers::GeneratorOptions::kKotlin,
    "Generate Kotlin classes for tables/structs",
    flatbuffers::GeneralMakeRule },
...
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## include/flatbuffers/idl.h

2) At the end, add an extern declaration for the Kotlin code generator

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Generate Kotlin files from the definitions in the Parser object.
// See idl_gen_kotlin.cpp.
extern bool GenerateKotlin(const Parser &parser,
                       const std::string &path,
                       const std::string &file_name,
                       const GeneratorOptions &opts);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3) in the GeneratorOptions struct declaration, add a Kotlin value to the Language enum

struct GeneratorOptions {
...
  // Possible options for the more general generator below.
  enum Language { kJava, kKotlin, kCSharp, kGo, kMAX };
}


4) add a column (and remember it's index) for Kotlin in the following macro, for the types that will be used in Kotlin to represent 
the theorical types of the first column.
Kotlin doesn't have unsigned types, so we used a widened signed type to represent those 
(Int will represent uChar and uShort, Long will represent uInt, slight issue : Long will represent uLong)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    #define FLATBUFFERS_GEN_TYPES_SCALAR(TD)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## include/flatbuffers/*.h + src/*.cpp 

5) in all files, modify the macros calls so that it now uses the new column you inserted : 
for Kotlin we inserted a column at index 4 (KTYPE), after the column for Java (JTYPE) and before the column for Go (GTYPE)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sometimes, you'll have to insert KTYPE at other places, like (in idl_gen_general.cpp) : 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static std::string GenTypeBasic(const LanguageParameters &lang,
                                const Type &type) {
  static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE) \
        #JTYPE, #KTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## src/idl_gen_kotlin.cpp 

6) create a src/idl_gen_kotlin.cpp file and implement the function you added at step 1 in the generators

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
namespace flatbuffers{
namespace kotlin {

bool GenerateKotlin(const Parser &parser,, const std::string &path,, const std::string & /*file_name */,const GeneratorOptions & /*opts*/) {
                    ...
                    return true
                }
}  // namespace kotlin
}  // namespace flatbuffers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

7) At this point, flatc should build. Look into idl_gen_kotlin.cpp for snipets to reuse stuff in idl_gen_general.cpp or to implement flatbuffers features

8) tests...

9) typee related functions

GenTypeGet returns the proper Type for what you get from source (storage) : 
offsets (Int) for object types (String, Union, Table, Struct)
Bit-thin type for scalars 

// GentypeBasic returns the type for scalars defined at step 4
// notice how the cSharp people are making this function return generics that depends on the pointer type for structs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
std::string GenTypeBasic(const LanguageParameters &lang, const Type &type) {
  static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE) \
        #JTYPE, #KTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };

  if(lang.language == GeneratorOptions::kCSharp && type.base_type == BASE_TYPE_STRUCT) {
    return "Offset<" + type.struct_def->name + ">";
  }

  return gtypename[type.base_type * GeneratorOptions::kMAX + lang.language];
}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BASE_TYPE_STRING -> lang.string_type
BASE_TYPE_VECTOR -> GenTypeGet(lang, type.VectorType())  (element type)
BASE_TYPE_STRUCT -> type.struct_def->name;
BASE_TYPE_UNION -> "Table"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static std::string GenTypePointer(const LanguageParameters &lang, const Type &type) 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-> GenTypeBasic & GenTypeGet on pointer types
Same output on everything exept for BASE_TYPE_VECTOR (Int instead of  Element type)

used in enum declaration 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.}
"public enum class " + enum_def.name + "(val value: " + GenTypeGet(kotlinLang, enum_def.underlying_type) + ") {\n"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
inline bool IsScalar (BaseType t) { return t >= BASE_TYPE_UTYPE && t <= BASE_TYPE_DOUBLE; }

std::string GenTypeGet(const LanguageParameters &lang, const Type &type) {
  return IsScalar(type.base_type) ? GenTypeBasic(lang, type) : GenTypePointer(lang, type);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



-> widdens the unsigned types (for the user)
-> for vectors : vectorElement = true -> return a widened element type
                        vectorElement = false -> returns the same vector type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
/ Find the destination type the user wants to receive the value in (e.g.
// one size higher signed types for unsigned serialized values in Java).
// removed static to allow reuse in external generator files
Type DestinationType(const LanguageParameters &lang, const Type &type,
                            bool vectorelem) {
  if (lang.language != GeneratorOptions::kJava && lang.language != GeneratorOptions::kKotlin) return type;
  switch (type.base_type) {
  	  // We use int for both uchar/ushort, since that generally means less casting
    // than using short for uchar.
    case BASE_TYPE_UCHAR:  return Type(BASE_TYPE_INT);
    case BASE_TYPE_USHORT: return Type(BASE_TYPE_INT);
    case BASE_TYPE_UINT:   return Type(BASE_TYPE_LONG);
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationType(lang, type.VectorType(), vectorelem);
      // else fall thru:
    default: return type;
  }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// returns enum names for kotlin/csharp enums and basic types otherwise (not widened types)
// -> despite the name, you don't want to use that for user api with unsigned types (or look at the addYourStuff table metods)
// -> not much used 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Generate type to be used in user-facing API
// removed static to allow reuse in the kotlin external generator
std::string GenTypeForUser(const LanguageParameters &lang, const Type &type) {
  if (lang.language == GeneratorOptions::kCSharp) {
    if (type.enum_def != nullptr &&
          type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  }
  if (lang.language == GeneratorOptions::kKotlin) {
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  }
  return GenTypeBasic(lang, type);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// this is used !
// because it widdens the type first and then does the enum trick
// maybee we can use that to return types form methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
GenTypeForUser(kotlinLang, DestinationType(kotlinLang, field.value.type, false))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




// does a generic trick for unions 
// on top of the previous methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Generate destination type name
// removed static for reuse in kotlin code generator
std::string GenTypeNameDest(const LanguageParameters &lang, const Type &type)
{
  if (lang.language == GeneratorOptions::kCSharp) {
    // C# enums are represented by themselves
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION)
      return type.enum_def->name;

    // Unions in C# use a generic Table-derived type for better type safety
    if (type.base_type == BASE_TYPE_UNION)
      return "TTable";
  }
  if (lang.language == GeneratorOptions::kKotlin) {
    // Kotlin enums are represented by themselves
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;

    // Unions in Kotlin use a generic Table-derived type 
    if (type.base_type == BASE_TYPE_UNION) return "Table";
  }
  
  // default behavior
  return GenTypeGet(lang, DestinationType(lang, type, true));
}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
