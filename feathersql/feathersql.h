#ifndef FEATHERSQL_H
#define FEATHERSQL_H


#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)
#define TODO(message) do { fprintf(stderr, "%s:%d TODO: %s\n", __FILE__, __LINE__, message);} while (0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while (0)
#define da_realloc(da)\
            (da)->items = \
                realloc((da)->items, (da)->capacity * sizeof(*(da)->items))

#define DA_INIT_CAPACITY 256
#define da_append(da, item)\
    do {\
        if ((da)->capacity == 0) {\
            (da)->capacity = DA_INIT_CAPACITY;\
        } else if ((da)->count + 1 > (da)->capacity) {\
            (da)->capacity *= 2;\
        }\
        da_realloc(da);\
        (da)->items[(da)->count++] = (item);\
    } while (0)

#define da_last(da) (da)->items[(da)->count-1]
#define da_pop(da)\
    do {\
        (da)->count--;\
    } while (0)

#define da_remove(da, idx)\
    do {\
        for (size_t index = (idx); index + 1 < (da)->count; ++index) {\
            (da)->items[index] = (da)->items[index + 1];\
        }\
        --(da)->count;\
    } while (0)
#define da_remove_unordered(da, i)\
    do {\
        size_t j = (i);\
        (da)->items[j] = (da)->items[--(da)->count];\
    } while(0)

#define da_foreach(da, Type, it)\
    for (Type *it = (da)->items; it < (da)->items + (da)->count; ++it)

#define da_copy(da_a, da_b) \
    do {\
        (da_a)->capacity = (da_b)->capacity; \
        (da_a)->count = (da_b)->count; \
        da_realloc((da_a)); \
        memcpy((da_a)->items, (da_b)->items, (da_a)->count); \
    } while (0)


void feather_repl(void);
void feather_run(const char* filepath);
void feather_usage(void);

#ifdef FEATHER_IMPLEMENTATION

typedef struct {
    char* items;
    size_t count;
    size_t capacity;
} StringView;

typedef struct {
    StringView* items;
    size_t count;
    size_t capacity;
} Commands;

static bool ESCAPABLE[128] = {
    ['a'] = true,
    ['b'] = true,
    ['e'] = true,
    ['f'] = true,
    ['n'] = true,
    ['r'] = true,
    ['t'] = true,
    ['v'] = true,
    ['\\'] = true,
    ['\''] = true,
    ['"'] = true,
};

static void clear_stdin(void) {
    char c;
    do {
        c = fgetc(stdin);
    } while (c != '\n' && c != EOF);
}

static void clear_command(StringView *command) {
    command->count = 0;
}

static void push_command(Commands *commands, StringView *command) {
    StringView cmd = {0};
    da_copy(&cmd, command);
    da_append(commands, cmd);
}

static bool run_commands(Commands *commands) {
    da_foreach(commands, StringView, sv) {
        char *s = malloc(sizeof(char) * (sv->count + 1));
        memcpy(s, sv->items, sv->count);
        s[sv->count] = '\0';
        if (s[0] == 'q') return true;
        printf("[COMMAND START]\n|%s|\n[COMMAND END]\n", s);
    }
    return false;
}

static void SV_append_char(StringView *sv, char c) {
    da_append(sv, c);
}


void feather_repl(void) {
    bool should_quit = false;
    while (!should_quit) {
        printf("> ");
        bool finish_command = false;
        bool in_string = false;
        bool should_run_commands = false;
        Commands commands = {0};
        StringView command = {0};
        while (!should_run_commands) {
            unsigned int c = fgetc(stdin);
            char to_append = '\0';
            char string_char = '\0';
            switch (c) {
                case ' ':  /* fallthrough */
                case '\t': /* fallthrough */
                case '\n': /* fallthrough */
                case '\v': /* fallthrough */
                case '\f': /* fallthrough */
                case '\r':
                    if (!in_string && c == '\n' && finish_command) {
                        should_run_commands = true;
                        ungetc(c, stdin);
                        break;
                    }
                    goto L_append_char;
                case ';':
                    if (in_string)
                        goto L_append_char;
                    finish_command = true;
                    push_command(&commands, &command);
                    clear_command(&command);
                    break;
                case '\\':
                    {
                        char cc = fgetc(stdin);
                        if (cc < 128 && ESCAPABLE[cc]) {
                            SV_append_char(&command, '\\');
                            c = cc;
                        } else {
                            ungetc(cc, stdin);
                        }
                        goto L_append_char;
                    }
                case '"': /* fallthrough */
                case '\'':
                    if (in_string && c == string_char) {
                        in_string = false;
                    } else {
                        string_char = c;
                        in_string = true;
                    }
                    goto L_append_char;
                default:
/* LABEL */     L_append_char:
                    finish_command = false;
                    to_append = (char)c;
                    break;
            }
            if (to_append) {
                SV_append_char(&command, to_append);
            }
            if (should_run_commands) break;
        }
        clear_stdin();
        should_quit = run_commands(&commands);
    }
}

void feather_run(const char* filepath) {
    UNUSED(filepath);
    TODO(__func__);
}

void feather_usage(void) {
    fprintf(stderr, "Usage: feathersql [filename]\n");
}


#endif // FEATHER_IMPLEMENTATION

#endif // FEATHERSQL_H
