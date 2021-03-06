#include <iostream>
#include "register.h"
#include "RAM.h"
#include "base.h"
using namespace std;

Register re;
RAM ram;
predict pre;

unsigned npc = 0;

int IF_flag = 1;
int ID_flag = 1;
int EXE_flag = 1;
int MEM_flag = 1;
int WB_flag = 1;//控制流水线的暂停

unsigned EXE_value = 0;
unsigned MEM_value = 0; //实现前传

unsigned before_pre_pc = 0;
unsigned jump_pc = 0;
int reality_flag;
int jump_flag;

int count = 0 , s_cnt = 0, f_cnt = 0;//计数预测

void get_pre_P(){
    if (count == 0) cout << 0;
    else cout << (double) s_cnt/count;
}

IF s1;
ID s2;
EXE s3;
MEM s4;
WB s5;

void predict_judge(){
    if (s3.op == 0b1100011){
        count++;
        if (jump_flag == 1 && reality_flag == 0){
            s1.clear();
            npc = before_pre_pc;
            f_cnt++;
        }
        else if (jump_flag == 0 && reality_flag == 1){
            s1.clear();
            npc = jump_pc;
            f_cnt++;
        }
        else {
            s_cnt++;
        }
    }
}

void deal_hazard(){
    if (s2.rs1 == s4.rd && s4.rd != 0){
        s2.value_rs1 = MEM_value;
    }
    if (s2.rs2 == s4.rd && s4.rd != 0){
        s2.value_rs2 = MEM_value;
    }
    if (s3.op == 0b0000011){ // load_use:流水暂停 + 数据前传 = 将MEM的操作提前一回合
        if (s3.rd == s2.rs1 && s3.rd != 0){
            unsigned data;
            switch (s3.fun3) {
                case 0b000: //lb
                    data = ram.read(1 , s3.pos);
                    if (check(data , 8)) data = data | (0b111111111111111111111111 << 8);
                    s2.value_rs1 = data;
                    break;
                case 0b001: //lh
                    data = ram.read(2 , s3.pos);
                    if (check(data , 16)) data = data | (0b1111111111111111 << 16);
                    s2.value_rs1 = data;
                    break;
                case 0b010: //lw
                    data = ram.read(4 , s3.pos);
                    s2.value_rs1 = data;
                    break;
                case 0b100: //lbu
                    data = ram.read(1 , s3.pos);
                    s2.value_rs1 = data;
                    break;
                default: //lhu
                    data = ram.read(2 , s3.pos);
                    s2.value_rs1 = data;
                    break;
            }
        }
        if (s3.rd == s2.rs2 && s3.rd != 0){
            unsigned data;
            switch (s3.fun3) {
                case 0b000: //lb
                    data = ram.read(1 , s3.pos);
                    if (check(data , 8)) data = data | (0b111111111111111111111111 << 8);
                    s2.value_rs2 = data;
                    break;
                case 0b001: //lh
                    data = ram.read(2 , s3.pos);
                    if (check(data , 16)) data = data | (0b1111111111111111 << 16);
                    s2.value_rs2 = data;
                    break;
                case 0b010: //lw
                    data = ram.read(4 , s3.pos);
                    s2.value_rs2 = data;
                    break;
                case 0b100: //lbu
                    data = ram.read(1 , s3.pos);
                    s2.value_rs2 = data;
                    break;
                default: //lhu
                    data = ram.read(2 , s3.pos);
                    s2.value_rs2 = data;
                    break;
            }
        }
    }
    else {
        if (s2.rs1 == s3.rd && s3.rd != 0){
            s2.value_rs1 = EXE_value;
        }
        if (s2.rs2 == s3.rd && s3.rd != 0){
            s2.value_rs2 = EXE_value;
        }
    }
}

void Run(){
    ram.init();
    while (WB_flag){
        s5.Write_back(s4);
        s4.Memory(s3);
        s3.Execute(s2);
        predict_judge();//分支预测的回滚
        s2.Decode(s1);
        deal_hazard();//处理数据hazard
        s1.Fitch();
        if (s1.code == 0x0ff00513) IF_flag = 0; //0ff00513为终止
    }
    cout << (re[10] & 255u);
}

int main() {
    Run();
//    cout << "\t";
//    get_pre_P();
    return 0;
}
