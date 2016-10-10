/*===================================================================================================
  File:                    preprocessor.cpp
  Author:                  Jonathan Livingstone
  Email:                   seagull127@ymail.com
  Licence:                 Public Domain
                           No Warranty is offered or implied about the reliability,
                           suitability, or usability
                           The use of this code is at your own risk
                           Anyone can use this code, modify it, sell it to terrorists, etc.
  ===================================================================================================*/

#include "platform.h"

internal U32
get_digit_count(S32 value)
{
    U32 res = 1;
    while((value /= 10) > 0) {
        ++res;
    }

    return(res);
}

internal void
copy_int(char *dest, S32 value, S32 start, U32 count)
{
    assert(dest);

    S32 end = start + count;
    for(S32 index = end - 1; (index >= start); --index, value /= 10) {
        *(dest + index) = cast(char)(value % 10 + 48);
    }

    dest[end] = 0;
}

internal char *
int_to_string(S32 value, char *buffer)
{
    assert(buffer);

    B32 is_neg = (value < 0);
    S32 abs_value = (is_neg) ? -value : value;
    U32 num_digits = get_digit_count(abs_value);
    if(is_neg) {
        char *ptr = buffer;
        *ptr = '-';
        copy_int(ptr, abs_value, 1, num_digits);
    } else {
        char *ptr = buffer;
        copy_int(ptr, abs_value, 0, num_digits);
    }

    return(buffer);
}

internal char *
float_to_string(F32 value, U32 dec_accuracy, char *buf)
{
    assert(buf);

    B32 is_neg = (value < 0);
    F32 abs_value = (is_neg) ? -value : value;

    S32 mul = 1;
    for(U32 dec_index = 0; (dec_index < dec_accuracy); ++dec_index) {
        mul *= 10;
    }

    S32 num_as_whole = cast(S32)(abs_value * mul);
    S32 num_before_dec = num_as_whole / mul;
    S32 num_after_dec = num_as_whole % mul;
    S32 digit_count_before_dec = get_digit_count(num_before_dec);
    S32 digit_count_after_dec = get_digit_count(num_after_dec);

    if (is_neg) {
        char *ptr = buf;
        *ptr = '-';
        copy_int(ptr, num_before_dec, 1, digit_count_before_dec);
        *(ptr + digit_count_before_dec + 1) = '.';
        copy_int(ptr, num_after_dec, digit_count_before_dec + 2, digit_count_after_dec);
    } else {
        char *ptr = buf;
        copy_int(ptr, num_before_dec, 0, digit_count_before_dec);
        *(ptr + digit_count_before_dec) = '.';
        copy_int(ptr, num_after_dec, digit_count_before_dec + 1, digit_count_after_dec);
    }

    return(buf);
}

internal char *
bool_to_string(B32 boolean)
{
    char *res = cast(char *)((boolean) ? "true" : "false");

    return(res);
}

internal B32
is_numeric(char input)
{
    B32 res = ((input >= '0') && (input <= '9'));

    return(res);
}

// Supports %s, %d, %u, %c, %b and %f. %S for string with length.
internal U32
format_string_varargs(char *buf, PtrSize buf_len, char const *format, va_list args)
{
    assert((buf) && (format));

    char *dest = cast(char *)buf;
    U32 bytes_written = 0;
    U32 float_precision = 1;
    U32 next_fractional_digit = float_precision;

    while (*format) {
        char temp_buffer[1024] = {};
        char c = *format;
        if (c == '%') {
            char type = format[1];
            if (type == 'c') {
                *dest++ = cast(char)va_arg(args, int);
                bytes_written++;
            } else {
                if (is_numeric(type)) {
                    next_fractional_digit = type - 48;
                    assert(format[2] == 'f');
                    type = 'f';
                    format++;
                }

                char *replacement = 0;
                switch (type) {
                    case 'b': {
                        int boolean = va_arg(args, int);
                        replacement = bool_to_string(boolean != 0);
                    } break;

                    case 's': {
                        char *str = va_arg(args, char *);
                        replacement = str;
                    } break;

                    case 'd': {
                        S32 s32 = va_arg(args, S32);
                        char temp_buffer[1024] = {};
                        replacement = int_to_string(s32, temp_buffer);
                    } break;

                    case 'u': {
                        U32 u32 = va_arg(args, S32);
                        char temp_buffer[1024] = {};
                        replacement = int_to_string(u32, temp_buffer);
                    } break;

                    case 'f': {
                        F32 float_num = cast(F32)va_arg(args, F64);
                        char temp_buffer[1024] = {};
                        replacement = float_to_string(float_num, next_fractional_digit, temp_buffer);
                        next_fractional_digit = float_precision;
                    } break;

                    case 'S': {
                        S32 len = va_arg(args, S32);
                        char *str = va_arg(args, char *);
                        char *at = temp_buffer;
                        for(S32 str_index = 0; (str_index < len); ++str_index, ++at) {
                            *at = str[str_index];
                        }

                        *at = 0;

                        replacement = temp_buffer;
                    } break;

                    case '%': {
                        temp_buffer[0] = '%';
                        replacement = temp_buffer;
                    }
                }

                if(replacement) {
                    while (*replacement) {
                        *dest++ = *replacement++;
                        bytes_written++;
                    }
                }
            }

            format++;
        } else {
            *dest++ = c;
            bytes_written++;
        }

        format++;
    }

    *dest = 0;

    assert(bytes_written < buf_len);

    return(bytes_written);
}

internal U32
format_string(char *buf, PtrSize buf_len, char const *format, ...)
{
    va_list args;
    va_start(args, format);
    U32 bytes_written = format_string_varargs(buf, buf_len, format, args);
    assert((bytes_written > 0) && (bytes_written < buf_len));
    va_end(args);

    return(bytes_written);
}

struct OutputBuffer {
    char *buffer;
    PtrSize index;
    PtrSize size;
};

