#ifndef SHELL_H
#define SHELL_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// Shell configuration
typedef struct {
    char prompt[64];
    char history_file[256];
    u32 max_history;
    bool enable_aliases;
    bool enable_completion;
} shell_config_t;

// Shell command structure
typedef struct shell_command {
    char name[64];
    char description[256];
    error_t (*execute)(int argc, char* argv[]);
    struct shell_command* next;
} shell_command_t;

// Shell functions
error_t shell_init(void);
void shell_main_loop(void);
void shell_update_prompt(void);
error_t shell_read_command(void);
error_t shell_execute_command(const char* command);
error_t shell_execute_external(const char* command, const char* args);

// Shell state
void shell_get_cursor(int* x, int* y);
const char* shell_get_current_directory(void);

// Built-in commands
error_t shell_cmd_cd(const char* args);
error_t shell_cmd_ls(const char* args);
error_t shell_cmd_pwd(const char* args);
error_t shell_cmd_cat(const char* args);
error_t shell_cmd_echo(const char* args);
error_t shell_cmd_ps(const char* args);
error_t shell_cmd_kill(const char* args);
error_t shell_cmd_help(const char* args);
error_t shell_cmd_clear(const char* args);
error_t shell_cmd_exit(const char* args);

// Shell utilities
void shell_print_prompt(void);
char* shell_read_line(void);
char** shell_parse_command(const char* line, int* argc);
void shell_free_argv(char** argv, int argc);
void shell_add_to_history(const char* command);
void shell_show_history(void);

#endif // SHELL_H 