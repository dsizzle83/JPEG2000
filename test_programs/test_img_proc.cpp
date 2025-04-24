#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>

#include "../include/my_struct.h"
#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"
#include "../include/code_block.h"
#include "../include/bp_coder.h"
#include "../include/img_proc.h"

#define DWT_LEVEL 3

using namespace std;

vector<int> de_scale_image(vector<double> &scaled_image, int bits, int rows, int cols){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<int> de_scaled_image(scaled_image.size(), 0);

    for(size_t i=0; i<rows; i++){
        for(size_t j=0; j<cols; j++){
            double val = scaled_image[i*cols + j];
            if(val <= -0.5) val = -0.5;
            else if(val >= 0.5) val = 0.5;
            val += 0.5;
            val *= dynamic_range;
            if(val < 0) val = 0;
            else if(val > dynamic_range - 1) val = dynamic_range - 1;
            int int_val = round(val);
            de_scaled_image[i*cols + j] = val;
        }
    }
    return de_scaled_image;
}

void saveVectorAsFile(const vector<int>& imgVector, const std::string& filename, int rows, int cols) {
    if (imgVector.empty()) {
        std::cerr << "Error: Image vector is empty\n";
        return;
    }

    cv::Mat img(rows, cols, CV_8U);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            img.at<uchar>(i, j) = imgVector[i*cols + j];

    cv::imwrite(filename, img);
}

tile compress(tile t, long unsigned int &compressed_size, int level){
    FDWT f(t.tile_data, t.anchor, level, t.height, t.width);
    vector<double> transform_coeffs = f.get_transformed();

    Quantizer q(transform_coeffs, level, t.height, t.width);
    vector<double>().swap(transform_coeffs);
    vector<pair<uint16_t, int8_t>> q_coeffs = q.get_quant_coeffs();
    vector<subband_info> bands = q.get_band_info();

    CodeBlocks my_cb(q_coeffs, bands, t.height, t.width);
    vector<pair<uint16_t, int8_t>>().swap(q_coeffs);
    vector<subband_info>().swap(bands);
    
    vector<code_block> code_blocks = my_cb.get_code_blocks();
    deque<coded_block> encoded_blocks;

    BitPlaneEncoder my_bpe;

    for(auto &cb : code_blocks){
        my_bpe.load_code_block(cb);
        my_bpe.reset_bp_encoder();
        my_bpe.encode_code_block();
        my_bpe.make_coded_block();
        coded_block cb_coded;
        cb_coded = my_bpe.get_coded_block();
        encoded_blocks.push_back(cb_coded);
    }
    vector<code_block>().swap(code_blocks);
    for(auto &val : my_bpe.get_lengths()){
        compressed_size += val;
    }    
    t.enc_data =  encoded_blocks;
    return t;
}

tile reconstruct_image(tile &enc_b, int level){
    vector<code_block> decoded_blocks;
    tile out_tile;
    BitPlaneDecoder my_bpd;
    for(int i=0; i<enc_b.enc_data.size(); i++){
        coded_block ecb = enc_b.enc_data.front();
        enc_b.enc_data.pop_front();
        my_bpd.load_encoded_block(ecb);
        my_bpd.reset_bp_decoder();
        my_bpd.recover_codeblock();
        code_block cb;
        cb = my_bpd.get_code_block();
        decoded_blocks.push_back(cb);
    }
    CodeBlocks inverse_cb(decoded_blocks, enc_b.height, enc_b.width);
    vector<pair<uint16_t, int8_t>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
    vector<subband_info> recon_bands = inverse_cb.get_band_info();
    
    DeQuantizer dq(recon_quant_coeffs, recon_bands, level, enc_b.height, enc_b.width);
    vector<pair<uint16_t, int8_t>>().swap(recon_quant_coeffs);
    vector<subband_info>().swap(recon_bands);
    vector<double> coeffs = dq.get_coeffs();

    IDWT i(coeffs, enc_b.anchor, level, enc_b.height, enc_b.width);
    vector<double> t_dat = i.get_image();
    vector<double>().swap(coeffs);

    out_tile.anchor = enc_b.anchor;
    out_tile.height = enc_b.height;
    out_tile.width = enc_b.width;
    out_tile.tile_data = t_dat;
    return out_tile;
}

