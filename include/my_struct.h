#ifndef MY_STRUCT_H
#define MY_STRUCT_H

#include <stdint.h>
#include <vector>
#include <deque>

using namespace std;

typedef struct mq_lut_entry{ 
    uint8_t mps;
    uint8_t lps;
    bool x_s;
    uint16_t p;
}mq_lut_entry;

typedef struct context_table_entry{
    uint8_t lut_entry;
    bool s_k;
}context_table_entry;

// Used to identify position of tile
typedef struct point{
    int x;
    int y;
}point;

typedef struct subband_info{
    int type; // 0 for LL, 1 for HL, 2 for LH, 3 for HH
    point top_l;
    point bot_r;
    double step_size;
    int mant;
    int expo;
    int level;
    int bit_depth;
}subband_info;

typedef struct bit_plane{
    vector<uint8_t> plane_data;
    int bit_level;
}bitplane;

typedef struct code_block{
    point anchor;
    subband_info s_b;
    vector<bit_plane> bit_planes;
    vector<int8_t> sign_data;
    int num_bits;
    int height;
    int width;
}code_block;

typedef struct cb_sample{
    uint8_t context;
    bool symbol;
    point location;
}cb_sample;

typedef struct coded_block{
    deque<vector<uint8_t>> code_words;
    int height;
    int width;
    int bits;
    point anchor;
    subband_info sb_info;
    deque<int> lengths;
    deque<pair<int, double>> rate_dis;
}coded_block;

typedef struct quality_layer{
    vector<coded_block> coded_blocks;
    long unsigned int max_length;
    int layer_number;
    double min_slope;
}quality_layer;

typedef struct tile{
    vector<double> tile_data;
    point anchor;
    size_t height;
    size_t width;
    deque<coded_block> enc_data;
}tile;

#endif