#define write_to_output_buffer(ob, format, ...) write_to_output_buffer_(ob, cast(char *)format, ##__VA_ARGS__)
internal void
write_to_output_buffer_(OutputBuffer *ob, char *format, ...)
{
    assert(((ob) && (ob->buffer) && (ob->size > 0) && (ob->index < ob->size)) && (format));

    va_list args;
    va_start(args, format);
    ob->index += format_string_varargs(ob->buffer + ob->index, ob->size - ob->index, format, args);
    va_end(args);
}

internal OutputBuffer
create_output_buffer(PtrSize size, Memory *memory)
{
    assert((size > 0) && (memory));

    OutputBuffer res = {};
    res.buffer = cast(char *)push_permanent_memory(memory, size);
    res.size = size;

    return(res);
}

enum struct TokenType {
    unknown,

    open_paren,
    close_paren,
    colon,
    close_param,
    semi_colon,
    asterisk,
    open_bracket,
    close_bracket,
    open_brace,
    close_brace,
    hash,
    equals,
    comma,
    tilde,
    period,
    var_args,

    number,
    identifier,
    string,

    end_of_stream,
};

struct Token {
    char *e;
    PtrSize len;

    TokenType type;
};

struct Tokenizer {
    char *at;
};

struct String {
    char *e;
    PtrSize len;
};

internal String
token_to_string(Token token)
{
    assert(token.type != TokenType::unknown);

    String res = {};
    res.e = token.e;
    res.len = token.len;

    return(res);
}

internal char *
token_to_string(Token token, char *buffer, PtrSize size)
{
    assert(size >= token.len);

    zero_memory_block(buffer, size);
    for(U32 str_index = 0; (str_index < token.len); ++str_index) {
        buffer[str_index] = token.e[str_index];
    }

    return(buffer);
}

internal U32
string_length(char *str)
{
    assert(str);

    U32 res = 0;
    while(*str) {
        ++res;
        ++str;
    }

    return(res);
}

internal B32
string_compare(char *a, char *b, U32 len)
{
    assert((a) && (b) && (len > 0));

    B32 res = true;

    for(U32 string_index = 0; (string_index < len); ++string_index) {
        if(a[string_index] != b[string_index]) {
            res = false;
            break; // while
        }
    }

    return(res);
}

internal void
eat_whitespace(Tokenizer *tokenizer)
{
    assert(tokenizer);

    for(;;) {
        if(is_whitespace(tokenizer->at[0])) { // Whitespace
            ++tokenizer->at;
        } else if((tokenizer->at[0] == '/') && (tokenizer->at[1] == '/')) { // C++ comments.
            tokenizer->at += 2;
            while((tokenizer->at[0]) && (!is_end_of_line(tokenizer->at[0]))) {
                ++tokenizer->at;
            }
        } else if((tokenizer->at[0] == '/') && (tokenizer->at[1] == '*')) { // C comments.
            tokenizer->at += 2;
            while((tokenizer->at[0] != '\0') && !((tokenizer->at[0] == '*') && (tokenizer->at[1] == '/'))) {
                ++tokenizer->at;
            }

            if(tokenizer->at[0] == '*') {
                tokenizer->at += 2;
            }
        } else if(tokenizer->at[0] == '#') { // #if 0 blocks.
            char *hash_if_zero = "#if 0";
            U32 hash_if_zero_length = string_length(hash_if_zero);

            char *hash_if_one = "#if 1";
            U32 hash_if_one_length = string_length(hash_if_one);

            if(string_compare(tokenizer->at, hash_if_zero, hash_if_zero_length)) {
                tokenizer->at += hash_if_zero_length;

                char *hash_end_if = "#endif";
                U32 hash_end_if_length = string_length(hash_end_if);

                U32 level = 0;
                while(++tokenizer->at) {
                    if(tokenizer->at[0] == '#') {
                        if((tokenizer->at[1] == 'i') && (tokenizer->at[2] == 'f')) {
                            ++level;

                        } else if(string_compare(tokenizer->at, hash_end_if, hash_end_if_length)) {
                            if(level != 0) {
                                --level;
                            } else {
                                tokenizer->at += hash_end_if_length;
                                eat_whitespace(tokenizer);

                                break; // for
                            }
                        }
                    }
                }
            } else if(string_compare(tokenizer->at, hash_if_one, hash_if_one_length)) { // #if 1 #else blocks.
                tokenizer->at += hash_if_zero_length;

                char *hash_else = "#else";
                U32 hash_else_length = string_length(hash_else);

                char *hash_end_if = "#endif";
                U32 hash_end_if_length = string_length(hash_end_if);

                U32 level = 0;
                while(++tokenizer->at) {
                    if(tokenizer->at[0] == '#') {
                        if((tokenizer->at[1] == 'i') && (tokenizer->at[2] == 'f')) {
                            ++level;
                        } else if(string_compare(tokenizer->at, hash_end_if, hash_end_if_length)) {
                            if(level != 0) {
                                --level;
                            } else {
                                tokenizer->at += hash_end_if_length;
                                break; // for
                            }
                        } else if(string_compare(tokenizer->at, hash_else, hash_else_length)) {
                            if(level == 0) {
                                tokenizer->at += hash_else_length;
                                U32 Level2 = 0;

                                while(++tokenizer->at) {
                                    if(tokenizer->at[0] == '#') {
                                        if((tokenizer->at[1] == 'i') && (tokenizer->at[2] == 'f')) {
                                            ++Level2;

                                        } else if(string_compare(tokenizer->at, hash_end_if, hash_end_if_length)) {
                                            if(Level2 != 0) {
                                                --Level2;
                                            } else {
                                                tokenizer->at += hash_end_if_length;
                                                eat_whitespace(tokenizer);

                                                break; // for
                                            }
                                        }
                                    }

                                    tokenizer->at[0] = ' ';
                                }

                                break; // for
                            }
                        }
                    }
                }
            }

            break; // for
        } else {
            break; // for
        }
    }
}