int main(){
    // Read in the image and get the rows and columns
    string filename = "./test_images/animal-9347331.jpg";
    ImagePreProcessor img_pre_proc(filename, true);
    vector<double> Y  = img_pre_proc.get_scaled_comp_0();
    vector<double> Cb = img_pre_proc.get_scaled_comp_1();
    vector<double> Cr = img_pre_proc.get_scaled_comp_2();

    string base_file_name = "./test_images/ram";
    string y_file  = base_file_name + "_y.png";
    string cb_file = base_file_name + "_cb.png";
    string cr_file = base_file_name + "_cr.png";

    int bits = img_pre_proc.get_bit_depth();
    int rows = img_pre_proc.get_rows();
    int cols = img_pre_proc.get_cols();
    int channels = 3;
    cout << "Bit Depth: " << bits << ", Rows: " << rows << ", Cols: " << cols << endl;

    Tiles y_comp_tiles(Y, rows, cols);
    vector<tile> y_tiles = y_comp_tiles.get_tiles();
    Tiles cr_comp_tiles(Cr, rows, cols);
    vector<tile> cr_tiles = cr_comp_tiles.get_tiles();
    Tiles cb_comp_tiles(Cb, rows, cols);
    vector<tile> cb_tiles = cb_comp_tiles.get_tiles();

    long unsigned int compressed_size = 0;


    for(auto &t : y_tiles){
        tile temp_tile;
        temp_tile = compress(t, compressed_size, DWT_LEVEL);
        t = temp_tile;
    }

    for(auto &t : cb_tiles){
        tile temp_tile;
        temp_tile = compress(t, compressed_size, DWT_LEVEL);
        t = temp_tile;
    }

    for(auto &t : cr_tiles){
        tile temp_tile;
        temp_tile = compress(t, compressed_size, DWT_LEVEL);
        t = temp_tile;
    }
    
    long unsigned int uncompressed_size;
    uncompressed_size = rows * cols * bits * channels / 8;
    cout << "Total compressed Size: " << (int)compressed_size;
    cout << ", Uncompressed Size: " << (int)uncompressed_size << endl;

    // Reconstruction below
    vector<tile> recon_y_tiles;
    vector<tile> recon_cb_tiles;
    vector<tile> recon_cr_tiles;

    for(auto &t : y_tiles){
        tile temp_tile;
        temp_tile = reconstruct_image(t, DWT_LEVEL);
        recon_y_tiles.push_back(temp_tile);
    }

    for(auto &t : cr_tiles){
        tile temp_tile;
        temp_tile = reconstruct_image(t, DWT_LEVEL);
        recon_cr_tiles.push_back(temp_tile);
    }

    for(auto &t : cb_tiles){
        tile temp_tile;
        temp_tile = reconstruct_image(t, DWT_LEVEL);
        recon_cb_tiles.push_back(temp_tile);
    }

    Tiles recon_y(recon_y_tiles);
    Tiles recon_cb(recon_cb_tiles);
    Tiles recon_cr(recon_cr_tiles);

    vector<double> recon_y_comp = recon_y.get_image();
    vector<double> recon_cb_comp = recon_cb.get_image();
    vector<double> recon_cr_comp = recon_cr.get_image();

    cout << "y_size: " << recon_y_comp.size() << endl;
    cout << "cb_size: " << recon_cb_comp.size() << endl;  
    cout << "cr_size: " << recon_cr_comp.size() << endl;  


    // vector<int> y_int  = de_scale_image(Y,  bits, rows, cols);
    // vector<int> cr_int = de_scale_image(Cr, bits, rows, cols);
    // vector<int> cb_int = de_scale_image(Cb, bits, rows, cols);

    // vector<int> R = img_pre_proc.get_comp_0();
    // vector<int> G = img_pre_proc.get_comp_1();
    // vector<int> B = img_pre_proc.get_comp_2();
    // string r_file = base_file_name + "_r.png";
    // string g_file = base_file_name + "_g.png";
    // string b_file = base_file_name + "_b.png";

    // saveVectorAsFile(y_int,  y_file,  rows, cols);
    // saveVectorAsFile(cr_int, cr_file, rows, cols);
    // saveVectorAsFile(cb_int, cb_file, rows, cols);

    // saveVectorAsFile(R, r_file, rows, cols);
    // saveVectorAsFile(G, g_file, rows, cols);
    // saveVectorAsFile(B, b_file, rows, cols);

    ImagePostProcessor img_post_proc(recon_y_comp, recon_cb_comp, recon_cr_comp, bits, rows, cols);

    cv::Mat recon_image = img_post_proc.get_image();
    string outfile = base_file_name + "_recon.png";
    cv::imwrite(outfile, recon_image);

    return 0;
}