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

    // Reconstruction below
    vector<tile> recon_y_tiles;
    vector<tile> recon_cb_tiles;
    vector<tile> recon_cr_tiles;

    std::cout << "Beginning Compression\n";


    for(tile &t : y_tiles){
        FDWT f(t.tile_data, t.anchor, 3, t.height, t.width);
        vector<double> transform_coeffs = f.get_transformed();
        
        Quantizer q(transform_coeffs, 3, t.height, t.width);
        vector<pair<uint16_t, int8_t>> q_coeffs = q.get_quant_coeffs();
        vector<subband_info> bands = q.get_band_info();
        
        CodeBlocks my_cb(q_coeffs, bands, t.height, t.width);
        vector<code_block> code_blocks = my_cb.get_code_blocks();
        
        BitPlaneEncoder my_bpe;
        BitPlaneDecoder my_bpd;  
        vector<code_block> decoded_blocks;
        for(code_block cb : code_blocks){
            my_bpe.load_code_block(cb);
            my_bpe.reset_bp_encoder();
            my_bpe.encode_code_block();
            my_bpe.make_coded_block();
           
            coded_block cb_coded;
            cb_coded = my_bpe.get_coded_block();


            ///////////////////////////
            // This is the bottom /////
            ///////////////////////////

            my_bpd.load_encoded_block(cb_coded);
            my_bpd.reset_bp_decoder();
            my_bpd.recover_codeblock();
            code_block dec_cb;
            dec_cb = my_bpd.get_code_block();
            decoded_blocks.push_back(cb);
        }

        CodeBlocks inverse_cb(decoded_blocks, t.height, t.width);
        vector<pair<uint16_t, int8_t>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
        vector<subband_info> recon_bands = inverse_cb.get_band_info();

        DeQuantizer dq(recon_quant_coeffs, recon_bands, 3, t.height, t.width);
        vector<double> dq_coeffs = dq.get_coeffs();

        IDWT i(dq_coeffs, t.anchor, 3, t.height, t.width);
        vector<double> t_dat = i.get_image();
        
        tile temp_tile;
        temp_tile.anchor = t.anchor;
        temp_tile.height = t.height;
        temp_tile.width = t.width;
        temp_tile.tile_data = t_dat;
        recon_y_tiles.push_back(temp_tile);
    }

    std::cout << "Done decoding Y\n";

    for(tile &t : cb_tiles){
        FDWT f(t.tile_data, t.anchor, 3, t.height, t.width);
        vector<double> transform_coeffs = f.get_transformed();
        
        Quantizer q(transform_coeffs, 3, t.height, t.width);
        vector<pair<uint16_t, int8_t>> q_coeffs = q.get_quant_coeffs();
        vector<subband_info> bands = q.get_band_info();
        
        CodeBlocks my_cb(q_coeffs, bands, t.height, t.width);
        vector<code_block> code_blocks = my_cb.get_code_blocks();
        
        BitPlaneEncoder my_bpe;
        BitPlaneDecoder my_bpd;  
        vector<code_block> decoded_blocks;
        for(code_block cb : code_blocks){
            my_bpe.load_code_block(cb);
            my_bpe.reset_bp_encoder();
            my_bpe.encode_code_block();
            my_bpe.make_coded_block();
           
            coded_block cb_coded;
            cb_coded = my_bpe.get_coded_block();


            ///////////////////////////
            // This is the bottom /////
            ///////////////////////////

            my_bpd.load_encoded_block(cb_coded);
            my_bpd.reset_bp_decoder();
            my_bpd.recover_codeblock();
            code_block dec_cb;
            dec_cb = my_bpd.get_code_block();
            decoded_blocks.push_back(cb);
        }

        CodeBlocks inverse_cb(decoded_blocks, t.height, t.width);
        vector<pair<uint16_t, int8_t>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
        vector<subband_info> recon_bands = inverse_cb.get_band_info();

        DeQuantizer dq(recon_quant_coeffs, recon_bands, 3, t.height, t.width);
        vector<double> dq_coeffs = dq.get_coeffs();

        IDWT i(dq_coeffs, t.anchor, 3, t.height, t.width);
        vector<double> t_dat = i.get_image();
        
        tile temp_tile;
        temp_tile.anchor = t.anchor;
        temp_tile.height = t.height;
        temp_tile.width = t.width;
        temp_tile.tile_data = t_dat;
        recon_cb_tiles.push_back(temp_tile);
    }

    std::cout << "Done decoding Cb\n";

    for(tile &t : cr_tiles){
        FDWT f(t.tile_data, t.anchor, 3, t.height, t.width);
        vector<double> transform_coeffs = f.get_transformed();
        
        Quantizer q(transform_coeffs, 3, t.height, t.width);
        vector<pair<uint16_t, int8_t>> q_coeffs = q.get_quant_coeffs();
        vector<subband_info> bands = q.get_band_info();
        
        CodeBlocks my_cb(q_coeffs, bands, t.height, t.width);
        vector<code_block> code_blocks = my_cb.get_code_blocks();
        
        BitPlaneEncoder my_bpe;
        BitPlaneDecoder my_bpd;  
        vector<code_block> decoded_blocks;
        for(code_block cb : code_blocks){
            my_bpe.load_code_block(cb);
            my_bpe.reset_bp_encoder();
            my_bpe.encode_code_block();
            my_bpe.make_coded_block();
           
            coded_block cb_coded;
            cb_coded = my_bpe.get_coded_block();


            ///////////////////////////
            // This is the bottom /////
            ///////////////////////////

            my_bpd.load_encoded_block(cb_coded);
            my_bpd.reset_bp_decoder();
            my_bpd.recover_codeblock();
            code_block dec_cb;
            dec_cb = my_bpd.get_code_block();
            decoded_blocks.push_back(cb);
        }

        CodeBlocks inverse_cb(decoded_blocks, t.height, t.width);
        vector<pair<uint16_t, int8_t>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
        vector<subband_info> recon_bands = inverse_cb.get_band_info();

        DeQuantizer dq(recon_quant_coeffs, recon_bands, 3, t.height, t.width);
        vector<double> dq_coeffs = dq.get_coeffs();

        IDWT i(dq_coeffs, t.anchor, 3, t.height, t.width);
        vector<double> t_dat = i.get_image();
        
        tile temp_tile;
        temp_tile.anchor = t.anchor;
        temp_tile.height = t.height;
        temp_tile.width = t.width;
        temp_tile.tile_data = t_dat;
        recon_cr_tiles.push_back(temp_tile);
    }

    std::cout << "Done Decoding Cr\n";

    // long unsigned int uncompressed_size;
    // // Calculate total length of tile;
    // deque<int> encoded_lengths = my_bpe.get_lengths();
    // for(int val: encoded_lengths){
    //     compressed_size += static_cast<long unsigned int>(val);
    // }
    // uncompressed_size = rows * cols * bits * channels / 8;
    // std::cout << "Total compressed Size: " << (int)compressed_size;
    // std::cout << ", Uncompressed Size: " << (int)uncompressed_size << endl;


    Tiles recon_y(recon_y_tiles);
    Tiles recon_cb(recon_cb_tiles);
    Tiles recon_cr(recon_cr_tiles);

    vector<double> recon_y_comp = recon_y.get_image();
    vector<double> recon_cb_comp = recon_cb.get_image();
    vector<double> recon_cr_comp = recon_cr.get_image();

    ImagePostProcessor img_post_proc(recon_y_comp, recon_cb_comp, recon_cr_comp, bits, rows, cols);

    cv::Mat recon_image = img_post_proc.get_image();
    string outfile = base_file_name + "_recon.png";
    cv::imwrite(outfile, recon_image);

    return 0;
}