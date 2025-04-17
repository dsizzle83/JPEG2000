#include <iostream>
#include <vector>

#include "../include/my_struct.h"
#include "../include/code_block.h"

using namespace std;

CodeBlocks::CodeBlocks(vector<vector<pair<uint16_t, int8_t>>> &q, vector<subband_info> &sb_info){
    band_info = sb_info;
    quant_coeffs = q;
    blocks = bands_to_blocks(quant_coeffs, band_info);
    // cout << "q: " << q.size() << " x " << q[0].size() << endl;
    // for(subband_info band : sb_info){
    //     cout << "Level: " << band.level << ", type: " << band.type;
    //     cout << ", top_l:(" << band.top_l.x << ", " << band.top_l.y << ")";
    //     cout << ", bot_r:(" << band.bot_r.x << ", " << band.bot_r.y << ")\n";
    // }
    // for(auto&block : blocks){
    //     size_t rows = block.sign_data.size();
    //     size_t cols = block.sign_data[0].size();
    //     cout << "CodeBlock Anchor: (" << block.anchor.x << ", " << block.anchor.y << ")\n";
    //     cout << "Band: " << block.s_b.type << ", level: " << block.s_b.level << "\n";
    //     cout << "Sign data Size: " << rows << "rows x " << cols << "cols\n";
    //     for(auto &plane : block.bit_planes){
    //         size_t rows = plane.plane_data.size();
    //         size_t cols = plane.plane_data[0].size();
    //         int bit = plane.bit_level;
    //         // cout << "Plane " << bit << " size: " << rows << "rows x " << cols << "cols\n";
    //     }
    // }
}

CodeBlocks::CodeBlocks(vector<code_block> &cb){
    blocks = cb;
    quant_coeffs = reconstruct_quant_coeffs(blocks);
    band_info = band_info_from_blocks(blocks);
}

vector<code_block> CodeBlocks::band_to_blocks(vector<vector<pair<uint16_t, int8_t>>> &a_b, subband_info b){
    vector<code_block> cb_vec_b;
    // cout << "Band: " << b.type << ", level: " << b.level <<", (" << a_b.size() << " x " << a_b[0].size() << ")\n";
    point relative_bot_r;
    relative_bot_r.x = b.bot_r.x - b.top_l.x;
    relative_bot_r.y = b.bot_r.y - b.top_l.y;
    point relative_anchor = {0, 0};
    while(relative_anchor.y < relative_bot_r.y){
        relative_anchor.x = 0;
        size_t height = max_height;
        if(relative_anchor.y + static_cast<int>(max_height) > relative_bot_r.y) height = relative_bot_r.y - relative_anchor.y;
        while(relative_anchor.x < relative_bot_r.x){
            code_block temp_block;
            size_t width = max_width;
            if(relative_anchor.x + static_cast<int>(max_width) > relative_bot_r.x) width = relative_bot_r.x - relative_anchor.x;
            temp_block.anchor = relative_anchor;
            temp_block.s_b = b;
            temp_block.num_bits = b.bit_depth;
            temp_block.sign_data = extract_sign_data(a_b, relative_anchor, height, width);
            temp_block.bit_planes = extract_bit_planes(a_b, b, relative_anchor, height, width);
            cb_vec_b.push_back(temp_block);
            relative_anchor.x += max_width;
        }
        relative_anchor.y += max_height;
    }
    return cb_vec_b;
}

vector<vector<int8_t>> CodeBlocks::extract_sign_data(vector<vector<pair<uint16_t, int8_t>>> &a, point anchor, size_t height, size_t width){
    vector<vector<int8_t>> x(height, vector<int8_t>(width, 0));
    for(size_t i=0; i<height; i++){
        if(i + anchor.y >= a.size()) break;
        for(size_t j=0; j<width; j++){
            if(j + anchor.x >= a[0].size()) break;
            x[i][j] = a[i+anchor.y][j+anchor.x].second;
        }
    }
    return x;
}

vector<bit_plane> CodeBlocks::extract_bit_planes(vector<vector<pair<uint16_t, int8_t>>> a, subband_info b, point anchor, size_t height, size_t width){
    vector<bit_plane> y; 
    for(int bit = b.bit_depth; bit>0; bit--){
        bit_plane y_p;
        y_p.bit_level = bit - 1;
        for(size_t i=0; i<height; i++){
            if(i + anchor.y >= a.size()){
                // cout << "Broke from y = " << i << "\n";
                break;
            }
            vector<uint8_t> temp_row;
            for(size_t j=0; j<width; j++){
                if(j + anchor.x >= a[0].size()){
                    // cout << "Broke from x = " << j << "\n";
                    break;
                }
                uint8_t sample = (a[i+anchor.y][j+anchor.x].first >= (1 << (bit-1))) ? 1 : 0;
                if(sample == 1){
                    a[i+anchor.y][j+anchor.x].first -= (1 << (bit-1));
                }
                temp_row.push_back(sample);
            }
            y_p.plane_data.push_back(temp_row);
        }
        y.push_back(y_p);
    }
    return y;
}

