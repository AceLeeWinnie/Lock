//
//  main.c
//  Lock
//
//  Created by ace on 15/10/16.
//  Copyright (c) 2015年 ace. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <regex.h>
#include "SmartLock.h"

//当前系统状态
enum state curState = FREE;
//允许错误次数
byte errortimes = 3;

//号码锁定文件
char* locked_number_file = "/etc/lockednumber";
//公式存储文件
char* formula_file = "/etc/formula";



int timer = 3000;
char* formula_format_tip = "Please input a new formula again including 3 variables(x,y,z) at most, 1 integer(1 - 9) at least and 4 operator(+, -, *, /) at most. eg. x+y-9";
char* formula_format_wrong_tip = "Your formula format is wrong.Please input a new formula again including 3 variables(x,y,z) at most, 1 integer(1 - 9) at least and 4 operator(+, -, *, /) at most. eg. x+y-9";
char* set_formula_correct_tip="Formula is set correctly.";
char* set_formula_wrong_tip="Formula is set wrong. Please try again";
char* add_validate_correct_tip="Please input the number you want to add.";
char* add_number_correct_tip="Number is added correctly.";
char* add_number_wrong_tip="Number is added wrong. Please try again.";
char* del_validate_correct_tip="Please input the number you want to delete.";
char* del_number_correct_tip="Number is deleted correctly.";
char* del_number_wrong_tip="Number is deleted wrong. Please try again.";
char* open_wrong_tip="Open door error. Please try again.";
char* password_wrong_tip="Your password is wrong, please try again.";
char* lock_tip="Your number is locked,please try tomorrow.";
char* operating_tip="Your family is operating. ";
char* number_isn_exist="The number you want to delete isn't exist.Please try again.";



/*
 * 初始化GSM模块
 */
void initGSM() {
    serialBegin(9600);
    TurnOn(9600);          		//module power on
    InitParam(PARAM_SET_1);		//configure the module
    Echo(1);               		//enable AT echo
}


/*
 *isReceived有没有收到短信
 *有则记住号码并且记住短信内容
 *返回 NOT_IN_WHITELIST 非白名单
 *    LOCK_NUMBER 被锁住的号码
 *    WAITING_NUMBER 正在等待的号码
 *    IN_WHITE_LIST 在白名单中但不在等待
 *返回 0 没收到
 *    1 收到
 *
 *
 */
bool isReceived(char* number, char* input) {
    char position = IsSMSPresent(SMS_UNREAD);
    if (position) {
        GetSMS(position, number, input, 140);
        return true;
    }
    return false;
};
/*
 *outTips提示
 *参数：tips提示信息，number发送的号码
 *返回：0 发送失败
 *     1 发送成功
 */
byte outTips(char* tips, char* number) {
    return SendSMS(number, tips);
};
/*
 *isTrusted是否是信任号码
 *参数：number号码
 *返回：电话本查询结果 
 *      TRUSTED信任
 *      UNTRUSTED不信任
 */
bool isTrusted(char* number){
    int i;
    bool ret_val = false;
    //获取电话本长度
    int numberlength = GetUsedPhoneNumberLength();
    //挨个比较电话本中的电话号码
    for (i = 0; i < numberlength; ++i) {
        //匹配成功
        if (ComparePhoneNumber(i, number) == 1) {
            ret_val = true;
        }
    }
    return ret_val;
};
/*
 *isLocked是否是被锁号码
 *参数：number号码
 *查询号码是否内存解锁列表中
 *如在查看解锁日期是否到，到的话在列表中删除并返回UNLOCKED
 *日期未到则返回LOCKED
 *不在返回UNLOCKED
 *返回：
 * LOCKED
 * UNLOCKED
 */
bool isLocked(char* number){
    //是否找到号码
    bool findflag = false;
    //锁住的号码
    char lockednumber[12];
    //锁住的截止时间
    long lockedtime;
    //当前毫秒数
    time_t now;
    time(&now);
    //返回值
    bool ret_val = false;
    char* temp = "/etc/lockednumber_temp";
    //读取/etc/lockednumber文件
    FILE* sourcefp = fopen(locked_number_file, "r+");
    FILE* tempfp = fopen(temp, "w+");
    if (!sourcefp || !tempfp) {
        perror("File opening failed");
    } else {
        //遍历文件
        while (fscanf(sourcefp, "%s %ld ", lockednumber, &lockedtime)) {
            //没找到号码
            if (!findflag) {
                //号码匹配
                if (strcmp(number, lockednumber)) {
                    findflag = true;
                    //时间超过
                    if (now - lockedtime > 86400) {
                        break;
                    } else {
                        ret_val = true;
                    }
                }
            }
            //复制
            fprintf(tempfp, "%s %ld ", lockednumber, lockedtime);
        }
        //删除源文件
        remove(locked_number_file);
        //重命名
        rename(temp, locked_number_file);
        fclose(sourcefp);
        fclose(tempfp);
    }
    return ret_val;
};

