#ifndef CODE_BLOCK_H
#define CODE_BLOCK_H

#include "my_struct.h"
#include <vector>

using namespace std;

class CodeBlocks{
private:
    vector<vector<pair<uint16_t, int8_t>>> quant_coeffs;
    vector<subband_info> band_info;
    vector<code_block> blocks;
    size_t max_height = 64;
    size_t max_width = 64;
public:
    CodeBlocks(vector<vector<pair<uint16_t, int8_t>>> &q, vector<subband_info> &sb_info);
    CodeBlocks(vector<code_block> &cb);
    vector<subband_info> get_band_info(){ return band_info; }
    vector<code_block> get_code_blocks(){ return blocks; }
    vector<vector<pair<uint16_t, int8_t>>> get_quant_coeffs(){ return quant_coeffs; }
    vector<code_block> band_to_blocks(vector<vector<pair<uint16_t, int8_t>>> &a_b, subband_info b);
    vector<vector<int8_t>> extract_sign_data(vector<vector<pair<uint16_t, int8_t>>> &a, point anchor, size_t height, size_t width);
    vector<bit_plane> extract_bit_planes(vector<vector<pair<uint16_t, int8_t>>> a, subband_info b, point anchor, size_t height, size_t width);
    vector<code_block> bands_to_blocks(vector<vector<pair<uint16_t, int8_t>>> &a, vector<subband_info> bands);
    point find_bottom_right(vector<code_block> &cb);
    vector<vector<pair<uint16_t, int8_t>>> reconstruct_quant_coeffs(vector<code_block> &cb);
    void map_block_to_canvas(code_block &c, vector<vector<pair<uint16_t, int8_t>>> &q);
    vector<subband_info> band_info_from_blocks(vector<code_block> &cb);
};


#endif