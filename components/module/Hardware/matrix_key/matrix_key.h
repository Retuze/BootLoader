#ifndef _MATRIX_KEY_H_
#define _MATRIX_KEY_H_

#include "Arduino.h"
#include "key.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
// 采用线反法扫描，io口设置为准双向口模式(开漏输出，加上拉电阻)
class MATRIX_KEY {
    int scan_code = 0xffffffff;
    enum row_or_col {
        ROW,
        COL,
    };
    int (*set_get_key)(enum row_or_col row_or_col, int num, int val);
    enum {
        SET_LOW,
        SET_HIGH,
        GET_NONE, // 表示读取
    };
    enum {
        UP,
        DOWN,
    };
    int row;
    int col;
    KEY* key;
    MATRIX_KEY(int row, int col, int (*set_get_key)(enum row_or_col row_or_col, int num, int val))
    {
        this->row = row;
        this->col = col;
        this->set_get_key = set_get_key;
        key = new KEY[row * col];
        for (int i = 0; i < (row * col); i++) {
            key[i] = KEY([]() -> uint8_t { return 0; });
        }
    }
    void scan()
    {
        // 先把行线全部拉高，列线全部拉低，读取行线电平，如果有低电平则认为有按键按下，可确认按下的键所在行
        int row_code = 0xffffffff;
        int col_code = 0xffffffff;
        for (int i = 0; i < row; i++) {
            set_get_key(ROW, i, SET_HIGH);
        }
        for (int i = 0; i < col; i++) {
            set_get_key(COL, i, SET_LOW);
        }
        for (int i = 0; i < row; i++) {
            bitWrite(row_code, i, set_get_key(ROW, i, GET_NONE));
        }
        // 再把列线全部拉高，行线全部拉低，读取列线电平，如果有低电平则认为有按键按下，可确认按下的键所在列
        for (int i = 0; i < col; i++) {
            set_get_key(COL, i, SET_HIGH);
        }
        for (int i = 0; i < row; i++) {
            set_get_key(ROW, i, SET_LOW);
        }
        for (int i = 0; i < col; i++) {
            bitWrite(col_code, i, set_get_key(COL, i, GET_NONE));
        }
        scan_code = row_code & col_code;
    }
    int read_key(int row, int col)
    {
        if (bitRead(scan_code, col) == 0 && bitRead(scan_code, 16 + row) == 0) {
            return DOWN;
        } else {
            return UP;
        }
    }
};

#endif

#endif // _MATRIX_KEY_H_