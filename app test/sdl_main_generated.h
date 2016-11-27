#if !defined(GENERATED_H)

#include "static_generated.h"

/* Enum with field for every type detected. */
typedef enum MetaType {
    meta_type_char,
    meta_type_short,
    meta_type_int,
    meta_type_long,
    meta_type_float,
    meta_type_double,
    meta_type_V2,
    meta_type_Transform,
    meta_type_Ball,
    meta_type_Paddle,
    meta_type_GameState,
} MetaType;


/* Struct meta data. */

/* Meta data for: V2. */
static int const num_members_for_V2 = 2;
static MemberDefinition *
get_members_of_V2(void)
{
    typedef struct V2 {  int x;  int y;  } V2;
    typedef struct Transform {  V2 pos;  V2 size;  } Transform;
    typedef struct Ball {  V2 pos;  int radius;  int speed;  int direction;  } Ball;
    typedef struct Paddle : public Transform {  int score;  } Paddle;
    typedef struct GameState {  Paddle right;  Paddle left;  Ball ball;  } GameState;

    static MemberDefinition res[] = {
        {meta_type_int, "x", (size_t)&((V2 *)0)->x, 0, 1},
        {meta_type_int, "y", (size_t)&((V2 *)0)->y, 0, 1},
    };

    return(res);
}

/* Meta data for: Transform. */
static int const num_members_for_Transform = 2;
static MemberDefinition *
get_members_of_Transform(void)
{
    typedef struct V2 {  int x;  int y;  } V2;
    typedef struct Transform {  V2 pos;  V2 size;  } Transform;
    typedef struct Ball {  V2 pos;  int radius;  int speed;  int direction;  } Ball;
    typedef struct Paddle : public Transform {  int score;  } Paddle;
    typedef struct GameState {  Paddle right;  Paddle left;  Ball ball;  } GameState;

    static MemberDefinition res[] = {
        {meta_type_V2, "pos", (size_t)&((Transform *)0)->pos, 0, 1},
        {meta_type_V2, "size", (size_t)&((Transform *)0)->size, 0, 1},
    };

    return(res);
}

/* Meta data for: Ball. */
static int const num_members_for_Ball = 4;
static MemberDefinition *
get_members_of_Ball(void)
{
    typedef struct V2 {  int x;  int y;  } V2;
    typedef struct Transform {  V2 pos;  V2 size;  } Transform;
    typedef struct Ball {  V2 pos;  int radius;  int speed;  int direction;  } Ball;
    typedef struct Paddle : public Transform {  int score;  } Paddle;
    typedef struct GameState {  Paddle right;  Paddle left;  Ball ball;  } GameState;

    static MemberDefinition res[] = {
        {meta_type_V2, "pos", (size_t)&((Ball *)0)->pos, 0, 1},
        {meta_type_int, "radius", (size_t)&((Ball *)0)->radius, 0, 1},
        {meta_type_int, "speed", (size_t)&((Ball *)0)->speed, 0, 1},
        {meta_type_int, "direction", (size_t)&((Ball *)0)->direction, 0, 1},
    };

    return(res);
}

/* Meta data for: Paddle. */
static int const num_members_for_Paddle = 3;
static MemberDefinition *
get_members_of_Paddle(void)
{
    typedef struct V2 {  int x;  int y;  } V2;
    typedef struct Transform {  V2 pos;  V2 size;  } Transform;
    typedef struct Ball {  V2 pos;  int radius;  int speed;  int direction;  } Ball;
    typedef struct Paddle : public Transform {  int score;  } Paddle;
    typedef struct GameState {  Paddle right;  Paddle left;  Ball ball;  } GameState;

    static MemberDefinition res[] = {
        {meta_type_int, "score", (size_t)&((Paddle *)0)->score, 0, 1},
        {meta_type_V2, "pos", (size_t)&((Paddle *)0)->pos, 0, 1},
        {meta_type_V2, "size", (size_t)&((Paddle *)0)->size, 0, 1},
    };

    return(res);
}

/* Meta data for: GameState. */
static int const num_members_for_GameState = 3;
static MemberDefinition *
get_members_of_GameState(void)
{
    typedef struct V2 {  int x;  int y;  } V2;
    typedef struct Transform {  V2 pos;  V2 size;  } Transform;
    typedef struct Ball {  V2 pos;  int radius;  int speed;  int direction;  } Ball;
    typedef struct Paddle : public Transform {  int score;  } Paddle;
    typedef struct GameState {  Paddle right;  Paddle left;  Ball ball;  } GameState;

    static MemberDefinition res[] = {
        {meta_type_Paddle, "right", (size_t)&((GameState *)0)->right, 0, 1},
        {meta_type_Paddle, "left", (size_t)&((GameState *)0)->left, 0, 1},
        {meta_type_Ball, "ball", (size_t)&((GameState *)0)->ball, 0, 1},
    };

    return(res);
}


