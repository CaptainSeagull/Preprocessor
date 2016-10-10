#if !defined(GENERATED_H)

#include <stdio.h>

enum MetaType {
    meta_type_char,
    meta_type_short,
    meta_type_int,
    meta_type_long,
    meta_type_float,
    meta_type_double,
    meta_type_Position,
    meta_type_test,
    meta_type_FooOne,
    meta_type_SomeStruct,
};


struct MemberDefinition {
    MetaType type;
    char *name;
    size_t offset;
    int is_ptr;
    unsigned arr_size;
};

#define get_num_of_members(type) num_members_for_##type


//
// Struct meta data.
//
// Meta Data for: Position
extern MemberDefinition members_of_Position[];
static const size_t num_members_for_Position = 2;

// Meta Data for: test
extern MemberDefinition members_of_test[];
static const size_t num_members_for_test = 6;

// Meta Data for: FooOne
extern MemberDefinition members_of_FooOne[];
static const size_t num_members_for_FooOne = 3;

// Meta Data for: SomeStruct
extern MemberDefinition members_of_SomeStruct[];
static const size_t num_members_for_SomeStruct = 2;


//
// Forward declared structs.
//
struct Position;
struct test;
struct FooOne;
struct SomeStruct;

//
// Forward declared enums.
//
enum struct FooBar : short;
enum BarFoo : long;

//
// Forward declared unions.
//
union V2;

//
// Forward declared functions.
//
static BarFoo foo(char *str);
void some_function(SomeStruct *some_struct);

//
// Function meta data.
//
struct Variable {
    char *ret_type;
    char *name;
};

unsigned const MAX_NUMBER_OF_PARAMS = 32;
struct FunctionMetaData {
    char *linkage;
    char *ret_type;
    char *name;
    unsigned param_count;
    Variable params[MAX_NUMBER_OF_PARAMS];
};
#define get_func_meta_data(func) function_data_##func
extern FunctionMetaData function_data_foo;
extern FunctionMetaData function_data_some_function;

// size_t serialize_struct(void *var, type VariableType, char *buffer, size_t buf_size);
#define serialize_struct(var, type, buffer, buf_size) serialize_struct_(var, type, 0, buffer, buf_size, 0)
#define serialize_struct_(var, type, indent, buffer, buf_size, bytes_written) serialize_struct__((void *)&var, members_of_##type, indent, get_num_of_members(type), buffer, buf_size, bytes_written)
size_t serialize_struct__(void *var, MemberDefinition members_of_Something[], unsigned indent, size_t num_members, char *buffer, size_t buf_size, size_t bytes_written);

#define GENERATED_H
#endif // !defined(GENERATED_H)