internal B32
is_alphabetical(char c)
{
    B32 res = (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));

    return(res);
}

internal B32
is_num(char c)
{
    B32 res = ((c >= '0') && (c <= '9'));

    return(res);
}

internal void
parse_number(Tokenizer *tokenizer)
{
    assert(tokenizer);

    // TODO(Jonny): Implement.
}

internal Token get_token(Tokenizer *tokenizer); // Because C++...
internal void
eat_tokens(Tokenizer *tokenizer, U32 num_tokens_to_eat)
{
    assert(tokenizer);

    for(U32 token_index = 0; (token_index < num_tokens_to_eat); ++token_index) {
        get_token(tokenizer);
    }
}

// TODO(Jonny): Parse var_args (...) as it's own Token.
// TODO(Jonny): Parse #if 0 blocks as whitespace. Also the #else of #if 1 blocks should also be whitespace.
internal Token
get_token(Tokenizer *tokenizer)
{
    assert(tokenizer);

    eat_whitespace(tokenizer);

    Token res = {};
    res.len = 1;
    res.e = tokenizer->at;
    char c = tokenizer->at[0];
    ++tokenizer->at;

    switch(c) {
        case '\0': { res.type = TokenType::end_of_stream;  } break;

        case '(':  { res.type = TokenType::open_paren;     } break;
        case ')':  { res.type = TokenType::close_param;    } break;
        case ':':  { res.type = TokenType::colon;          } break;
        case ';':  { res.type = TokenType::semi_colon;     } break;
        case '*':  { res.type = TokenType::asterisk;       } break;
        case '[':  { res.type = TokenType::open_bracket;   } break;
        case ']':  { res.type = TokenType::close_bracket;  } break;
        case '{':  { res.type = TokenType::open_brace;     } break;
        case '}':  { res.type = TokenType::close_brace;    } break;
        case '=':  { res.type = TokenType::equals;         } break;
        case ',':  { res.type = TokenType::comma;          } break;
        case '~':  { res.type = TokenType::tilde;          } break;

        case '.':  {
            B32 var_args = false;
            Tokenizer tokenizer_copy = *tokenizer;
            Token next = get_token(&tokenizer_copy);
            if(next.type == TokenType::period) {
                next = get_token(&tokenizer_copy);
                if(next.type == TokenType::period) {
                    var_args = true;

                    res.type = TokenType::var_args;
                    res.len = 3;

                    eat_tokens(tokenizer, 2);
                }
            }

            if(!var_args) {
                res.type = TokenType::period;
            }
        } break;

        case '"': {
            res.e = tokenizer->at;
            while((tokenizer->at[0]) && (tokenizer->at[0] != '"')) {
                if((tokenizer->at[0] == '\\') && (tokenizer->at[1])) { // TODO(Jonny): Is this right??
                    ++tokenizer->at;
                }

                ++tokenizer->at;
            }

            res.type = TokenType::string;
            res.len = tokenizer->at - res.e;
            if(tokenizer->at[0] == '"') {
                ++tokenizer->at;
            }
        } break;

        case '\'': {
            res.e = tokenizer->at;
            while((tokenizer->at[0]) && (tokenizer->at[0] != '\'')) {
                if((tokenizer->at[0] == '\\') && (tokenizer->at[1])) { // TODO(Jonny): Is this right??
                    ++tokenizer->at;
                }

                ++tokenizer->at;
            }

            res.type = TokenType::string;
            res.len = tokenizer->at - res.e;
            if(tokenizer->at[0] == '\'') {
                ++tokenizer->at;
            }
        } break;

        default: {
            if((is_alphabetical(c)) || (tokenizer->at[0] == '_')) {
                while((is_alphabetical(tokenizer->at[0])) || (is_num(tokenizer->at[0])) || (tokenizer->at[0] == '_')) {
                    ++tokenizer->at;
                }

                res.len = tokenizer->at - res.e;
                res.type = TokenType::identifier;
            } else if(is_num(c)) {
                while(is_num(tokenizer->at[0])) {
                    ++tokenizer->at;
                }

                res.len = tokenizer->at - res.e;
                res.type = TokenType::number;
            } else {
                res.type = TokenType::unknown;
            }
        } break;
    }

    return(res);
}

internal B32
token_equals(Token token, char *tatch)
{
    assert(tatch);

    B32 res = false;

    char *at = tatch;
    for(U32 str_index = 0; (str_index < token.len); ++str_index, ++at) {
        if((*at == '\0') == (*at == token.e[str_index])) {
            goto exit_func;
        }
    }

    res = (*at == 0);

exit_func:

    return(res);
}

struct ResultInt {
    S32 e;
    B32 success;
};

internal ResultInt
char_to_int(char C)
{
    ResultInt res = {};
    switch(C) {
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

        default: { assert(0); } break;
    }

    return(res);
}

internal ResultInt
string_to_int(String str)
{
    ResultInt res = {};

    for(U32 str_index = 0; (str_index < str.len); ++str_index) {
        ResultInt temp_int = char_to_int(str.e[str_index]);
        if(!temp_int.success) {
            break; // for
        }

        res.e *= 10;
        res.e += temp_int.e;

        if(str_index == str.len - 1) {
            res.success = true;
        }
    }

    assert(res.success);
    return(res);
}

internal ResultInt
string_to_int(char *str)
{
    String string = {};
    string.e = str;
    string.len = string_length(str);
    ResultInt res = string_to_int(string);

    return(res);
}

struct Variable {
    String type;
    String name;
    B32 is_ptr;
    U32 array_count; // This is 1 if it's not an array.
};

