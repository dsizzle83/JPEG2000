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
#include "../include/mq_coder.h"
#include "../include/img_proc.h"

using namespace std;

vector<int> readJpegToVector(cv::Mat &img) {
    if (img.empty()) {
        std::cerr << "Error: Could not read image\n";
        return {};
    }

    vector<int> imgVector(img.rows *img.cols);
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            imgVector[i*img.cols + j] = img.at<uchar>(i, j);

    return imgVector;
}

void displayVector(vector<int>& imgVector, int rows, int cols) {
    if (imgVector.empty()) {
        cerr << "Error: Image vector is empty\n";
        return;
    }

    cv::Mat img(rows, cols, CV_8U);  // 8-bit grayscale image

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            img.at<uchar>(i, j) = imgVector[i * cols + j];

    cv::imshow("Image", img);
    cv::waitKey(0);  // Wait for a key press before closing window
}

vector<double> scale_image(vector<int> &img, int bits, int rows, int cols){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<double> scaled_image(img.size(), 0);

    for(size_t i=0; i<rows; i++){
        for(size_t j=0; j<cols; j++){
            scaled_image[i*cols + j] = static_cast<double>(img[i*cols + j])/ dynamic_range - 0.5;
        }
    }
    return scaled_image;
}

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
            else if(val > pow(2, bits) - 1) val = pow(2, bits) - 1;
            int int_val = static_cast<int>(round(val));
            de_scaled_image[i*cols + j] = int_val;
        }
    }
    return de_scaled_image;
}

void saveVectorAsJpeg(const vector<int>& imgVector, const std::string& filename, int rows, int cols) {
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
    string filename = "./test_images/colorscales.png";
    cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    int image_rows = img.rows;
    int image_cols = img.cols;
    vector<int> imgVector = readJpegToVector(img);

    saveVectorAsJpeg(imgVector, "./test_images/input_colorscales.png", image_rows, image_cols);
    vector<double> scaled_image = scale_image(imgVector, 8, image_rows, image_cols);
    Tiles my_tile(scaled_image, image_rows, image_cols);
    vector<tile> my_tiles = my_tile.get_tiles();

    vector<tile> recon_tiles;

    long unsigned int compressed_size = 0;
    for(tile &t : my_tiles){
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
        recon_tiles.push_back(temp_tile);
    }
    
    std::cout << "Original Size: " << image_cols * image_rows * 8 << ", Compressed Size: " << (int)compressed_size;

    Tiles reconstructed(recon_tiles);
    vector<double> recon_scaled = reconstructed.get_image();
    vector<int> recon = de_scale_image(recon_scaled, 8, image_rows, image_cols);

    saveVectorAsJpeg(recon, "./test_images/output_colorscales.png", image_rows, image_cols);    

    return 0;
}