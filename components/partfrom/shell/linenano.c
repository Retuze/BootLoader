#include "linenano.h"
#include "printk.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 10
#define LINENOISE_MAX_LINE 256

static linenoiseCompletionCallback* completionCallback = NULL;
static linenoiseHintsCallback* hintsCallback = NULL;
static linenoiseFreeHintsCallback* freeHintsCallback = NULL;
static void refreshLineWithCompletion(struct linenoiseState* ls, linenoiseCompletions* lc, int flags);
static void refreshLineWithFlags(struct linenoiseState* l, int flags);

static int maskmode = 0;

static int history_max_len = LINENOISE_DEFAULT_HISTORY_MAX_LEN;
static int history_len = 0;
static char** history = NULL;
enum KEY_ACTION {
    KEY_NULL = 0,
    CTRL_A = 1,
    CTRL_B = 2,
    CTRL_C = 3,
    CTRL_D = 4,
    CTRL_E = 5,
    CTRL_F = 6,
    CTRL_H = 8,
    TAB = 9,
    CTRL_K = 11,
    CTRL_L = 12,
    ENTER = 13,
    CTRL_N = 14,
    CTRL_P = 16,
    CTRL_T = 20,
    CTRL_U = 21,
    CTRL_W = 23,
    ESC = 27,
    BACKSPACE = 127
};

int linenoiseHistoryAdd(const char* line);
#define REFRESH_CLEAN (1 << 0)
#define REFRESH_WRITE (1 << 1)
#define REFRESH_ALL (REFRESH_CLEAN | REFRESH_WRITE)
static void refreshLine(struct linenoiseState* l);

#define lndebug(fmt, ...)