/* Function to serialize a struct to a char array buffer. */
static size_t
serialize_struct__(void *var, MemberDefinition members_of_Something[], char const *name, int indent, size_t num_members, char *buffer, size_t buf_size, size_t bytes_written)
{
    char indent_buf[256];
    unsigned indent_index = 0, member_index = 0, arr_index = 0;

    memset(indent_buf, 0, 256);


    assert((var) && (members_of_Something) && (num_members > 0) && (buffer) && (buf_size > 0));
    memset(buffer + bytes_written, 0, buf_size - bytes_written);
    for(indent_index = 0; (indent_index < indent); ++indent_index) {
        indent_buf[indent_index] = ' ';
    }

    bytes_written += sprintf((char *)buffer + bytes_written, "\n%s%s", indent_buf, name);
    indent += 4;

    for(indent_index = 0; (indent_index < indent); ++indent_index) {
        indent_buf[indent_index] = ' ';
    }

    for(member_index = 0; (member_index < num_members); ++member_index) {
         MemberDefinition *member = members_of_Something + member_index;

         void *member_ptr = (char *)var + member->offset;
         switch(member->type) {
            case meta_type_float: {
                for(arr_index = 0; (arr_index < member->arr_size); ++arr_index) {
                    float *value = (member->is_ptr) ? *(float **)member_ptr : (float *)member_ptr;
                    if(member->arr_size > 1) {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sfloat %s[%d] = %f", indent_buf, member->name, arr_index, value[arr_index]);
                    } else {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sfloat %s = %f", indent_buf, member->name, value[arr_index]);
                    }
                }
            } break;

            case meta_type_short: case meta_type_int: case meta_type_long: {
                for(arr_index = 0; (arr_index < member->arr_size); ++arr_index) {
                    int *value = (member->is_ptr) ? *(int **)member_ptr : (int *)member_ptr;
                    if(member->arr_size > 1) {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sint %s[%d] = %d", indent_buf, member->name, arr_index, value[arr_index]);
                    } else {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sint %s = %d", indent_buf, member->name, value[arr_index]);
                    }
                }
            } break;

            case meta_type_char: {
                if(member->is_ptr) {
                    bytes_written += sprintf(buffer + bytes_written, "\n%schar * %s = \"%s\"", indent_buf, member->name, *(char **)member_ptr);
                } else {
                    bytes_written += sprintf(buffer + bytes_written, "\n%schar %s = %c", indent_buf, member->name, *(char *)member_ptr);
                }
            } break;

            case meta_type_double: {
                for(arr_index = 0; (arr_index < member->arr_size); ++arr_index) {
                    double *value = (member->is_ptr) ? *(double **)member_ptr : (double *)member_ptr;
                    if(member->arr_size > 1) {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sfloat %s[%d] = %f", indent_buf, member->name, arr_index, value[arr_index]);
                    } else {
                        bytes_written += sprintf((char *)buffer + bytes_written, "\n%sfloat %s = %f", indent_buf, member->name, value[arr_index]);
                    }
                }
            } break;

            default: {
                switch(member->type) {
                    case meta_type_V2: {
                        if(member->is_ptr) {
                            bytes_written = serialize_struct_(**(char **)member_ptr, V2, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        } else {
                            bytes_written = serialize_struct_(*(char *)member_ptr, V2, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        }
                    } break;

                    case meta_type_Transform: {
                        if(member->is_ptr) {
                            bytes_written = serialize_struct_(**(char **)member_ptr, Transform, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        } else {
                            bytes_written = serialize_struct_(*(char *)member_ptr, Transform, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        }
                    } break;

                    case meta_type_Ball: {
                        if(member->is_ptr) {
                            bytes_written = serialize_struct_(**(char **)member_ptr, Ball, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        } else {
                            bytes_written = serialize_struct_(*(char *)member_ptr, Ball, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        }
                    } break;

                    case meta_type_Paddle: {
                        if(member->is_ptr) {
                            bytes_written = serialize_struct_(**(char **)member_ptr, Paddle, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        } else {
                            bytes_written = serialize_struct_(*(char *)member_ptr, Paddle, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        }
                    } break;

                    case meta_type_GameState: {
                        if(member->is_ptr) {
                            bytes_written = serialize_struct_(**(char **)member_ptr, GameState, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        } else {
                            bytes_written = serialize_struct_(*(char *)member_ptr, GameState, member->name, indent, buffer, buf_size - bytes_written, bytes_written);
                        }
                    } break;

                }
            } break; /* default */
        }
    }

    return(bytes_written);
}


/* Enum meta data. */

/* Meta Data for: Letters. */
static size_t number_of_elements_in_enum_Letters = 3;
static char const *
enum_to_string_Letters(int v)
{
    switch(v) {
        case 0: { return("letter_a"); } break;
        case 1: { return("letter_b"); } break;
        case 2: { return("letter_c"); } break;

        default: { return(0); } break; /* v is out of bounds. */
    }
}

static int
string_to_enum_Letters(char const *str)
{
    if(0) {}
    else if(strcmp(str, "letter_a") == 0) { return(0); }
    else if(strcmp(str, "letter_b") == 0) { return(1); }
    else if(strcmp(str, "letter_c") == 0) { return(2); }

    else { return(0); }
}


#define GENERATED_H
#endif /* !defined(GENERATED_H) */