internal Variable
parse_member(Tokenizer *tokenizer, Token member_type_token)
{
    assert(tokenizer);

    Variable res = {};
    res.array_count = 1;
    res.type = token_to_string(member_type_token);

    B32 parsing = true;
    while(parsing) {
        Token token = get_token(tokenizer);
        switch(token.type) {
            case TokenType::asterisk: {
                res.is_ptr = true;
            } break;

            case TokenType::open_bracket: {
                Token size_token = get_token(tokenizer);
                if(size_token.type == TokenType::number) {
                    char buffer[256] = {};
                    token_to_string(size_token, buffer, array_count(buffer));
                    ResultInt arr_coount = string_to_int(buffer);
                    if(arr_coount.success) {
                        res.array_count = arr_coount.e;
                    }
                }
            } break;

            case TokenType::identifier: {
                res.name = token_to_string(token);

            } break;

            case TokenType::semi_colon: case TokenType::end_of_stream: {
                parsing = false;
            } break;
        }
    }

    return(res);
}

internal B32
require_token(Tokenizer *tokenizer, TokenType desired_type)
{
    assert(tokenizer);

    Token token = get_token(tokenizer);
    B32 res = (token.type == desired_type);

    return(res);
}

struct StructData {
    String name;
    U32 member_count;
    Variable *members;
};

internal StructData
parse_struct(Tokenizer *tokenizer, Memory *memory)
{
    assert((tokenizer) && (memory));

    StructData res = {};
    char *start = tokenizer->at;

    Token name = {};
    name = get_token(tokenizer);
    if(name.len > 1) {
        res.name = token_to_string(name);
        if(require_token(tokenizer, TokenType::open_brace)) {
            res.member_count = 0;
            char *member_pos[256] = {};
            Token member_token[256] = {};
            for(;;) {
                Token *mt = member_token + res.member_count;
                *mt = get_token(tokenizer);
                if(mt->type == TokenType::close_brace) {
                    break; // for
                } else if(mt->e[0] == '#') {
                    while(tokenizer->at[0] != '\n') {
                        ++tokenizer->at;
                    }
                } else if(mt->type == TokenType::tilde) {
                    // Do nothing.

                } else if((token_equals(*mt, "inline")) || (token_equals(*mt, "func"))) { // TODO(Jonny): Hacky way to skip member functions...
                    Token temp = get_token(tokenizer);
                    while(temp.type != TokenType::semi_colon) {
                        temp = get_token(tokenizer);
                    }
                } else {
                    member_pos[res.member_count++] = tokenizer->at;

                    Token token = get_token(tokenizer);
                    while(token.type != TokenType::semi_colon) {
                        token = get_token(tokenizer);
                    }
                }
            }

            res.members = push_permanent_array(memory, Variable, res.member_count);
            for(U32 member_index = 0; (member_index < res.member_count); ++member_index) {
                Tokenizer fake_tokenizer = { member_pos[member_index] };
                res.members[member_index] = parse_member(&fake_tokenizer, member_token[member_index]);
            }
        }
    }

    return(res);
}

internal char *
get_serialize_struct_implementation(char *def_struct_code, Memory *mem)
{
    assert((def_struct_code) && (mem));

    PtrSize res_size = 10000;
    char *res = cast(char *)push_permanent_memory(mem, res_size);
    if(res) {
        format_string(res, res_size,
                      "// TODO(Jonny): At some point, I'd like to replace memset, assert, and sprintf with my own versions. \n"
                      "size_t\n"
                      "serialize_struct__(void *var, MemberDefinition members_of_Something[], unsigned indent, size_t num_members, char *buffer, size_t buf_size, size_t bytes_written)\n"
                      "{\n"
                      "    char indent_buf[256] = {};\n"
                      "    unsigned indent_index = 0, member_index = 0;\n"
                      "\n"
                      "    assert((var) && (members_of_Something) && (num_members > 0) && (buffer) && (buf_size > 0));\n"
                      "    memset(buffer + bytes_written, 0, buf_size - bytes_written);/* TODO(Jonny): Implement my own memset. */\n"
                      "    for(indent_index = 0; (indent_index < indent); ++indent_index) {\n"
                      "        indent_buf[indent_index] = ' ';\n"
                      "    }\n"
                      "\n"
                      "    for(member_index = 0; (member_index < num_members); ++member_index) {\n"
                      "        MemberDefinition *member = members_of_Something + member_index;\n"
                      "\n"
                      "         void *member_ptr = (char *)var + member->offset;\n"
                      "         switch(member->type) {\n"
                      "            case meta_type_float: {\n"
                      "                for(unsigned arr_index = 0; (arr_index < member->arr_size); ++arr_index) {\n"
                      "                    float *value = (member->is_ptr) ? *(float **)member_ptr : (float *)member_ptr;\n"
                      "                    if(member->arr_size > 1) {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sfloat %%s[%%d] : %%f\", indent_buf, member->name, arr_index, value[arr_index]);\n"
                      "                    } else {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sfloat %%s : %%f\", indent_buf, member->name, value[arr_index]);\n"
                      "                    }\n"
                      "                }\n"
                      "            } break;\n"
                      "\n"
                      "            case meta_type_short: case meta_type_int: case meta_type_long: {\n"
                      "                for(unsigned arr_index = 0; (arr_index < member->arr_size); ++arr_index) {\n"
                      "                    int *value = (member->is_ptr) ? *(int **)member_ptr : (int *)member_ptr;\n"
                      "                    if(member->arr_size > 1) {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sint %%s[%%d] : %%d\", indent_buf, member->name, arr_index, value[arr_index]);\n"
                      "                    } else {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sint %%s : %%d\", indent_buf, member->name, value[arr_index]);\n"
                      "                    }\n"
                      "                }\n"
                      "            } break;\n"
                      "\n"
                      "            case meta_type_char: {\n"
                      "                if(member->is_ptr) {\n"
                      "                    bytes_written += sprintf(buffer + bytes_written, \"\\n%%schar * %%s : %%s\", indent_buf, member->name, *(char **)member_ptr);\n"
                      "                } else {\n"
                      "                    bytes_written += sprintf(buffer + bytes_written, \"\\n%%schar %%s : %%c\", indent_buf, member->name, *(char *)member_ptr);\n"
                      "                }\n"
                      "            } break;\n"
                      "\n"
                      "            case meta_type_double: {\n"
                      "                for(unsigned arr_index = 0; (arr_index < member->arr_size); ++arr_index) {\n"
                      "                    double *value = (member->is_ptr) ? *(double **)member_ptr : (double *)member_ptr;\n"
                      "                    if(member->arr_size > 1) {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sfloat %%s[%%d] : %%f\", indent_buf, member->name, arr_index, value[arr_index]);\n"
                      "                    } else {\n"
                      "                        bytes_written += sprintf((char *)buffer + bytes_written, \"\\n%%sfloat %%s : %%f\", indent_buf, member->name, value[arr_index]);\n"
                      "                    }\n"
                      "                }\n"
                      "            } break;\n"
                      "\n"
                      "            default: {\n"
                      "                %s\n"
                      "            } break; // default\n"
                      "        }\n"
                      "    }\n"
                      "\n"
                      "    return(bytes_written);\n"
                      "}\0",
                      def_struct_code
                     );
    }

    return(res);
}