void linenoiseMaskModeEnable(void)
{
    maskmode = 1;
}
void linenoiseMaskModeDisable(void)
{
    maskmode = 0;
}
void linenoiseClearScreen(void)
{
    printk("\x1b[H\x1b[2J");
}
static void linenanoBeep(void)
{
    printk("\x7");
}
static void freeCompletions(linenoiseCompletions* lc)
{
    size_t i;
    for (i = 0; i < lc->len; i++)
        free(lc->cvec[i]);
    if (lc->cvec != NULL) {
        free(lc->cvec);
    }
}
static void refreshLineWithCompletion(struct linenoiseState* ls, linenoiseCompletions* lc, int flags)
{
    linenoiseCompletions ctable = { 0, NULL };
    if (lc == NULL) {
        completionCallback(ls->buf, &ctable);
        lc = &ctable;
    }
    if (ls->completion_idx < lc->len) {
        struct linenoiseState saved = *ls;
        ls->len = ls->pos = strlen(lc->cvec[ls->completion_idx]);
        ls->buf = lc->cvec[ls->completion_idx];
        refreshLineWithFlags(ls, flags);
        ls->len = saved.len;
        ls->pos = saved.pos;
        ls->buf = saved.buf;
    } else {
        refreshLineWithFlags(ls, flags);
    }
    if (lc != &ctable)
        freeCompletions(&ctable);
}
static int completeLine(struct linenoiseState* ls, int keypressed)
{
    linenoiseCompletions lc = { 0, NULL };
    int nwritten;
    char c = keypressed;
    completionCallback(ls->buf, &lc);
    if (lc.len == 0) {
        linenanoBeep();
        ls->in_completion = 0;
        refreshLine(ls);
        c = 0;
    } else {
        switch (c) {
        case TAB:
            if (ls->in_completion == 0) {
                ls->in_completion = 1;
                ls->completion_idx = 0;
            } else {
                ls->completion_idx = (ls->completion_idx + 1) % (lc.len + 1);
                if (ls->completion_idx == lc.len) {
                    linenanoBeep();
                }
            }
            c = 0;
            break;
        case 27:
            if (ls->completion_idx < lc.len) {
                refreshLine(ls);
            }
            ls->in_completion = 0;
            c = 0;
            break;
        default:
            if (ls->completion_idx < lc.len) {
                nwritten = snprintf(ls->buf, ls->buflen, "%s",
                    lc.cvec[ls->completion_idx]);
                ls->len = ls->pos = nwritten;
            }
            ls->in_completion = 0;
            break;
        }
        if (ls->in_completion && ls->completion_idx < lc.len) {
            refreshLineWithCompletion(ls, &lc, REFRESH_ALL);
        } else {
            refreshLine(ls);
        }
    }
    freeCompletions(&lc);
    return c;
}
void linenoiseSetCompletionCallback(linenoiseCompletionCallback* fn)
{
    completionCallback = fn;
}
void linenoiseSetHintsCallback(linenoiseHintsCallback* fn)
{
    hintsCallback = fn;
}
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback* fn)
{
    freeHintsCallback = fn;
}
void linenoiseAddCompletion(linenoiseCompletions* lc, const char* str)
{
    size_t len = strlen(str);
    char *copy, **cvec;
    copy = malloc(len + 1);
    if (copy == NULL)
        return;
    memcpy(copy, str, len + 1);
    cvec = realloc(lc->cvec, sizeof(char*) * (lc->len + 1));
    if (cvec == NULL) {
        free(copy);
        return;
    }
    lc->cvec = cvec;
    lc->cvec[lc->len++] = copy;
}
struct abuf {
    char* b;
    int len;
};
static void abInit(struct abuf* ab)
{
    ab->b = NULL;
    ab->len = 0;
}
static void abAppend(struct abuf* ab, const char* s, int len)
{
    char* new = realloc(ab->b, ab->len + len);
    if (new == NULL)
        return;
    memcpy(new + ab->len, s, len);
    ab->b = new;
    ab->len += len;
}
static void abFree(struct abuf* ab)
{
    free(ab->b);
}
void refreshShowHints(struct abuf* ab, struct linenoiseState* l, int plen)
{
    char seq[64];
    if (hintsCallback) {
        int color = -1, bold = 0;
        char* hint = hintsCallback(l->buf, &color, &bold);
        if (hint) {
            int hintlen = strlen(hint);
            if (bold == 1 && color == -1)
                color = 37;
            if (color != -1 || bold != 0)
                snprintf(seq, 64, "\033[%d;%d;49m", bold, color);
            else
                seq[0] = '\0';
            abAppend(ab, seq, strlen(seq));
            abAppend(ab, hint, hintlen);
            if (color != -1 || bold != 0)
                abAppend(ab, "\033[0m", 4);
            if (freeHintsCallback)
                freeHintsCallback(hint);
        }
    }
}
static void refreshSingleLine(struct linenoiseState* l, int flags)
{
    char seq[64];
    size_t plen = strlen(l->prompt);
    char* buf = l->buf;
    size_t len = l->len;
    size_t pos = l->pos;
    struct abuf ab;
    abInit(&ab);
    snprintf(seq, sizeof(seq), "\r");
    abAppend(&ab, seq, strlen(seq));
    if (flags & REFRESH_WRITE) {
        abAppend(&ab, l->prompt, strlen(l->prompt));
        if (maskmode == 1) {
            while (len--)
                abAppend(&ab, "*", 1);
        } else {
            abAppend(&ab, buf, len);
        }
        refreshShowHints(&ab, l, plen);
    }
    snprintf(seq, sizeof(seq), "\x1b[0K");
    abAppend(&ab, seq, strlen(seq));
    if (flags & REFRESH_WRITE) {
        snprintf(seq, sizeof(seq), "\r\x1b[%dC", (int)(pos + plen));
        abAppend(&ab, seq, strlen(seq));
    }
    output(ab.b, ab.len);
    abFree(&ab);
}
static void refreshLineWithFlags(struct linenoiseState* l, int flags)
{
    refreshSingleLine(l, flags);
}
static void refreshLine(struct linenoiseState* l)
{
    refreshLineWithFlags(l, REFRESH_ALL);
}
void linenoiseHide(struct linenoiseState* l)
{
    refreshSingleLine(l, REFRESH_CLEAN);
}
void linenoiseShow(struct linenoiseState* l)
{
    if (l->in_completion) {
        refreshLineWithCompletion(l, NULL, REFRESH_WRITE);
    } else {
        refreshLineWithFlags(l, REFRESH_WRITE);
    }
}
int linenoiseEditInsert(struct linenoiseState* l, char c)
{
    if (l->len < l->buflen) {
        if (l->len == l->pos) {
            l->buf[l->pos] = c;
            l->pos++;
            l->len++;
            l->buf[l->len] = '\0';
            if (!hintsCallback) {
                char d = (maskmode == 1) ? '*' : c;
                output(&d, 1);
            } else {
                refreshLine(l);
            }
        } else {
            memmove(l->buf + l->pos + 1, l->buf + l->pos, l->len - l->pos);
            l->buf[l->pos] = c;
            l->len++;
            l->pos++;
            l->buf[l->len] = '\0';
            refreshLine(l);
        }
    }
    return 0;
}
void linenoiseEditMoveLeft(struct linenoiseState* l)
{
    if (l->pos > 0) {
        l->pos--;
        refreshLine(l);
    }
}
void linenoiseEditMoveRight(struct linenoiseState* l)
{
    if (l->pos != l->len) {
        l->pos++;
        refreshLine(l);
    }
}
void linenoiseEditMoveHome(struct linenoiseState* l)
{
    if (l->pos != 0) {
        l->pos = 0;
        refreshLine(l);
    }
}
void linenoiseEditMoveEnd(struct linenoiseState* l)
{
    if (l->pos != l->len) {
        l->pos = l->len;
        refreshLine(l);
    }
}
#define LINENOISE_HISTORY_NEXT 0
#define LINENOISE_HISTORY_PREV 1
void linenoiseEditHistoryNext(struct linenoiseState* l, int dir)
{
    if (history_len > 1) {
        free(history[history_len - 1 - l->history_index]);
        history[history_len - 1 - l->history_index] = strdup(l->buf);
        l->history_index += (dir == LINENOISE_HISTORY_PREV) ? 1 : -1;
        if (l->history_index < 0) {
            l->history_index = 0;
            return;
        } else if (l->history_index >= history_len) {
            l->history_index = history_len - 1;
            return;
        }
        strncpy(l->buf, history[history_len - 1 - l->history_index], l->buflen);
        l->buf[l->buflen - 1] = '\0';
        l->len = l->pos = strlen(l->buf);
        refreshLine(l);
    }
}
void linenoiseEditDelete(struct linenoiseState* l)
{
    if (l->len > 0 && l->pos < l->len) {
        memmove(l->buf + l->pos, l->buf + l->pos + 1, l->len - l->pos - 1);
        l->len--;
        l->buf[l->len] = '\0';
        refreshLine(l);
    }
}
void linenoiseEditBackspace(struct linenoiseState* l)
{
    if (l->pos > 0 && l->len > 0) {
        memmove(l->buf + l->pos - 1, l->buf + l->pos, l->len - l->pos);
        l->pos--;
        l->len--;
        l->buf[l->len] = '\0';
        refreshLine(l);
    }
}
void linenoiseEditDeletePrevWord(struct linenoiseState* l)
{
    size_t old_pos = l->pos;
    size_t diff;
    while (l->pos > 0 && l->buf[l->pos - 1] == ' ')
        l->pos--;
    while (l->pos > 0 && l->buf[l->pos - 1] != ' ')
        l->pos--;
    diff = old_pos - l->pos;
    memmove(l->buf + l->pos, l->buf + old_pos, l->len - old_pos + 1);
    l->len -= diff;
    refreshLine(l);
}