/*
 *isWaitingNumber 是不是正在等待的号码
 *参数 number 现在进入的号码
 *返回 
 *true 是
 *false 不是
 */
bool isWaitingNumber (char* coming, char* waiting) {
    
};
/*
 *stopTimer停止计时器 多线程?
 *
 */
void stopTimer() {
    
};

/*
 *lockNumber锁定某一号码
 *设置过期日期
 *返回
 *true 设置失败
 *false 设置成功
 */
bool lockPhoneNumber(char* number){
    bool ret_val = false;
    time_t now;
    time(&now);
    FILE* fp = NULL;
    //打开文件 /etc/locknumber
    fp = fopen(locked_number_file, "a+");
    //打开成功
    if (fp) {
        //写入 1884418**** 1446024656
        fprintf(fp,"%s %ld ",number, now);
        //成功
        ret_val = true;
    }
    //关闭文件
    fclose(fp);
    return ret_val;
}
/*
 *nextTip 根据当前状态返回提示信息
 *参数 curState 当前状态
 *返回 char* nextTip 指针
 */
char* nextTip(byte curState) {
    char* ret_val = NULL;
    switch (curState) {
        case SET_VALIDATE :
            ret_val = formula_format_tip;
            break;
        case ADD_VALIDATE :
            ret_val = add_validate_correct_tip;
            break;
        case DEL_VALIDATE :
            ret_val = del_validate_correct_tip;
            break;
        default:
            break;
    }
    return ret_val;
}
/*
 *outRandom 输出随机数
 *参数 ret 得数
 *返回 随机数字符串如 "2 3 5"
 *outRandom中需要产生随机数并进行预计算 返回的是能够计算的随机数的字符串 预计算所得结果存储在equation_ans中
 */
char* outRandom(int* ret) {
    char* random_number = NULL;
    //读取文件
    FILE* fp = fopen(formula_file, "r+");
    if (!fp) {
        perror("Formula store file open failed");
    } else {
        
    }
}
/*
 *nextState 确定下一阶段状态
 *参数 curState 当前状态
 *返回 nextState 下一状态
 *enum state排列如下
 *FREE-SET-SET_VALIDATE—SET_FORMULA-ADD-ADD_VALIDATE-ADD_NUMBER-DEL-DEL_VALIDATE-DEL_NUMBER-OPEN-OPEN_VALIDATE
 */
byte nextState(byte curState) {
    byte nextState = FREE;
    switch (curState) {
        case SET:
        case SET_VALIDATE:
        case OPEN:
        case ADD:
        case ADD_VALIDATE:
        case DEL:
        case DEL_VALIDATE:
            nextState = curState + 1;
            break;
        default:
            break;
    }
    return nextState;
}
/*
 *validateResult 验证答案
 *返回
 *true 正确
 *false 错误
 */
bool validateResult(char* input, int expectanswer) {
    bool ret_val = false;
    char buf[10];
    sprintf(buf, "%d", expectanswer);
    //比较expectresult与input
    if (strcmp(input, buf) == 0) {
        //正确 true
        ret_val = true;
    }
    return ret_val;
}

/*
 *validateFormula 验证公式
 *返回
 *true 正确
 *false 错误
 */
bool validateFormula(char* formula) {
    bool ret_val = false;
    int status;
    int cflags = REG_EXTENDED;
    regmatch_t pmatch[1];
    const size_t nmatch = 1;
    regex_t reg;
    const char* pattern = "^[\\dXxYyZz](([+*-][\\dXxYyZz])|(/[1-9XxYyZz]))*$";
    
    regcomp(&reg, pattern, cflags);
    status = regexec(&reg, formula, nmatch, pmatch, 0);
    if (status == 0) {
        ret_val = true;
    }
    regfree(&reg);
    return ret_val;
}
/*
 *validatePhoneNumber 验证电话号码
 *返回
 *true
 *false
 */
bool validatePhoneNumber(char* number) {
    bool ret_val = false;
    int status;
    int cflags = REG_EXTENDED;
    regmatch_t pmatch[1];
    const size_t nmatch = 1;
    regex_t reg;
    const char* pattern = "^[0-9]{11}$";
    
    regcomp(&reg, pattern, cflags);
    status = regexec(&reg, number, nmatch, pmatch, 0);
    if (status == 0) {
        ret_val = true;
    }
    regfree(&reg);
    return ret_val;
}

/*
 *openDoor调用开门python指令
 *返回
 *true 成功
 *false 失败
 */
bool openDoor() {
    char* opencom = "sudo python ./led.py";
    FILE* fp;
    bool ret_val = false;
    if((fp=popen(opencom,"r"))!=NULL){
        pclose(fp);
        ret_val = true;
    }
    return ret_val;
}
/*
 * 主函数 重复循环执行
 */