internal char *
get_serialize_struct_declaration(void)
{
    char *res = "\n// size_t serialize_struct(void *var, type VariableType, char *buffer, size_t buf_size);\n"
                "#define serialize_struct(var, type, buffer, buf_size) serialize_struct_(var, type, 0, buffer, buf_size, 0)\n"
                "#define serialize_struct_(var, type, indent, buffer, buf_size, bytes_written) serialize_struct__((void *)&var, members_of_##type, indent, get_num_of_members(type), buffer, buf_size, bytes_written)\n"
                "size_t serialize_struct__(void *var, MemberDefinition members_of_Something[], unsigned indent, size_t num_members, char *buffer, size_t buf_size, size_t bytes_written);";

    return(res);
}

enum struct ExtensionType {
    unknown,
    cpp,
    c,
};

internal ExtensionType
get_extension_from_str(char *str)
{
    ExtensionType res = ExtensionType::unknown;

    PtrSize len = string_length(str);
    // TODO(Jonny): Do this properly...
    if((str[len - 1] == 'c') && (str[len - 2] == '.')) {
        res = ExtensionType::c;
    } else if((str[len - 1] == 'p') && (str[len - 2] == 'p') && (str[len - 3] == 'c') && (str[len - 4] == '.') ) {
        res = ExtensionType::cpp;
    }

    return(res);
}

struct EnumData {
    String name;
    String type;
    B32 is_struct;
};

internal EnumData
add_token_to_enum(Token name, Token type, B32 is_enum_struct)
{
    assert(name.type == TokenType::identifier);
    assert((type.type == TokenType::identifier) || (type.type == TokenType::unknown));

    EnumData res = {};
    res.is_struct = is_enum_struct;
    res.name = token_to_string(name);
    if(type.type == TokenType::identifier) {
        res.type = token_to_string(type);
    }

    return(res);
}

internal B32
str_compare(String a, String b)
{
    B32 res = false;

    if(a.len == b.len) {
        res = true;

        for(U32 str_index = 0; (str_index < a.len); ++str_index) {
            if(a.e[str_index] != b.e[str_index]) {
                res = false;
                break; // for
            }
        }
    }

    return(res);
}

internal B32
is_meta_type_already_in_array(String *array, U32 len, String test)
{
    B32 res = false;

    for(U32 arr_index = 0; (arr_index < len); ++arr_index) {
        if(str_compare(array[arr_index], test)) {
            res = true;
            break; // for
        }
    }

    return(res);
}

internal U32
set_primitive_type(String *array)
{
    assert(array);

    U32 res = 0;

    array[res].e = "char";
    array[res].len = string_length(array[res].e);
    ++res;

    array[res].e = "short";
    array[res].len = string_length(array[res].e);
    ++res;

    array[res].e = "int";
    array[res].len = string_length(array[res].e);
    ++res;

    array[res].e = "long";
    array[res].len = string_length(array[res].e);
    ++res;

    array[res].e = "float";
    array[res].len = string_length(array[res].e);
    ++res;

    array[res].e = "double";
    array[res].len = string_length(array[res].e);
    ++res;

    return(res);
}

#define copy_literal_to_char_buffer(buffer, index, lit) copy_literal_to_char_buffer_(buffer + index, index, lit, sizeof(lit) - 1)
internal U32
copy_literal_to_char_buffer_(char *buffer, U32 index, char *lit, U32 lit_len)
{
    assert(buffer);
    assert((lit) && (lit_len));

    for(U32 str_index = 0; (str_index < lit_len); ++str_index) {
        buffer[str_index] = lit[str_index];
    }

    U32 res = index + lit_len;
    return(res);
}

internal char *
get_default_struct_string(void)
{
    char *res = "                    case meta_type_%S: {\n"
                "                        if(member->is_ptr) {\n"
                "                            bytes_written += serialize_struct_(**(char **)member_ptr, %S, indent + 4, buffer, buf_size - bytes_written, bytes_written);\n"
                "                        } else {\n"
                "                            bytes_written += serialize_struct_(*(char *)member_ptr, %S, indent + 4, buffer, buf_size - bytes_written, bytes_written);\n"
                "                        }\n"
                "                    } break;\n ";

    return(res);
}

struct FunctionData {
    String linkage;
    String ret_type;
    String name;
    Variable params[32];
    U32 param_count;
};