// 初始化一些参数，行编辑的buf，buf大小，buf使用了的大小，提示词，历史记录索引，光标位置，上次光标位置，是否处于自动补全状态，添加一条历史记录
int linenanoEditStart(struct linenoiseState* l, char* buf, size_t buflen, const char* prompt)
{
    l->in_completion = 0;
    l->buf = buf;
    l->buflen = buflen;
    l->prompt = prompt;
    l->oldpos = l->pos = 0;
    l->len = 0;
    l->history_index = 0;
    l->buf[0] = '\0';
    l->buflen--;
    linenoiseHistoryAdd("");
    output(prompt, strlen(prompt));
    return 0;
}

char* linenoiseEditMore = "If you see this, you are misusing the API: when linenoiseEditFeed() is called, if it returns linenoiseEditMore the user is yet editing the line. See the README file for more information.";

char* linenoiseEditFeed(struct linenoiseState* l)
{
    char c = 0;
    int nread;
    char seq[3];
    nread = shell_input(&c, 1);
    if (nread <= 0)
        return NULL;
    if ((l->in_completion || c == TAB) && completionCallback != NULL) {
        c = completeLine(l, c);
        if (c < 0)
            return NULL;
        if (c == 0)
            return linenoiseEditMore;
    }
    switch (c) {
    case ENTER:
        history_len--;
        free(history[history_len]);
        if (hintsCallback) {
            linenoiseHintsCallback* hc = hintsCallback;
            hintsCallback = NULL;
            refreshLine(l);
            hintsCallback = hc;
        }
        return strdup(l->buf);
    case CTRL_C:
        // errno = EAGAIN;
        return NULL;
    case BACKSPACE:
    case 8:
        linenoiseEditBackspace(l);
        break;
    case CTRL_D:
        if (l->len > 0) {
            linenoiseEditDelete(l);
        } else {
            history_len--;
            free(history[history_len]);
            // errno = ENOENT;
            return NULL;
        }
        break;
    case CTRL_T:
        if (l->pos > 0 && l->pos < l->len) {
            int aux = l->buf[l->pos - 1];
            l->buf[l->pos - 1] = l->buf[l->pos];
            l->buf[l->pos] = aux;
            if (l->pos != l->len - 1)
                l->pos++;
            refreshLine(l);
        }
        break;
    case CTRL_B:
        linenoiseEditMoveLeft(l);
        break;
    case CTRL_F:
        linenoiseEditMoveRight(l);
        break;
    case CTRL_P:
        linenoiseEditHistoryNext(l, LINENOISE_HISTORY_PREV);
        break;
    case CTRL_N:
        linenoiseEditHistoryNext(l, LINENOISE_HISTORY_NEXT);
        break;
    case ESC:
        if (shell_input(seq, 1) == -1)
            break;
        if (shell_input(seq + 1, 1) == -1)
            break;
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (shell_input(seq + 2, 1) == -1)
                    break;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '3':
                        linenoiseEditDelete(l);
                        break;
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A':
                    linenoiseEditHistoryNext(l, LINENOISE_HISTORY_PREV);
                    break;
                case 'B':
                    linenoiseEditHistoryNext(l, LINENOISE_HISTORY_NEXT);
                    break;
                case 'C':
                    linenoiseEditMoveRight(l);
                    break;
                case 'D':
                    linenoiseEditMoveLeft(l);
                    break;
                case 'H':
                    linenoiseEditMoveHome(l);
                    break;
                case 'F':
                    linenoiseEditMoveEnd(l);
                    break;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
            case 'H':
                linenoiseEditMoveHome(l);
                break;
            case 'F':
                linenoiseEditMoveEnd(l);
                break;
            }
        }
        break;
    default:
        if (linenoiseEditInsert(l, c))
            return NULL;
        break;
    case CTRL_U:
        l->buf[0] = '\0';
        l->pos = l->len = 0;
        refreshLine(l);
        break;
    case CTRL_K:
        l->buf[l->pos] = '\0';
        l->len = l->pos;
        refreshLine(l);
        break;
    case CTRL_A:
        linenoiseEditMoveHome(l);
        break;
    case CTRL_E:
        linenoiseEditMoveEnd(l);
        break;
    case CTRL_L:
        linenoiseClearScreen();
        refreshLine(l);
        break;
    case CTRL_W:
        linenoiseEditDeletePrevWord(l);
        break;
    }
    return linenoiseEditMore;
}
void linenoiseEditStop(struct linenoiseState* l)
{
    printk("\r\n");
}

