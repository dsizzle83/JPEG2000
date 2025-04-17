#ifndef MQ_CODER_H
#define MQ_CODER_H

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include "my_struct.h"

using namespace std;

const struct mq_lut_entry mq_lut[47] ={
    {1,  1,  true,  0x5601},
    {2,  6,  false, 0x3401},
    {3,  9,  false, 0x1801},
    {4,  12, false, 0x0AC1},
    {5,  29, false, 0x0521},
    {38, 33, false, 0x0221},
    {7,  6,  true,  0x5601},
    {8,  14, false, 0x5401},
    {9,  14, false, 0x4801},
    {10, 14, false, 0x3801},
    {11, 17, false, 0x3001},
    {12, 18, false, 0x2401},
    {13, 20, false, 0x1C01},
    {29, 21, false, 0x1601},
    {15, 14, true,  0x5601},
    {16, 14, false, 0x5401},
    {17, 15, false, 0x5101},
    {18, 16, false, 0x4801},
    {19, 17, false, 0x3801},
    {20, 18, false, 0x3401},
    {21, 19, false, 0x3001},
    {22, 19, false, 0x2801},
    {23, 20, false, 0x2401},
    {24, 21, false, 0x2201}, 
    {25, 22, false, 0x1C01},    
    {26, 23, false, 0x1801},
    {27, 24, false, 0x1601},
    {28, 25, false, 0x1401},
    {29, 26, false, 0x1201},
    {30, 27, false, 0x1101},
    {31, 28, false, 0x0AC1},
    {32, 29, false, 0x09C1},
    {33, 30, false, 0x08A1},
    {34, 31, false, 0x0521},
    {35, 32, false, 0x0441},
    {36, 33, false, 0x02A1},
    {37, 34, false, 0x0221},
    {38, 35, false, 0x0141},
    {39, 36, false, 0x0111},
    {40, 37, false, 0x0085},
    {41, 38, false, 0x0049},
    {42, 39, false, 0x0025},
    {43, 40, false, 0x0015},
    {44, 41, false, 0x0009},
    {45, 42, false, 0x0005},
    {45, 43, false, 0x0001},
    {46, 46, false, 0x5601}
};

class MQEncoder{
private:
    uint16_t A;
    uint32_t C;
    uint8_t temp_byte;
    uint8_t t_count;
    long long int L;
    vector<context_table_entry> context_table;
    vector<uint8_t> codeword;

public:
    MQEncoder();
    void reset_encoder();
    vector<context_table_entry> init_context_table();
    void encode(bool x, uint8_t k);
    void code_mps(uint8_t k);
    void code_lps(uint8_t k);
    void transfer_byte();
    void put_byte();
    void easy_term();
    void print_state();
    vector<uint8_t> get_codeword();
};

class MQDecoder{
private:
    uint32_t C;
    uint16_t A;
    uint8_t t_count;
    uint8_t temp_byte;
    uint8_t BL;
    vector<context_table_entry> context_table;
    vector<uint8_t> codeword;
    long long int L;
    long long int L_max;

public:
    MQDecoder(vector<uint8_t> cw);
    MQDecoder();
    void reset_decoder();
    void load_codeword(vector<uint8_t> cw);
    vector<context_table_entry> init_context_table();
    void set_context_table(vector<context_table_entry> cx_table){ context_table = cx_table; }
    void set_l(long long int new_l){ L = new_l;}
    void fill_lsbs();
    bool decode(uint8_t k);
    void renorm_once();
    void print_state();
};

#endif