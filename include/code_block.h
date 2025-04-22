#ifndef CODE_BLOCK_H
#define CODE_BLOCK_H

#include "my_struct.h"
#include <vector>

using namespace std;

class CodeBlocks{
private:
    vector<pair<uint16_t, int8_t>> quant_coeffs;
    vector<subband_info> band_info;
    vector<code_block> blocks;
    size_t max_height = 64;
    size_t max_width = 64;
    int rows;
    int cols;
public:
    CodeBlocks(vector<pair<uint16_t, int8_t>> &q, vector<subband_info> &sb_info, int height, int width);
    CodeBlocks(vector<code_block> &cb, int height, int width);
    vector<subband_info> get_band_info(){ return band_info; }
    vector<code_block> get_code_blocks(){ return blocks; }
    vector<pair<uint16_t, int8_t>> get_quant_coeffs(){ return quant_coeffs; }
    vector<code_block> band_to_blocks(vector<pair<uint16_t, int8_t>> &a_b, subband_info b);
    vector<int8_t> extract_sign_data(vector<pair<uint16_t, int8_t>> &a, point anchor, int height, int width, int band_rows, int band_cols);
    vector<bit_plane> extract_bit_planes(vector<pair<uint16_t, int8_t>> a, subband_info b, point anchor, size_t height, size_t width, int band_rows, int band_cols);
    vector<code_block> bands_to_blocks(vector<pair<uint16_t, int8_t>> &a, vector<subband_info> bands);
    point find_bottom_right(vector<code_block> &cb);
    vector<pair<uint16_t, int8_t>> reconstruct_quant_coeffs(vector<code_block> &cb);
    void map_block_to_canvas(code_block &c, vector<pair<uint16_t, int8_t>> &q);
    vector<subband_info> band_info_from_blocks(vector<code_block> &cb);
    void print_cb_info(code_block &c_b);
};


#endif