#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "partfrom.h"
#include "printk.h"

#define CMD_ARG uint8_t argc, char *argv[]
#define VAR(var) #var, &var

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

class SHELL {
private:
    struct cmd_list {
        const char* cmd;
        const char* help;
        void (*fun)(uint8_t argc, char* argv[]);
        struct list_head list;
    };
    struct list_head cmd_entry;
    struct var_list {
        const char* name;
        int* pvar;
        struct list_head list;
    };
    struct list_head var_entry;
    uint8_t split_string(char* str, const char* delim, char*** pargv)
    {
        char* token = nullptr;
        uint8_t idx = 0;
        token = strtok(str, delim);
        while (token) {
            if (idx >= 3) {
                *pargv = (char**)realloc(*pargv, (idx + 1) * sizeof(char*));
            }
            (*pargv)[idx++] = token;
            token = strtok(nullptr, delim);
        }
        return idx;
    }

public:
    SHELL()
    {
        INIT_LIST_HEAD(&cmd_entry);
        INIT_LIST_HEAD(&var_entry);
    }
    void help(CMD_ARG)
    {
        printk("支持的命令：\n");
        printk(" %-12s%s\n", "help", "列出所有支持的命令");
        printk(" %-12s%s\n", "var", "读写导出的变量");
        struct cmd_list* node = NULL;
        list_for_each_entry(node, &cmd_entry, list)
        {
            if (node->help == NULL) {
                printk(" %-12s\n", node->cmd);
            } else {
                printk(" %-12s%s\n", node->cmd, node->help);
            }
        }
    }
    void var(CMD_ARG)
    {
        if (argc == 1) {
            printk("导出的所有变量\r\n");
            struct var_list* node = NULL;
            list_for_each_entry(node, &var_entry, list)
            {
                printk("%-6s = %d\r\n", node->name, *(node->pvar));
            }
        } else if (argc == 2) {
            struct var_list* node = NULL;
            list_for_each_entry(node, &var_entry, list)
            {
                if (strcmp(node->name, argv[1]) == 0) {
                    printk("%-6s = %d\r\n", node->name, *(node->pvar));
                    return;
                }
            }
            printk("该变量未导出或不存在\r\n");
        } else if (argc == 4 && strcmp(argv[2], "=") == 0) {
            struct var_list* node = NULL;
            list_for_each_entry(node, &var_entry, list)
            {
                if (strcmp(node->name, argv[1]) == 0) {
                    *(node->pvar) = atoi(argv[3]);
                    printk("%-6s = %d\r\n", node->name, *(node->pvar));
                    return;
                }
            }
            printk("该变量未导出或不存在\r\n");
        } else {
            printk("命令参数错误!!!\r\n");
        }
    }
    void add_var(const char* name, int* pvar)
    {
        auto obj = (struct var_list*)malloc(sizeof(struct var_list));
        obj->name = name;
        obj->pvar = pvar;
        list_add(&obj->list, &var_entry);
    }
    void add_cmd(const char* str, void (*fun)(uint8_t argc, char* argv[]))
    {
        auto obj = (struct cmd_list*)malloc(sizeof(struct cmd_list));
        obj->help = NULL;
        obj->fun = fun;
        obj->cmd = str;
        list_add(&obj->list, &cmd_entry);
    }
    void add_cmd(const char* str, const char* help_str, void (*fun)(uint8_t argc, char* argv[]))
    {
        auto obj = (struct cmd_list*)malloc(sizeof(struct cmd_list));
        obj->fun = fun;
        obj->cmd = str;
        obj->help = help_str;
        list_add(&obj->list, &cmd_entry);
    }

    /// @brief 该函数会改变str，如果不希望str改变请使用exec(const char *str)
    /// @param str
    void exec(char* str)
    {
        if (*str == '\0' || str == NULL) {
            return;
        }
        auto argv = (char**)malloc(sizeof(char*) * 3);
        auto argc = split_string(str, " (),{}", &argv);
        if (strcmp(argv[0], "help") == 0) {
            help(argc, argv);
            free(argv);
            return;
        } else if (strcmp(argv[0], "var") == 0) {
            var(argc, argv);
            free(argv);
            return;
        }
        struct cmd_list* node = NULL;
        list_for_each_entry(node, &cmd_entry, list)
        {
            if (strcmp(argv[0], node->cmd) == 0) {
                node->fun(argc, argv);
                free(argv);
                return;
            }
        }
        printk("unkonw cmd %s !!!\n", argv[0]);
        free(argv);
    }

    /// @brief 安全的命令解析函数
    /// @param str
    void exec(const char* str)
    {
        auto temp = (char*)malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(temp, str);
        exec(temp);
        free(temp);
    }

    int prefix_match(const char* prefix, const char* buf[])
    {
        if (*prefix == '\0' || prefix == NULL) {
            help(0, NULL);
            return 0;
        } else {
            struct cmd_list* node = NULL;
            auto index = buf;
            if (!strncmp(prefix, "help", strlen(prefix))) {
                *(buf++) = "help";
            } else if (!strncmp(prefix, "var", strlen(prefix))) {
                *(buf++) = "var";
            }
            list_for_each_entry(node, &cmd_entry, list)
            {
                if (!strncmp(prefix, node->cmd, strlen(prefix))) {
                    *(buf++) = node->cmd;
                }
            }
            return buf - index;
        }
    }
};

#endif

extern SHELL shell;

#endif // _SHELL_H_