int main(int argc, const char * argv[]) {
    //当前处理的号码和输入
    char* number_incoming = NULL;
    char* input = NULL;
    char number_waiting[13];
    int* equation_ans;
    //设置GSM相关参数
    initGSM();
    while (1) {
        //isReceived有没有收到短信
        //有则记住号码并且记住短信内容
        while (!isReceived(number_incoming, input));
        //不在信任列表或者被锁住都不返回信息
        if(!isTrusted(number_incoming) || isLocked(number_incoming)){
            continue;
        };
        //不是自由状态，是正在等待的号码，计时器停止并继续
        //不是自由状态，不是正在等待的号码，返回您的家人正在操作，请稍后再试
        //是自由状态 继续
        if (curState != FREE) {
            if (isWaitingNumber(number_incoming, number_waiting)) {
                stopTimer(timer);
            } else {
                outTips(operating_tip, number_incoming);
            }
        }
        //是否是正在等待的信息
        //不是则返回期望信息提示
        //是则进入下一步
        if(!isExpected(curState, input)){
            //根据当前期望返回提示并启动错误次数计数器
            //错误次数超过三次返回锁定信息,锁定号码
            if(errortimes == 0){
                outTips(lock_tip, number_incoming);
                lockPhoneNumber(number_incoming);
                continue;
            }
            --errortimes;
        };
        //状态自由则根据输入进入相应状态 设置当前号码为waitingnumber
        if(curState == FREE) {
            strcpy(number_waiting, number_incoming);
            curState = changeSTB(input);
        }
        switch (curState) {
            case SET:
            case ADD:
            case OPEN:
            case DEL:
                //发送随机数
                //outRandom中需要产生随机数并进行预计算 返回的是能够计算的随机数的字符串 预计算所得结果存储在equation_ans中
                outTips(outRandom(equation_ans), number_incoming);
                //进入curState_VALIDATE状态
                curState =  nextState(curState);
                //定时,返回定时器句柄
                setTimer(timer);
                break;
            case SET_VALIDATE:
            case ADD_VALIDATE:
            case DEL_VALIDATE:
                //用户输入与计算结果不符并且错误三次锁住号码
                if (validateResult(input, equation_ans)){
                    //根据当前状态输出相应下一步提示信息
                    outTips(nextTip(curState), number_incoming);
                    //进入相应的下一步状态
                    curState = nextState(curState);
                }else if (errortimes == 0) {
                    outTips(lock_tip, number_incoming);
                    lockPhoneNumber(number_incoming);
                }else {
                    --errortimes;
                    outTips(password_wrong_tip, number_incoming);
                    //定时
                    setTimer(timer);
                }
                
                break;
            case OPEN_VALIDATE:
                if (validateResult(input, equation_ans)) {
                    //调用开锁程序正确
                    if(openDoor()) {
                        //恢复默认状态 errortimes为3，curState为FREE, 清空number input,清空等待的号码
                        initParam();
                    }else {
                        --errortimes;
                        outTips(open_wrong_tip, number_incoming);
                    };
                    
                } else if (errortimes == 0) {
                    outTips(lock_tip, number_incoming);
                    lockPhoneNumber(number_incoming);
                }else {
                    --errortimes;
                    outTips(password_wrong_tip, number_incoming);
                    //定时
                    setTimer(timer);
                }
                break;
            case SET_FORMULA:
                //公式格式验证正确
                //存储公式
                //输出正确信息
                //初始化
                //错误
                //错误次数？
                if(validateFormula(input)){
                    //设置公式
                    if (setFormula(input)) {
                        //输出 set correctly
                        outTips(set_formula_correct_tip, number_incoming);
                        initParam();
                    } else {
                        --errortimes;
                        outTips(set_formula_wrong_tip, number_incoming);
                    }
                } else if (errortimes == 0) {
                    outTips(lock_tip, number_incoming);
                    lockPhoneNumber(number_incoming);
                } else {
                    --errortimes;
                    outTips(formula_format_wrong_tip, number_incoming);
                    setTimer(timer);

                }
                break;
            case ADD_NUMBER:
                if (validatePhoneNumber(input)) {
                    if(addPhoneNumber(input)) {
                        outTips(add_number_correct_tip, number_incoming);
                        initParam();
                    } else {
                        outTips(add_number_wrong_tip, number_incoming);
                        --errortimes;
                    };
                } else if (errortimes == 0) {
                    outTips(lock_tip, number_incoming);
                    lockPhoneNumber(number_incoming);
                }else {
                    --errortimes;
                    outTips(number_isn_exist, number_incoming);
                    //定时
                    setTimer(timer);
                }
                break;
            
           
            case DEL_NUMBER:
                if (validatePhoneNumber(input)) {
                    if(delPhoneNumber(input)) {
                        outTips(del_number_correct_tip, number_incoming);
                        initParam();
                    } else {
                        outTips(del_number_wrong_tip, number_incoming);
                        --errortimes;
                    };
                } else if (errortimes == 0) {
                    outTips(lock_tip, number_incoming);
                    lockPhoneNumber(number_incoming);
                }else {
                    --errortimes;
                    outTips(number_isn_exist, number_incoming);
                    //定时
                    setTimer(timer);
                }
                break;
            default:
                break;
        }
    }
    return 0;
}
