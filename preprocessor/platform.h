/*===================================================================================================
  File:                    platform.h
  Author:                  Jonathan Livingstone
  Email:                   seagull127@ymail.com
  Licence:                 Public Domain
                           No Warranty is offered or implied about the reliability,
                           suitability, or usability
                           The use of this code is at your own risk
                           Anyone can use this code, modify it, sell it to terrorists, etc.
  ===================================================================================================*/

// TODO(Jonny):
// Generate metadata for enums so they can be converted to strings.
// Rewrite the union parser. I'm not sure if metadata for a union could really be generated, but possibly?
// Build without crt and only link to kernel32.
// Convert all the char * string stuff to use the string struct.
// I'll need some way to track overloaded functions, because this'll flop right now with them...
// Make a slots/signal system.
// Make a "lose" keyword. The use-case for this is write_to_output_buffer in preprocessor.cpp.

#include <stdarg.h>
#include <stdint.h>

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

typedef int64_t S64;
typedef int32_t S32;
typedef int16_t s16;
typedef int8_t S8;

#if ENVIRONMENT64
    typedef U64 PtrSize;
    typedef S64 SignedPtrSize;
#elif ENVIRONMENT32
    typedef U32 PtrSize;
    typedef S32 SignedPtrSize;
#endif

typedef float F32;
typedef double F64;

typedef S32 B32;

#define internal static
#define persist static
#define global static

#define cast(type) (type)

#if INTERNAL
    #define assert(Expression) do { persist B32 Ignore = false; if(!Ignore) { if(!(Expression)) { *cast(int volatile *)0 = 0; } } } while(0)
#else
    #define assert(Expression) { }
#endif

#define array_count(arr) (sizeof(arr) / (sizeof((arr)[0])))

//
// Memory stuff.
//
#define copy_memory_block(dest, source, size) copy_memory_block_(cast(U8 *)dest, cast(U8 *)source, size)
internal void
copy_memory_block_(U8 *dest, U8 *source, PtrSize size)
{
    assert((dest) && (source) && (size > 0));

    while(size-- > 0) {
        *dest++ = *source++;
    }
}

#define zero_memory_block(dest, size) set_memory_block(dest, 0, size)
#define set_memory_block(dest, value, size) set_memory_block_(cast(U8 *)dest, value, size)
internal void
set_memory_block_(U8 *dest, U8 value, PtrSize size)
{
    assert((dest) && (size > 0));

    while(size-- > 0) {
        *dest++ = value;
    }
}

struct Memory {
    void *file_memory;
    PtrSize file_index;
    PtrSize file_size;

    void *permanent_memory;
    PtrSize permanent_index;
    PtrSize permanent_size;

    void *temp_memory;
    PtrSize temp_index;
    PtrSize temp_size;
};

internal Memory
create_memory(void *all_memory, PtrSize file_size, PtrSize permanent_size, PtrSize temp_size)
{
    assert((all_memory) && (file_size) && (permanent_size > 0) && (temp_size > 0));

    zero_memory_block(all_memory, file_size + permanent_size + temp_size);

    Memory res = {};
    res.file_memory = all_memory;
    res.file_size = file_size;

    res.permanent_memory = cast(char *)all_memory + file_size;
    res.permanent_size = permanent_size;

    res.temp_memory = cast(char *)all_memory + file_size + permanent_size;
    res.temp_size = temp_size;

    return(res);
}

U32 const DEFAULT_MEMORY_ALIGNMENT = 4;

internal PtrSize
get_alignment_offset(void *memory, PtrSize index, PtrSize desired_alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    assert(memory);

    PtrSize res = 0;

    PtrSize result_pointer = cast(PtrSize)memory + index;
    PtrSize alignment_mask = desired_alignment - 1;
    if(result_pointer & alignment_mask) {
        res = desired_alignment - (result_pointer & alignment_mask);
    }

    return(res);
}

internal char *
push_file_memory(Memory *memory, PtrSize size, PtrSize alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    assert((memory) && (size > 0));

    PtrSize alignment_offset = get_alignment_offset(memory->file_memory, memory->file_index, alignment);
    assert(memory->file_index + alignment_offset + size <= memory->file_size);

    char *res = cast(char *)memory->file_memory + (memory->file_index + alignment_offset);

    memory->file_index += size + alignment_offset;

    return(res);
}