// TODO(Jonny): Always make sure arguments[0] is actually the exe being run...
StuffToWrite
start_parsing(AllFiles all_files, Memory *memory)
{
    EnumData *enum_data = push_permanent_array(memory, EnumData, 256);
    U32 enum_count = 0;

    StructData *struct_data = push_permanent_array(memory, StructData, 256);
    U32 struct_count = 0;

    String *union_data = push_permanent_array(memory, String,  256);
    U32 union_count = 0;

    FunctionData *func_data = push_permanent_array(memory, FunctionData, 256);
    U32 func_count = 0;

    for(U32 index = 0; (index < all_files.count); ++index) {
        //char *file = platform_services.read_entire_file_and_null_terminate(arguments[index], memory);
        char *file = all_files.file[index];

        ExtensionType extension_type = ExtensionType::cpp; // TODO(Jonny): Debug.
        if(extension_type != ExtensionType::unknown) {
            if(file) {
                Tokenizer tokenizer = {};
                tokenizer.at = file;

                B32 parsing = true;
                while(parsing) {
                    Token token = get_token(&tokenizer);
                    switch(token.type) {
                        case TokenType::end_of_stream: {
                            parsing = false;
                        } break;

                        case TokenType::identifier: {
                            if(token_equals(token, "struct")) { // TODO(Jonny): Support classes and typedef struct {} name structs.
                                struct_data[struct_count++] = parse_struct(&tokenizer, memory); // TODO(Jonny): This fails at a struct declared within a struct/union.
                            } else if((token_equals(token, "union"))) {
                                Token name = get_token(&tokenizer);
                                union_data[union_count++] = token_to_string(name);
                            } else if((token_equals(token, "enum"))) {
                                Token name = get_token(&tokenizer);
                                B32 is_enum_struct = false;
                                if((token_equals(name, "class")) || (token_equals(name, "struct"))) {
                                    is_enum_struct = true;
                                    name = get_token(&tokenizer);
                                }

                                if(name.type == TokenType::identifier) {
                                    // If the enum has an underlying type, get it.
                                    Token underlying_type = {};
                                    Token next = get_token(&tokenizer);
                                    if(next.type == TokenType::colon) {
                                        underlying_type = get_token(&tokenizer);
                                        next = get_token(&tokenizer);
                                    }

                                    // TODO(Jonny): This will fail if the size has two names (eg, long long).
                                    if(next.type == TokenType::open_brace) {
                                        enum_data[enum_count++] = add_token_to_enum(name, underlying_type, is_enum_struct);
                                    }
                                }
                            } else {
                                Token linkage = {};
                                Token return_type = {};
                                if((token_equals(token, "static")) || (token_equals(token, "inline")) || (token_equals(token, "internal"))) {
                                    linkage = token;
                                    return_type = get_token(&tokenizer);
                                } else {
                                    return_type = token;
                                }

                                if(return_type.type == TokenType::identifier) {
                                    if((!token_equals(return_type, "if")) && (!token_equals(return_type, "do")) && (!token_equals(return_type, "while")) && (!token_equals(return_type, "switch"))) { // TODO(Jonny): Extra check...
                                        Token name = get_token(&tokenizer);
                                        if(name.type == TokenType::identifier) {
                                            // Don't forward declare main.
                                            if((!token_equals(name, "main")) && (!token_equals(name, "WinMain")) && (!token_equals(name, "_mainCRTStartup")) && (!token_equals(name, "_WinMainCRTStartup")) && (!token_equals(name, "__DllMainCRTStartup"))) {
                                                if((linkage.type == TokenType::identifier) || (linkage.type == TokenType::unknown)) {
                                                    Token should_be_open_brace = get_token(&tokenizer);
                                                    if(should_be_open_brace.type == TokenType::open_paren) {
                                                        FunctionData *fd = func_data + func_count;
                                                        if(linkage.type == TokenType::identifier) {
                                                            fd->linkage = token_to_string(linkage);
                                                        }
                                                        fd->ret_type = token_to_string(return_type);
                                                        fd->name = token_to_string(name);

                                                        // Set the array size to 1 for all the variables...
                                                        for(U32 param_index = 0; (param_index < array_count(fd->params)); ++param_index) {
                                                            fd->params[param_index].array_count = 1;
                                                        }

                                                        B32 parsing = true;
                                                        Variable *var = fd->params + fd->param_count;

                                                        // If there aren't any parameters then just skip them.
                                                        {
                                                            Tokenizer copy = tokenizer;
                                                            Token token = get_token(&copy);
                                                            if(token_equals(token, "void")) {
                                                                Token next = get_token(&copy);
                                                                if(next.type == TokenType::close_param) {
                                                                    parsing = false;
                                                                }
                                                            }
                                                        }

                                                        // Parse the parameters.
                                                        while(parsing) {
                                                            Token token = get_token(&tokenizer);
                                                            switch(token.type) {
                                                                case TokenType::asterisk: {
                                                                    var->is_ptr = true;
                                                                } break;

                                                                case TokenType::open_bracket: {
                                                                    Token SizeToken = get_token(&tokenizer);
                                                                    if(SizeToken.type == TokenType::number) {
                                                                        char buffer[256] = {};
                                                                        token_to_string(SizeToken, buffer, array_count(buffer));
                                                                        ResultInt arr_count = string_to_int(buffer);
                                                                        if(arr_count.success) {
                                                                            var->array_count = arr_count.e;
                                                                        }
                                                                    }
                                                                } break;

                                                                case TokenType::identifier: {
                                                                    if(var->type.len == 0) {
                                                                        var->type = token_to_string(token);
                                                                    } else {
                                                                        var->name = token_to_string(token);
                                                                        ++fd->param_count;
                                                                    }
                                                                } break;

                                                                case TokenType::comma: {
                                                                    ++var;
                                                                } break;

                                                                case TokenType::end_of_stream: case TokenType::open_brace: {
                                                                    parsing = false; // TODO(Jonny): Something baaaad happened...
                                                                } break;
                                                            }
                                                        }

                                                        ++func_count;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } break;
                    }
                }
            }
        }
    }

    //
    // Source file.
    //
    StuffToWrite res = {};

    OutputBuffer source_output = create_output_buffer(256 * 256, memory); // TODO(Jonny): Random size...

    write_to_output_buffer(&source_output, "#if !defined(GENERATED_CPP)\n\n");
    write_to_output_buffer(&source_output, "#include \"generated.h\"\n#include <stdio.h>\n#include <string.h>\n#include <assert.h>\n\n");

    // Recreated Structs.
    write_to_output_buffer(&source_output, "//\n// Recreated structs.\n//\n");
    for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
        StructData *sd = struct_data + struct_index;
        write_to_output_buffer(&source_output, "struct %S {\n", sd->name.len, sd->name.e);
        for(U32 member_index = 0; (member_index < sd->member_count); ++member_index) {
            Variable *md = sd->members + member_index;
            char const *arr = (md->array_count > 1) ? "[%u]" : "";
            char arr_buffer[256] = {};
            if(md->array_count > 1) {
                format_string(arr_buffer, 256, arr, md->array_count);
            }

            write_to_output_buffer(&source_output, "    %S %s%S%s;\n",
                                   md->type.len, md->type.e,
                                   (md->is_ptr) ? "*" : "",
                                   md->name.len, md->name.e,
                                   (md->array_count > 1) ? arr_buffer : arr);

        }
        write_to_output_buffer(&source_output, "};\n\n");
    }

    // Struct Meta Data
    write_to_output_buffer(&source_output, "//\n// Struct meta data.\n//\n");
    for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
        StructData *sd = struct_data + struct_index;
        write_to_output_buffer(&source_output, "// Meta data for: %S\n", sd->name.len, sd->name.e);
        write_to_output_buffer(&source_output, "MemberDefinition members_of_%S[] = {\n", sd->name.len, sd->name.e);
        for(U32 member_index = 0; (member_index < sd->member_count); ++member_index) {
            Variable *md = sd->members + member_index;
            write_to_output_buffer(&source_output, "    {meta_type_%S, \"%S\", (size_t)&((%S *)0)->%S, %d, %d},\n",
                                   md->type.len, md->type.e,
                                   md->name.len, md->name.e,
                                   sd->name.len, sd->name.e,
                                   md->name.len, md->name.e,
                                   md->is_ptr,
                                   md->array_count);
        }
        write_to_output_buffer(&source_output, "};\n");
    }

    // Function meta data.
    write_to_output_buffer(&source_output, "\n//\n// Function meta data.\n//\n");
    for(U32 func_index = 0; (func_index < func_count); ++func_index) {
        FunctionData *fd = func_data + func_index;
        write_to_output_buffer(&source_output, "FunctionMetaData function_data_%S = {\n", fd->name.len, fd->name.e);
        char buf[256] = {};
        if(fd->linkage.len > 0) {
            char *meta_data = "    \"%S\",\n"
                              "    \"%S\",\n"
                              "    \"%S\",\n"
                              "    %u,\n";
            format_string(buf, array_count(buf), meta_data,
                          fd->linkage.len, fd->linkage.e,
                          fd->ret_type.len, fd->ret_type.e,
                          fd->name.len, fd->name.e,
                          fd->param_count);
        } else {
            char *meta_data = "    0,\n"
                              "    \"%S\",\n"
                              "    \"%S\",\n"
                              "    %u,\n";
            format_string(buf, array_count(buf), meta_data,
                          fd->ret_type.len, fd->ret_type.e,
                          fd->name.len, fd->name.e,
                          fd->param_count);
        }

        write_to_output_buffer(&source_output, buf);

        write_to_output_buffer(&source_output, "    {\n");
        {
            for(U32 param_index = 0; (param_index < fd->param_count); ++param_index) {
                Variable *param = fd->params + param_index;
                write_to_output_buffer(&source_output, "        {\"%S\", \"%S\"}", param->type.len, param->type.e, param->name.len, param->name.e);
                if(param_index != fd->param_count - 1) {
                    write_to_output_buffer(&source_output, ",\n");
                }
            }
        }
        write_to_output_buffer(&source_output, "\n    }");

        write_to_output_buffer(&source_output, "\n};\n\n");
    }

    write_to_output_buffer(&source_output, "\n\n");

    PtrSize def_struct_code_size = 256 * 256;
    char *def_struct_code = cast(char *)push_permanent_memory(memory, def_struct_code_size);
    if(def_struct_code) {
        U32 index = 0;
        index = copy_literal_to_char_buffer(def_struct_code, index, "switch(member->type) {\n");
        for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
            StructData *sd = struct_data + struct_index;
            char *DefaultStructString = get_default_struct_string();
            index += format_string(def_struct_code + index, def_struct_code_size, DefaultStructString,
                                   sd->name.len, sd->name.e, sd->name.len, sd->name.e, sd->name.len, sd->name.e);

        }

        index = copy_literal_to_char_buffer(def_struct_code, index, "                }");
    }

    char *serialize_struct_implementation = get_serialize_struct_implementation(def_struct_code, memory);
    write_to_output_buffer(&source_output, "%s", serialize_struct_implementation);

    // # Guard stuff
    write_to_output_buffer(&source_output, "\n\n#define GENERATED_CPP\n");
    write_to_output_buffer(&source_output, "#endif // #if !defined(GENERATED_CPP)\n");
    //platform_services.write_to_file("generated.cpp", source_output.buffer, source_output.index);

    res.source_size = source_output.index;
    res.source_data = source_output.buffer;

    //
    // Header file.
    //
    OutputBuffer header_output = create_output_buffer(256 * 256, memory);

    write_to_output_buffer(&header_output, "#if !defined(GENERATED_H)\n\n#include <stdio.h>\n\n");

    // Write out meta types
    TempMemory types_memory = push_temp_arr(memory, String, 256);
    {
        String *types = cast(String *)types_memory.memory;
        U32 type_count = set_primitive_type(types);
        for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
            StructData *sd = struct_data + struct_index;

            if(!is_meta_type_already_in_array(types, type_count, sd->name)) {
                types[type_count++] = sd->name;
            }

            for(U32 member_index = 0; (member_index < sd->member_count); ++member_index) {
                Variable *md = sd->members + member_index;

                if(!is_meta_type_already_in_array(types, type_count, md->type)) {
                    types[type_count++] = md->type;
                }
            }
        }

        write_to_output_buffer(&header_output, "enum MetaType {\n");
        for(U32 type_index = 0; (type_index < type_count); ++type_index) {
            String *type = types + type_index;
            write_to_output_buffer(&header_output, "    meta_type_%S,\n", type->len, type->e);
        }
        write_to_output_buffer(&header_output, "};\n");
    }
    pop_temp_memory(&types_memory);

    // struct member_defintion.
    write_to_output_buffer(&header_output, "\n\nstruct MemberDefinition {\n    MetaType type;\n    char *name;\n    size_t offset;\n    int is_ptr;\n    unsigned arr_size;\n};\n\n#define get_num_of_members(type) num_members_for_##type\n\n");

    // Struct meta data.
    write_to_output_buffer(&header_output, "\n//\n// Struct meta data.\n//\n");
    for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
        StructData *sd = &struct_data[struct_index];
        write_to_output_buffer(&header_output, "// Meta Data for: %S\n", sd->name.len, sd->name.e);
        write_to_output_buffer(&header_output, "extern MemberDefinition members_of_%S[];\n", sd->name.len, sd->name.e);
        write_to_output_buffer(&header_output, "static const size_t num_members_for_%S = %u;\n\n", sd->name.len, sd->name.e, sd->member_count);
    }

    // Struct forward declarations.
    write_to_output_buffer(&header_output, "\n//\n// Forward declared structs.\n//\n");
    for(U32 struct_index = 0; (struct_index < struct_count); ++struct_index) {
        StructData *sd = struct_data + struct_index;
        write_to_output_buffer(&header_output, "struct %S;\n", sd->name.len, sd->name.e);
    }

    // Enum forward declarations.
    write_to_output_buffer(&header_output, "\n//\n// Forward declared enums.\n//\n");
    for(U32 enum_index = 0; (enum_index < enum_count); ++enum_index) {
        EnumData *ED = &enum_data[enum_index];
        char const *struct_part = (ED->is_struct) ? "struct ": "";
        if(ED->type.e) {
            write_to_output_buffer(&header_output, "enum %s%S : %S;\n", struct_part, ED->name.len, ED->name.e, ED->type.len, ED->type.e);
        } else {
            write_to_output_buffer(&header_output, "enum %s%S;\n", struct_part, ED->name.len, ED->name.e);
        }
    }

    // Union forward declarations.
    write_to_output_buffer(&header_output, "\n//\n// Forward declared unions.\n//\n");
    for(U32 union_index = 0; (union_index < union_count); ++union_index) {
        String *UD = &union_data[union_index];
        write_to_output_buffer(&header_output, "union %S;\n", UD->len, UD->e);
    }

    // Function forward declarations.
    write_to_output_buffer(&header_output, "\n//\n// Forward declared functions.\n//\n");
    for(U32 func_index = 0; (func_index < func_count); ++func_index) {
        FunctionData *fd = func_data + func_index;

        if(fd->linkage.len > 0) {
            write_to_output_buffer(&header_output, "%S ", fd->linkage.len, fd->linkage.e);
        }

        write_to_output_buffer(&header_output, "%S %S(", fd->ret_type.len, fd->ret_type.e, fd->name.len, fd->name.e);
        if(fd->param_count == 0) {
            write_to_output_buffer(&header_output, "void");
        } else {
            for(U32 param_index = 0; (param_index < fd->param_count); ++param_index) {
                Variable *param = fd->params + param_index;
                if(param_index != 0) {
                    write_to_output_buffer(&header_output, ", ");
                }

                write_to_output_buffer(&header_output, "%S %s%S%s", param->type.len, param->type.e, (param->is_ptr) ? "*" : "", param->name.len, param->name.e, (param->array_count > 1) ? "[]" : "");
            }
        }

        write_to_output_buffer(&header_output, ");\n");
    }

    // Function meta data.
    write_to_output_buffer(&header_output, "\n//\n// Function meta data.\n//\n");
    char *variable_and_func_meta_data_structs = "struct Variable {\n"
                                                "    char *ret_type;\n"
                                                "    char *name;\n"
                                                "};"
                                                "\n"
                                                "\n"
                                                "unsigned const MAX_NUMBER_OF_PARAMS = 32;\n"
                                                "struct FunctionMetaData {\n"
                                                "    char *linkage;\n"
                                                "    char *ret_type;\n"
                                                "    char *name;\n"
                                                "    unsigned param_count;\n"
                                                "    Variable params[MAX_NUMBER_OF_PARAMS];\n"
                                                "};\n"
                                                "#define get_func_meta_data(func) function_data_##func"
                                                "\n";
    write_to_output_buffer(&header_output, variable_and_func_meta_data_structs);
    for(U32 func_index = 0; (func_index < func_count); ++func_index) {
        FunctionData *fd = func_data + func_index;
        char buf[256] = {};
        if(fd->linkage.len > 0) {
            char const *meta_data = "extern FunctionMetaData function_data_%S;\n";
            format_string(buf, array_count(buf), meta_data,
                          fd->name.len, fd->name.e);
        } else {
            char const *meta_data = "extern FunctionMetaData function_data_%S;\n";
            format_string(buf, array_count(buf), meta_data,
                          fd->name.len, fd->name.e);
        }

        write_to_output_buffer(&header_output, buf);
    }

    // SerializeStruct header.
    write_to_output_buffer(&header_output, get_serialize_struct_declaration());

    // # Guard macro.
    write_to_output_buffer(&header_output, "\n\n#define GENERATED_H");
    write_to_output_buffer(&header_output, "\n#endif // !defined(GENERATED_H)\n");

    //platform_services.write_to_file("generated.h", header_output.buffer, header_output.index);

    res.header_size = header_output.index;
    res.header_data = header_output.buffer;

    return(res);
}