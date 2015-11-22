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


4) add a column for Kotlin for the user types  in the macro, and remember it's index (very important)

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
