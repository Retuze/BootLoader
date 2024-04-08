
#ifndef __LINENOISE_H
#define __LINENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* For size_t. */
#include <stdbool.h>

extern char* linenoiseEditMore;

struct linenoiseState {
    int in_completion; // 标记Tab键，此时输入由completion()处理
    size_t completion_idx; // 下一个补全的索引

    char* buf; // 行编辑缓存
    size_t buflen; // 行编辑缓冲大小
    const char* prompt; // 提示词
    // size_t plen; // 提示词长度，用于多行模式
    size_t pos; // 当前光标位置
    size_t oldpos; // 上次刷新光标位置
    size_t len; // 当前编辑行的长度
    // size_t cols; // 当前终端的长度
    // size_t oldrows; // 最后引用的行，仅用于多行模式
    int history_index; // 正在编辑的历史记录索引
};

typedef struct linenoiseCompletions {
    size_t len;
    char** cvec;
} linenoiseCompletions;

/* Non blocking API. */
int linenoiseEditStart(struct linenoiseState* l,char* buf, size_t buflen, const char* prompt);
char* linenoiseEditFeed(struct linenoiseState* l);
void linenoiseEditStop(struct linenoiseState* l);
void linenoiseHide(struct linenoiseState* l);
void linenoiseShow(struct linenoiseState* l);

/* Blocking API. */
char* linenano(const char* prompt);
void linenanoFree(void* ptr);

/* Completion API. */
typedef void(linenoiseCompletionCallback)(const char*, linenoiseCompletions*);
typedef char*(linenoiseHintsCallback)(const char*, int* color, int* bold);
typedef void(linenoiseFreeHintsCallback)(void*);
void linenoiseSetCompletionCallback(linenoiseCompletionCallback*);
void linenoiseSetHintsCallback(linenoiseHintsCallback*);
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback*);
void linenoiseAddCompletion(linenoiseCompletions*, const char*);

/* History API. */
int linenoiseHistoryAdd(const char* line);
int linenoiseHistorySetMaxLen(int len);
int linenoiseHistorySave(const char* filename);
int linenoiseHistoryLoad(const char* filename);

/* Other utilities. */
void linenoiseClearScreen(void);
void linenoiseSetMultiLine(int ml);
void linenoisePrintKeyCodes(void);
void linenoiseMaskModeEnable(void);
void linenoiseMaskModeDisable(void);

// 由用户提供读取数据的接口
int shell_input(char* buf, char len);

#ifdef __cplusplus
}
#endif

#endif /* __LINENOISE_H */