// 阻塞行编辑
static char* linenanoBlockingEdit(char* buf, size_t buflen, const char* prompt)
{
    struct linenoiseState l;
    linenanoEditStart(&l, buf, buflen, prompt);
    char* res;
    while ((res = linenoiseEditFeed(&l)) == linenoiseEditMore)
        ;
    linenoiseEditStop(&l);
    return res;
}

void linenoisePrintKeyCodes(void)
{
    char quit[4];
    printk("Linenoise key codes debugging mode.\n"
           "Press keys to see scan codes. Type 'quit' at any time to exit.\n");
    memset(quit, ' ', 4);
    while (1) {
        char c;
        int nread;
        nread = shell_input(&c, 1);
        if (nread <= 0)
            continue;
        memmove(quit, quit + 1, sizeof(quit) - 1);
        quit[sizeof(quit) - 1] = c;
        if (memcmp(quit, "quit", sizeof(quit)) == 0)
            break;
        printk("'%c' %02x (%d) (type quit to exit)\n",
            isprint((int)c) ? c : '?', (int)c, (int)c);
        printk("\r");
    }
}

// 该函数会阻塞住直到按下回车键返回行编辑的结果
char* linenano(const char* prompt)
{
    char buf[LINENOISE_MAX_LINE];
    char* retval = linenanoBlockingEdit(buf, LINENOISE_MAX_LINE, prompt);
    return retval;
}
void linenanoFree(void* ptr)
{
    if (ptr == linenoiseEditMore)
        return;
    free(ptr);
}
int linenoiseHistoryAdd(const char* line)
{
    char* linecopy;
    if (history_max_len == 0)
        return 0;
    if (history == NULL) {
        history = malloc(sizeof(char*) * history_max_len);
        if (history == NULL)
            return 0;
        memset(history, 0, (sizeof(char*) * history_max_len));
    }
    if (history_len && !strcmp(history[history_len - 1], line))
        return 0;
    linecopy = strdup(line);
    if (!linecopy)
        return 0;
    if (history_len == history_max_len) {
        free(history[0]);
        memmove(history, history + 1, sizeof(char*) * (history_max_len - 1));
        history_len--;
    }
    history[history_len] = linecopy;
    history_len++;
    return 1;
}
int linenoiseHistorySetMaxLen(int len)
{
    char** new;
    if (len < 1)
        return 0;
    if (history) {
        int tocopy = history_len;
        new = malloc(sizeof(char*) * len);
        if (new == NULL)
            return 0;
        if (len < tocopy) {
            int j;
            for (j = 0; j < tocopy - len; j++)
                free(history[j]);
            tocopy = len;
        }
        memset(new, 0, sizeof(char*) * len);
        memcpy(new, history + (history_len - tocopy), sizeof(char*) * tocopy);
        free(history);
        history = new;
    }
    history_max_len = len;
    if (history_len > history_max_len)
        history_len = history_max_len;
    return 1;
}