#define push_permanent_struct(memory, type, ...) cast(type *)push_permanent_memory(memory, sizeof(type), ##__VA_ARGS__)
#define push_permanent_array(memory, type, len, ...) cast(type *)push_permanent_memory(memory, sizeof(type) * len, ##__VA_ARGS__)
internal void *
push_permanent_memory(Memory *memory, PtrSize size, PtrSize alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    assert((memory) && (size > 0));

    PtrSize alignment_offset = get_alignment_offset(memory->permanent_memory, memory->permanent_index, alignment);
    assert(memory->permanent_index + alignment_offset + size <= memory->permanent_size);

    void *res = cast(U8 *)memory->permanent_memory + (memory->permanent_index + alignment_offset);

    memory->permanent_index += size + alignment_offset;

    return(res);
}

struct TempMemory {
    Memory *memory;
    U8 *Block;
    PtrSize size;
    PtrSize used;
    PtrSize alignment_offset;
};

// Temp memory.
#define push_temp_struct(memory, type, ...) push_temp_memory(memory, sizeof(type), ##__VA_ARGS__)
#define push_temp_arr(memory, type, len, ...) push_temp_memory(memory, sizeof(type) * len, ##__VA_ARGS__)
internal TempMemory
push_temp_memory(Memory *memory, PtrSize size, PtrSize alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    assert((memory) && (size > 0));

    PtrSize alignment_offset = get_alignment_offset(memory->temp_memory, memory->temp_index, alignment);
    assert(memory->temp_index + alignment_offset + size <= memory->temp_size);

    TempMemory res = {};
    res.memory = memory;
    res.alignment_offset = alignment_offset;
    res.Block = cast(U8 *)memory->temp_memory + (memory->temp_index + res.alignment_offset);

    res.size = size;
    res.used = 0;
    memory->temp_index += size + res.alignment_offset;
    zero_memory_block(res.Block, res.size);

    return(res);
}

internal void
pop_temp_memory(TempMemory *temp_memory)
{
    assert((temp_memory) && (temp_memory->memory) && (temp_memory->Block) && (temp_memory->size > 0));

    temp_memory->memory->temp_index -= temp_memory->size + temp_memory->alignment_offset;
    zero_memory_block(temp_memory, sizeof(*temp_memory));
}

#define push_struct_off_temp_memory(temp_memory, type, ...) cast(type *)push_off_temp_memory(temp_memory, sizeof(type), ##__VA_ARGS__)
#define push_array_off_temp_memory(temp_memory, type, len, ...) cast(type *)push_off_temp_memory(temp_memory, sizeof(type) * len, ##__VA_ARGS__)
internal void *
push_off_temp_memory(TempMemory *temp_memory, PtrSize size, PtrSize alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    assert((temp_memory) && (size > 0));

    PtrSize alignment_offset = get_alignment_offset(temp_memory->Block, temp_memory->used, alignment);
    void *res = temp_memory->Block + temp_memory->used + alignment_offset;
    assert(temp_memory->used + alignment_offset <= temp_memory->size);

    temp_memory->used += size + alignment_offset;

    return(res);
}

//
// Utils.
//

// TODO(Jonny): Pass the Destination size in here too...
internal B32
is_end_of_line(char c)
{
    B32 res = ((c == '\n') || (c == '\r'));

    return(res);
}

internal B32
is_whitespace(char c)
{
    B32 res = ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || (is_end_of_line(c)));

    return(res);
}

//
// Platform Services.
//
#if 0
#define READ_ENTIRE_FILE_AND_NULL_TERMINATE(name) char *name(char *filename, Memory *memory)
typedef READ_ENTIRE_FILE_AND_NULL_TERMINATE(PlatformReadEntireFileAndNullTerminate);

struct PlatformServices {
    PlatformReadEntireFileAndNullTerminate *read_entire_file_and_null_terminate;
};
#endif
//
// Start Parsing function.
//
struct AllFiles {
    char *file[16]; // TODO(Jonny): Random size.
    U32 count;
};

struct StuffToWrite {
    PtrSize header_size;
    void *header_data;

    PtrSize source_size;
    void *source_data;
};

StuffToWrite start_parsing(AllFiles all_files, Memory *memory);