vector<code_block> CodeBlocks::bands_to_blocks(vector<vector<pair<uint16_t, int8_t>>> &a, vector<subband_info> bands){
    vector<code_block> my_blocks;
    for(const auto &band : bands){
        // cout << "Band: " << band.type << ", level: " << band.level;
        // cout << ", top_l:(" << band.top_l.x << ", " << band.top_l.y << ")";
        // cout << ", bot_r:(" << band.bot_r.x << ", " << band.bot_r.y << ")\n";
        vector<code_block> band_blocks;
        size_t rows = band.bot_r.y - band.top_l.y;
        size_t cols = band.bot_r.x - band.top_l.x;
        // cout << "Allocating Subband Data Vector... ";
        vector<vector<pair<uint16_t, int8_t>>> subband_data(rows, vector<pair<uint16_t, int8_t>>(cols, {0, 0}));
        // cout << "Okay!" << endl;
        for(size_t i=0; i<rows; i++){
            for(size_t j=0; j<cols; j++){
                subband_data[i][j] = a[i+band.top_l.y][j+band.top_l.x];
            }
        }
        band_blocks = band_to_blocks(subband_data, band);
        my_blocks.insert(my_blocks.end(), band_blocks.begin(), band_blocks.end());
    }
    return my_blocks;
}

point CodeBlocks::find_bottom_right(vector<code_block> &cb){
    int max_x = 0;
    int max_y = 0;
    for(const auto &block : cb){
        point bot_r = block.s_b.bot_r;
        if(bot_r.x > max_x) max_x = bot_r.x;
        if(bot_r.y > max_y) max_y = bot_r.y;
    }
    point bot_r = {max_x, max_y};
    return bot_r;
}

vector<vector<pair<uint16_t, int8_t>>> CodeBlocks::reconstruct_quant_coeffs(vector<code_block> &cb){
    point bot_r = find_bottom_right(cb);
    // cout << "Bottom Right: (" << bot_r.x << ", " << bot_r.y << ")\n";
    // cout << "Allocating Coefficient Vector... ";
    vector<vector<pair<uint16_t, int8_t>>> q(bot_r.y, vector<pair<uint16_t, int8_t>>(bot_r.x, {0, false}));
    // cout << "Okay!\n";
    for(auto&c : cb){
        map_block_to_canvas(c, q);
    }
    return q;
}

void CodeBlocks::map_block_to_canvas(code_block &c, vector<vector<pair<uint16_t, int8_t>>> &q){
    size_t rows = c.sign_data.size();
    size_t cols = c.sign_data[0].size();
    // cout << "Codeblock Anchor: (" << c.anchor.x << ", " << c.anchor.y << ")\n";
    // cout << "Band: " << c.s_b.type << ", level: " << c.s_b.level << "\n";
    // cout << "Sign Data: " << rows << "rows x " << cols << "cols\n";
    // cout << "Allocating codeblock coefficient vector... ";
    vector<vector<pair<uint16_t, int8_t>>> q_c(rows, vector<pair<uint16_t, int8_t>>(cols, {0, false}));
    // cout << "Okay!\n";

    for(size_t i=0; i<rows; i++){
        for(size_t j=0; j<cols; j++){
            q_c[i][j].second = c.sign_data[i][j];
        }
    }
    // cout << "Good past sign data\n";
    for(auto &plane : c.bit_planes){
        // cout << "Bit level: " << plane.bit_level << endl;
        // cout << "Plane Size: " << plane.plane_data.size() << "rows x " << plane.plane_data[0].size() << "cols\n";
        for(size_t i=0; i<rows; i++){
            for(size_t j=0; j<cols; j++){
                q_c[i][j].first += (static_cast<int>(plane.plane_data[i][j])) << (plane.bit_level);
            }
        }
    }
    // cout << "q_c: " << q_c.size() << "rows x " << q_c[0].size() << "cols\n";
    // cout << "q:  " << q.size() << "rows x " << q[0].size() << "cols\n";
    for(size_t i=0; i<rows; i++){
        for(size_t j=0; j<cols; j++){
            q[i + c.anchor.y][j + c.anchor.x] = q_c[i][j];
        }
    }
}

vector<subband_info> CodeBlocks::band_info_from_blocks(vector<code_block> &cb){
    vector<subband_info> bands;
    for(const auto &c : cb){
        bool in_bands = false;
        for(const auto &b : bands){
            if((c.s_b.level == b.level) && (c.s_b.type == b.type)){
                in_bands = true;
                break;
            }
        }
        if(!in_bands) bands.push_back(c.s_b);
    }
    return bands;
}
