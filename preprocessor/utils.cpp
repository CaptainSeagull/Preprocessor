/*===================================================================================================
  File:                    utils.cpp
  Author:                  Jonathan Livingstone
  Email:                   seagull127@ymail.com
  Licence:                 Public Domain
                           No Warranty is offered or implied about the reliability,
                           suitability, or usability
                           The use of this code is at your own risk
                           Anyone can use this code, modify it, sell it to terrorists, etc.
  ===================================================================================================*/

#define MEM_ROOT_FILE
#include "utils.h"
#include "platform.h"

#define STB_SPRINTF_NOFLOAT
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

//
// Error stuff.
//
internal Error global_errors[32];
internal Int global_error_count = 0;

Char const *ErrorTypeToString(ErrorType e) {
    Char const *res = 0;

#define ERROR_TYPE_TO_STRING(err) err: { res = #err; } break

    switch(e) {
        case ERROR_TYPE_TO_STRING(ErrorType_ran_out_of_memory);
        case ERROR_TYPE_TO_STRING(ErrorType_assert_failed);
        case ERROR_TYPE_TO_STRING(ErrorType_no_parameters);
        case ERROR_TYPE_TO_STRING(ErrorType_cannot_find_file);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_write_to_disk);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_load_file);
        case ERROR_TYPE_TO_STRING(ErrorType_no_files_pass_in);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_find_mallocd_ptr);
        case ERROR_TYPE_TO_STRING(ErrorType_memory_not_freed);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_find_struct);
        case ERROR_TYPE_TO_STRING(ErrorType_unknown_token_found);
        case ERROR_TYPE_TO_STRING(ErrorType_failed_to_parse_enum);
        case ERROR_TYPE_TO_STRING(ErrorType_failed_parsing_variable);
        case ERROR_TYPE_TO_STRING(ErrorType_failed_to_find_size_of_array);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_detect_struct_name);
        case ERROR_TYPE_TO_STRING(ErrorType_did_not_write_entire_file);
        case ERROR_TYPE_TO_STRING(ErrorType_did_not_read_entire_file);
        case ERROR_TYPE_TO_STRING(ErrorType_could_not_create_directory);
        case ERROR_TYPE_TO_STRING(ErrorType_incorrect_number_of_members_for_struct);
        case ERROR_TYPE_TO_STRING(ErrorType_incorrect_struct_name);
        case ERROR_TYPE_TO_STRING(ErrorType_incorrect_number_of_base_structs);

        default: assert(0); break;
    }

    if(res) {
        Int offset = string_length("ErrorType_");
        res += offset;
    }

#undef ERROR_TYPE_TO_STRING

    return(res);
}

Void push_error_(ErrorType type, Char const *guid) {
    if(global_error_count + 1 < array_count(global_errors)) {
        Error *e = global_errors + global_error_count++;

        e->type = type;
        e->guid = guid;
    }
}

Bool print_errors() {
    Bool res = false;

    if(global_error_count) {
        res = true;

        // TODO(Jonny): Write errors to disk?
        system_write_to_stderr("\nPreprocessor errors:\n");
        for(Int i = 0; (i < global_error_count); ++i) {
            Char buffer[256] = {};
            stbsp_snprintf(buffer, array_count(buffer), "%s %s\n\n",
                           global_errors[i].guid, ErrorTypeToString(global_errors[i].type));
            system_write_to_stderr(buffer);
        }

        Char buffer2[256] = {};
        stbsp_snprintf(buffer2, array_count(buffer2), "Preprocessor finished with %d error(s).\n\n\n", global_error_count);
        system_write_to_stderr(buffer2);
    }

    return(res);
}

//
// Scratch memory.
//
// A quick-to-access temp region of memory. Should be frequently cleared.
internal Int scratch_memory_index = 0;
internal Void *global_scratch_memory = 0;

Void *push_scratch_memory(Int size/*= scratch_memory_size*/) {
    if(!global_scratch_memory) {
        global_scratch_memory = system_alloc(Byte, scratch_memory_size + 1);
        zero(global_scratch_memory, scratch_memory_size + 1);
    }

    Void *res = 0;
    if(global_scratch_memory) {
        assert(scratch_memory_size + 1 > scratch_memory_index + size);
        res = cast(Byte *)global_scratch_memory + scratch_memory_index;
        scratch_memory_index += size;
    }

    return(res);
}

Void clear_scratch_memory(void) {
    if(global_scratch_memory) {
        zero(global_scratch_memory, scratch_memory_index);
        scratch_memory_index = 0;
    }
}

Void free_scratch_memory() {
    if(global_scratch_memory) {
        system_free(global_scratch_memory);
    }
}

//
// Strings.
//
String create_string(Char const *str, Int len/*= 0*/) {
    String res = {str, (len) ? len : string_length(str)};

    return(res);
}

Int string_length(Char const *str) {
    Int res = 0;

    if(str) {
        while(*str++) {
            ++res;
        }
    }

    return(res);
}

Bool string_concat(Char *dest, Int len, Char const *a, Int a_len, Char const *b, Int b_len) {
    Bool res = false;
    Int i;

    if(len > a_len + b_len) {
        for(i = 0; (i < a_len); ++i) *dest++ = *a++;
        for(i = 0; (i < b_len); ++i) *dest++ = *b++;

        res = true;
    }

    return(res);
}

