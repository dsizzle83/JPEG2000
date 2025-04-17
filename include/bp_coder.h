#ifndef BP_CODER_H
#define BP_CODER_H

#include <vector>
#include <deque>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iterator>
#include "../include/my_struct.h"
#include "../include/mq_coder.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"
#include "../include/code_block.h"

using namespace std;

class BitPlaneEncoder{
private:
    deque<vector<uint8_t>> code_stream;
    vector<vector<uint8_t>> sig_state;
    vector<vector<uint8_t>> del_sig;
    vector<vector<uint8_t>> member;
    code_block B_i;
    int rows;
    int cols;
    MQEncoder mq_encoder = MQEncoder();
    deque<int> L_z;
    coded_block out_block;
public:
    BitPlaneEncoder(code_block &c_b);
    ~BitPlaneEncoder(){
        code_stream.clear();
        code_stream.resize(0);
        sig_state.clear();
        sig_state.resize(0);
        del_sig.clear();
        del_sig.resize(0);
        member.clear();
        member.resize(0);
        for(auto &bp : B_i.bit_planes){
            bp.plane_data.clear();
            bp.plane_data.resize(0);
        }
        B_i.sign_data.clear();
        B_i.sign_data.resize(0);
        deque<int>().swap(L_z);
    }
    deque<vector<uint8_t>> get_code_stream(){ return code_stream; }
    int get_rows(){ return rows; }
    int get_cols(){ return cols; }
    deque<int> get_lengths(){ return L_z; }
    coded_block get_coded_block(){ return out_block; }
    void init_state_tables();
    uint8_t sig_context(point loc);
    pair<uint8_t, int8_t> sign_context(point loc);
    uint8_t mag_context(point loc);
    void sig_prop_pass(vector<vector<uint8_t>> &stripes, point anchor);
    void encode_sign(point loc);
    void mag_ref_pass(vector<vector<uint8_t>> &stripes, point anchor);
    void clean_up_pass(vector<vector<uint8_t>> &stripes, point anchor);
    bit_plane get_bit_plane(int p);
    vector<vector<uint8_t>> get_stripes(int start_row, int p);
    void encode_code_block();
    void make_coded_block();
    void term_and_append_length();
    void sig_prop_pass_full(int p);
    void mag_ref_pass_full(int p);
    void cleanup_pass_full(int p);
    void snapshot(int pass_count);
    void print_sign_state();
};

class BitPlaneDecoder{
private:
    code_block B_i;
    deque<vector<uint8_t>> code_stream;
    vector<vector<uint8_t>> sig_state;
    vector<vector<uint8_t>> del_sig;
    vector<vector<uint8_t>> member;
    MQDecoder mq_decoder;
    //coded_block in_block;
    point anchor;
    int rows;
    int cols;
    int num_bits;
    int Z_hat;
    deque<int> L_z;
public:
    BitPlaneDecoder(deque<vector<uint8_t>> code_words, int height, int width, int bits, subband_info sb_info, point origin, deque<int> lengths);
    BitPlaneDecoder(coded_block in_block);
    ~BitPlaneDecoder(){
        vector<vector<int8_t>>().swap(B_i.sign_data);
        for(auto &bp: B_i.bit_planes){
            vector<vector<uint8_t>>().swap(bp.plane_data);
        }
        vector<vector<uint8_t>>().swap(sig_state);
        vector<vector<uint8_t>>().swap(del_sig);
        vector<vector<uint8_t>>().swap(member);
        deque<vector<uint8_t>>().swap(code_stream);
        deque<int>().swap(L_z);
    }
    void init_mq_decoder();
    code_block get_code_block(){ return B_i; }
    void init_code_block();
    void init_state_tables();
    void snapshot(int pass_count);
    void recover_codeblock();
    uint8_t sig_context(point loc);
    pair<uint8_t, int8_t> sign_context(point loc);
    uint8_t mag_context(point loc);
    void sig_prop_pass(int p);
    int get_plane_index(int p);
    void decode_sign(point loc);
    void mag_ref_pass(int p);
    void cleanup_pass(int p);
    void resync_decoder();
    void print_sign_state();
};

#endif