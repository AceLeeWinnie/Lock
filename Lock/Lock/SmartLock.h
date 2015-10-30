//
//  SmartLock.h
//  Lock
//
//  Created by ace on 15/10/16.
//  Copyright (c) 2015年 ace. All rights reserved.
//

#include "GSM.h"

//状态
enum state {
    FREE,
    SET,
    SET_VALIDATE,
    SET_FORMULA,
    ADD,
    ADD_VALIDATE,
    ADD_NUMBER,
    DEL,
    DEL_VALIDATE,
    DEL_NUMBER,
    OPEN,
    OPEN_VALIDATE
};

//辅助函数
//判断是否信任
bool isTrusted (char *number);
//判断是否是期望输入
byte isExpected(byte state, char *input);
//产生随机数
int generate();
//预计算
char* preCal(int *var, char* formula);
//转变当前状态
byte changeCurState(byte state);
//定时
int* setTimer(int millis);
//取消定时
byte stopTimer(int* timer);
//把字符串对应状态
byte changeSTB(char *s);
//是否接收到用户输入
bool isReceived(char* number, char* input);
//输出信息



//处理函数
//分析用户输入
byte analysisUser();
//开门
bool openDoor();
//添加电话
byte addNumber(char *number);
//删除电话
byte delNumber(char *number);


