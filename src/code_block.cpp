#include <iostream>
#include <vector>

#include "../include/my_struct.h"
#include "../include/code_block.h"

using namespace std;

CodeBlocks::CodeBlocks(vector<pair<uint16_t, int8_t>> &q, vector<subband_info> &sb_info, int height, int width){
    band_info = sb_info;
    quant_coeffs = q;
    rows = height;
    cols = width;
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

CodeBlocks::CodeBlocks(vector<code_block> &cb, int height, int width){
    cols = width;
    rows = height;
    blocks = cb;
    quant_coeffs = reconstruct_quant_coeffs(blocks);
    band_info = band_info_from_blocks(blocks);
}

vector<code_block> CodeBlocks::band_to_blocks(vector<pair<uint16_t, int8_t>> &a_b, subband_info b){
    vector<code_block> cb_vec_b;
    // cout << "Band: " << b.type << ", level: " << b.level <<", (" << a_b.size() << " x " << a_b[0].size() << ")\n";
    point relative_bot_r;
    // This point represents the bottom right corner of the subband.  The top_left point is (0, 0)
    relative_bot_r.x = b.bot_r.x - b.top_l.x;
    relative_bot_r.y = b.bot_r.y - b.top_l.y;
    point relative_anchor = {0, 0}; // Relative top left corner
    while(relative_anchor.y < relative_bot_r.y){
        relative_anchor.x = 0;
        int height = max_height;
        // If there are less than 64 rows available (typical code block height), just use the rest
        if(relative_anchor.y + static_cast<int>(max_height) > relative_bot_r.y) height = relative_bot_r.y - relative_anchor.y;
        while(relative_anchor.x < relative_bot_r.x){
            code_block temp_block;
            int width = max_width;
            // If there are less than 64 columns available (typical code block width), just use the rest
            if(relative_anchor.x + static_cast<int>(max_width) > relative_bot_r.x) width = relative_bot_r.x - relative_anchor.x;
            temp_block.anchor = relative_anchor; // Represents the top_left of the codeblock w.r.t. subband
            temp_block.s_b = b;
            temp_block.num_bits = b.bit_depth;
            temp_block.height = height;
            temp_block.width = width;
            temp_block.sign_data = extract_sign_data(a_b, relative_anchor, height, width, relative_bot_r.y, relative_bot_r.x);
            temp_block.bit_planes = extract_bit_planes(a_b, b, relative_anchor, height, width, relative_bot_r.y, relative_bot_r.x);
            cb_vec_b.push_back(temp_block);
            relative_anchor.x += max_width;
        }
        relative_anchor.y += max_height;
    }
    return cb_vec_b;
}

vector<int8_t> CodeBlocks::extract_sign_data(vector<pair<uint16_t, int8_t>> &a, point anchor, int height, int width, int band_rows, int band_cols){
    vector<int8_t> x(height * width, 0);
    for(int i=0; i<height; i++){
        if(i + anchor.y >= band_rows) break;
        for(size_t j=0; j<width; j++){
            if(j + anchor.x >= band_cols) break;
            x[i*width + j] = a[(i+anchor.y) * band_cols + (j+anchor.x)].second;
        }
    }
    return x;
}

vector<bit_plane> CodeBlocks::extract_bit_planes(vector<pair<uint16_t, int8_t>> a, subband_info b, point anchor, size_t height, size_t width, int band_rows, int band_cols){
    vector<bit_plane> y; 
    for(int bit = b.bit_depth; bit>0; bit--){
        bit_plane y_p;
        y_p.bit_level = bit - 1;
        for(size_t i=0; i<height; i++){
            if(i + anchor.y >= band_rows){
                // cout << "Broke from y = " << i << "\n";
                break;
            }
            vector<uint8_t> temp_row;
            for(size_t j=0; j<width; j++){
                if(j + anchor.x >= band_cols){
                    // cout << "Broke from x = " << j << "\n";
                    break;
                }
                uint8_t sample = (a[(i+anchor.y)*band_cols + (j+anchor.x)].first >= (1 << (bit-1))) ? 1 : 0;
                if(sample == 1){
                    a[(i+anchor.y)*band_cols + (j+anchor.x)].first -= (1 << (bit-1));
                }
                y_p.plane_data.push_back(sample);
            }
        }
        y.push_back(y_p);
    }
    return y;
}

vector<code_block> CodeBlocks::bands_to_blocks(vector<pair<uint16_t, int8_t>> &a, vector<subband_info> bands){
    vector<code_block> my_blocks;
    for(const auto &band : bands){
        // cout << "Band: " << band.type << ", level: " << band.level;
        // cout << ", top_l:(" << band.top_l.x << ", " << band.top_l.y << ")";
        // cout << ", bot_r:(" << band.bot_r.x << ", " << band.bot_r.y << ")\n";
        vector<code_block> band_blocks;
        int band_rows = band.bot_r.y - band.top_l.y;
        int band_cols = band.bot_r.x - band.top_l.x;
        // cout << "Allocating Subband Data Vector... ";
        vector<pair<uint16_t, int8_t>> subband_data(band_rows * band_cols, {0, 0});
        // cout << "Okay!" << endl;
        for(size_t i=0; i<band_rows; i++){
            for(size_t j=0; j<band_cols; j++){
                subband_data[i*band_cols + j] = a[(i+band.top_l.y) * cols + j+band.top_l.x];
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

vector<pair<uint16_t, int8_t>> CodeBlocks::reconstruct_quant_coeffs(vector<code_block> &cb){
    point bot_r = find_bottom_right(cb);
    // cout << "Bottom Right: (" << bot_r.x << ", " << bot_r.y << ")\n";
    // cout << "Allocating Coefficient Vector... ";
    vector<pair<uint16_t, int8_t>> q(bot_r.y * bot_r.x, {0, false});
    // cout << "Okay!\n";
    for(auto&c : cb){
        map_block_to_canvas(c, q);
        // print_cb_info(c);
        // cout << "------------------------------------------------------\n";
    }
    return q;
}

void CodeBlocks::map_block_to_canvas(code_block &c, vector<pair<uint16_t, int8_t>> &q){
    // cout << "Codeblock Anchor: (" << c.anchor.x << ", " << c.anchor.y << ")\n";
    // cout << "Band: " << c.s_b.type << ", level: " << c.s_b.level << "\n";
    // cout << "Sign Data: " << rows << "rows x " << cols << "cols\n";
    // cout << "Allocating codeblock coefficient vector... ";
    vector<pair<uint16_t, int8_t>> q_c(c.height * c.width, {0, false});
    // cout << "Okay!\n";

    point anchor;
    anchor.x = c.anchor.x + c.s_b.top_l.x;
    anchor.y = c.anchor.y + c.s_b.top_l.y;

    for(int i=0; i<c.height; i++){
        for(int j=0; j<c.width; j++){
            q_c[i*c.width + j].second = c.sign_data[i * c.width + j];
        }
    }
    // cout << "Good past sign data\n";
    for(auto &plane : c.bit_planes){
        // cout << "Bit level: " << plane.bit_level << endl;
        // cout << "Plane Size: " << plane.plane_data.size() << "rows x " << plane.plane_data[0].size() << "cols\n";
        for(size_t i=0; i<c.height; i++){
            for(size_t j=0; j<c.width; j++){
                q_c[i*c.width + j].first += (static_cast<int>(plane.plane_data[i*c.width + j])) << (plane.bit_level);
            }
        }
    }
    // cout << "q_c: " << q_c.size() << "rows x " << q_c[0].size() << "cols\n";
    // cout << "q:  " << q.size() << "rows x " << q[0].size() << "cols\n";
    for(size_t i=0; i<c.height; i++){
        for(size_t j=0; j<c.width; j++){
            point tile_reference = {i + anchor.y, j + anchor.x};
            // if(tile_reference.x >= cols || tile_reference.y >= rows){
            //     cout << "Error.  Tried to write past canvas at point: (";
            //     cout << tile_reference.x << ", " << tile_reference.y << ")\n";
            // }
            q[(i + anchor.y) * cols + j + anchor.x] = q_c[i * c.width + j];
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

void CodeBlocks::print_cb_info(code_block &c_b){
    cout << "Anchor: (" << c_b.anchor.x << ", " << c_b.anchor.y << ")\n";
    cout << "Subband Type: " << c_b.s_b.type << ", Level: " << c_b.s_b.level << endl;
    cout << "Height: " << c_b.height << ", Width: " << c_b.width << ", Bits: " << (int)c_b.bit_planes.size() << endl;
}