Bool string_compare(Char const *a, Char const *b, Int len) {
    for(Int i = 0; (i < len); ++i, ++a, ++b) {
        if(*a != *b) {
            return(false);
        }
    }

    return(true);
}

Bool string_compare(Char const *a, Char const *b) {
    for(;; ++a, ++b) {
        if((*a == 0) && (*b == 0)) return(true);
        else if(*a != *b)          return(false);
    }
}

Void string_copy(Char *dest, Char const *src) {
    while(*src) {
        *dest = *src;
        ++dest;
        ++src;
    }
}

Bool string_compare(String a, String b) {
    Bool res = false;

    if(a.len == b.len) {
        res = true;

        for(Int i = 0; (i < a.len); ++i) {
            if(a.e[i] != b.e[i]) {
                res = false;
                break;
            }
        }
    }

    return(res);
}

Bool string_compare_array(String *a, String *b, Int cnt) {
    Bool res = true;
    for(Int i = 0; (i < cnt); ++i) {
        if(!string_compare(a[i], b[i])) {
            res = false;
            break;
        }
    }

    return(res);
}

Bool string_contains(String str, Char const *target) {
    Int target_len = string_length(target);

    for(Int i = 0; (i < str.len); ++i) {
        if(str.e[i] == target[0]) {
            for(int j = 0; (j < target_len); ++j) {
                if(str.e[i + j] != target[j]) {
                    break;
                }

                if(j == (target_len - 1)) {
                    return(true);
                }
            }
        }
    }

    return(false);
}

Bool string_contains(Char const *str, Char const *target) {
    String s = {str, string_length(str)};
    return(string_contains(s, target));
}

Int string_contains_pos(Char const *str, Char const *target) {
    Int target_len = string_length(target);
    Int str_len = string_length(str);

    for(Int i = 0; (i < str_len); ++i) {
        if(str[i] == target[0]) {
            for(int j = 0; (j < target_len); ++j) {
                if(str[i + j] != target[j]) {
                    break;
                }

                if(j == (target_len - 1)) {
                    return(i);
                }
            }
        }
    }

    return(-1);
}


//
// Stuff
//
ResultInt char_to_int(Char c) {
    ResultInt res = {};
    switch(c) {
        case '0': { res.e = 0; res.success = true; } break;
        case '1': { res.e = 1; res.success = true; } break;
        case '2': { res.e = 2; res.success = true; } break;
        case '3': { res.e = 3; res.success = true; } break;
        case '4': { res.e = 4; res.success = true; } break;
        case '5': { res.e = 5; res.success = true; } break;
        case '6': { res.e = 6; res.success = true; } break;
        case '7': { res.e = 7; res.success = true; } break;
        case '8': { res.e = 8; res.success = true; } break;
        case '9': { res.e = 9; res.success = true; } break;
    }

    return(res);
}

ResultInt string_to_int(String str) {
    ResultInt res = {};

    for(Int i = 0; (i < str.len); ++i) {
        ResultInt temp_int = char_to_int(str.e[i]);
        if(!temp_int.success) {
            break;
        }

        res.e *= 10;
        res.e += temp_int.e;

        if(i == (str.len - 1)) {
            res.success = true;
        }
    }

    return(res);
}

ResultInt string_to_int(Char *str) {
    String string;
    string.e = str;
    string.len = string_length(str);
    ResultInt res = string_to_int(string);

    return(res);
}

Bool is_in_string_array(String target, String *arr, Int arr_cnt) {
    Bool res = false;
    for(int i = 0; (i < arr_cnt); ++i) {
        if(string_compare(target, arr[i])) {
            res = true;
            break;
        }
    }

    return(res);
}

Uint32 safe_truncate_size_64(Uint64 v) {
    assert(v <= 0xFFFFFFFF);
    Uint32 res = cast(Uint32)v;

    return(res);
}

Variable create_variable(Char const *type, Char const *name, Int ptr/*= 0*/, Int array_count/*= 1*/) {
    Variable res;
    res.type = create_string(type);
    res.name = create_string(name);
    res.ptr = ptr;
    res.array_count = array_count;

    return(res);
}

Bool compare_variable(Variable a, Variable b) {
    Bool res = true;

    if(!string_compare(a.type, b.type))      { res = false; }
    else if(!string_compare(a.name, b.name)) { res = false; }
    else if(a.ptr != b.ptr)                  { res = false; }
    else if(a.array_count != b.array_count)  { res = false; }

    return(res);
}

Bool compare_variable_array(Variable *a, Variable *b, Int count) {
    for(Int i = 0; (i < count); ++i) {
        if(!compare_variable(a[i], b[i])) {
            return(false);
        }
    }

    return(true);
}

Char to_caps(Char c) {
    Char res = c;
    if((c >= 'a') && (c <= 'z')) {
        res -= 32;
    }

    return(res);
}

Void copy(Void *dest, Void *src, PtrSize size) {
    Byte *dest8 = cast(Byte *)dest;
    Byte *src8 = cast(Byte *)src;

    for(Int i = 0; (i < size); ++i, ++dest8, ++src8) {
        *dest8 = *src8;
    }
}

Void set(Void *dest, Byte v, PtrSize n) {
    Byte *dest8 = cast(Byte *)dest;
    for(Int i = 0; (i < n); ++i, ++dest8) {
        *dest8 = cast(Byte)v;
    }
}
