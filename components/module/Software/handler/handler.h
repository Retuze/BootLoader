#ifndef _HANDLER_H_
#define _HANDLER_H_

#include "list.h"
#include "partfrom.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

class Handler {
private:
    struct Message {
        void (*fun)(void* arg);
        void* parm;
        int rept;
        const char* name;
        struct list_head list;
    };
    struct list_head entry;

public:
    Handler()
    {
        INIT_LIST_HEAD(&entry);
    }
    void post(void (*fun)(void* arg), void* parm = NULL, const char* name = NULL, int rept = 1)
    {
        auto obj = (struct Message*)malloc(sizeof(struct Message));
        obj->fun = fun;
        obj->parm = parm;
        obj->rept = rept;
        obj->name = name;
        list_add(&obj->list, &entry);
    }
    void remove(const char* name)
    {
        struct Message* node = NULL;
        struct Message* n = NULL;
        list_for_each_entry_safe(node, n, &entry, list)
        {
            if (strcmp(node->name, name) == 0) {
                list_del(&node->list);
                free(node);
            }
        }
    }
    void show(void)
    {
        struct Message* node = NULL;
        list_for_each_entry(node, &entry, list)
        {
            printk("任务名称：%s 任务剩余执行次数：%d \n", node->name, node->rept);
        }
    }
    void loop()
    {
        struct Message* node = NULL;
        struct Message* n = NULL;
        list_for_each_entry_safe(node, n, &entry, list)
        {
            if (node->rept > 0) {
                node->rept--;
                node->fun(node->parm);
            } else if (node->rept < 0) {
                node->fun(node->parm);
            } else {
                list_del(&node->list);
                free(node);
            }
        }
    }
};

#endif

#endif // _HANDLER_H_
