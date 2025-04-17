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

using namespace std;

std::vector<std::vector<int>> readJpegToVector(const std::string& filename) {
    cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);  // Read as grayscale
    if (img.empty()) {
        std::cerr << "Error: Could not read image\n";
        return {};
    }

    std::vector<std::vector<int>> imgVector(img.rows, std::vector<int>(img.cols));
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            imgVector[i][j] = img.at<uchar>(i, j);

    return imgVector;
}

void displayVector(const std::vector<std::vector<int>>& imgVector) {
    if (imgVector.empty()) {
        cerr << "Error: Image vector is empty\n";
        return;
    }

    int rows = imgVector.size(), cols = imgVector[0].size();
    cv::Mat img(rows, cols, CV_8U);  // 8-bit grayscale image

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            img.at<uchar>(i, j) = imgVector[i][j];

    cv::imshow("Image", img);
    cv::waitKey(0);  // Wait for a key press before closing window
}

vector<vector<double>> scale_image(vector<vector<int>> &img, int bits){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<vector<double>> scaled_image(img.size(), vector<double>(img[0].size(), 0));

    for(size_t i=0; i<img.size(); i++){
        for(size_t j=0; j<img[0].size(); j++){
            scaled_image[i][j] = static_cast<double>(img[i][j])/ dynamic_range - 0.5;
        }
    }
    return scaled_image;
}

vector<vector<int>> de_scale_image(vector<vector<double>> &scaled_image, int bits){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<vector<int>> de_scaled_image(scaled_image.size(), vector<int>(scaled_image[0].size(), 0));

    for(size_t i=0; i<scaled_image.size(); i++){
        for(size_t j=0; j<scaled_image[0].size(); j++){
            de_scaled_image[i][j] = static_cast<int>(round((scaled_image[i][j] + 0.5) * dynamic_range));
            if(de_scaled_image[i][j] < 0) de_scaled_image[i][j] = 0;
            if(de_scaled_image[i][j] > (int)dynamic_range) de_scaled_image[i][j] = (int)dynamic_range;
        }
    }
    return de_scaled_image;
}

void saveVectorAsJpeg(const std::vector<std::vector<int>>& imgVector, const std::string& filename) {
    if (imgVector.empty()) {
        std::cerr << "Error: Image vector is empty\n";
        return;
    }

    int rows = imgVector.size(), cols = imgVector[0].size();
    cv::Mat img(rows, cols, CV_8U);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            img.at<uchar>(i, j) = imgVector[i][j];

    cv::imwrite(filename, img);
}

int main(){
    string filename = "marc-kleen-eRwmBvhfvG8-unsplash.jpg";
    auto imgVector = readJpegToVector(filename);
    saveVectorAsJpeg(imgVector, "input_city.jpg");
    vector<vector<double>> scaled_image = scale_image(imgVector, 8);
    Tiles my_tile(scaled_image);
    vector<tile> my_tiles = my_tile.get_tiles();

    for(tile &t : my_tiles){
        FDWT f(t.tile_data, t.anchor, 3);
        vector<vector<double>> transform_coeffs = f.get_transformed();
        Quantizer q(transform_coeffs, 3);
        
        vector<vector<pair<uint16_t, int8_t>>> q_coeffs = q.get_quant_coeffs();
        vector<subband_info> bands = q.get_band_info();
        
        CodeBlocks my_cb(q_coeffs, bands);
        // cout << "Done with making code blocks\n";
        vector<vector<pair<uint16_t, int8_t>>>().swap(q_coeffs);
        vector<vector<double>>().swap(transform_coeffs);
        
        vector<code_block> code_blocks = my_cb.get_code_blocks();
        // cout << "1";
        deque<coded_block> encoded_blocks;
        // cout << "2\n";        
        for(auto &cb : code_blocks){
            // cout << "3";
            BitPlaneEncoder my_bpe(cb);
            // cout << "4";
            my_bpe.encode_code_block();
            my_bpe.make_coded_block();
            // cout << "5";
            coded_block cb_coded;
            // cout << "6";
            cb_coded = my_bpe.get_coded_block();
            // cout << "7";
            encoded_blocks.push_back(cb_coded);
            // cout << "8\n";    
        }
        ///////////////////////////
        // This is the bottom /////
        ///////////////////////////
        vector<code_block> decoded_blocks;
        // cout << "9\n";
        for(int i=0; i<encoded_blocks.size(); i++){
            coded_block ecb = encoded_blocks.front();
            encoded_blocks.pop_front();
            cout << "ECB Stats: \n" << "bits: " << ecb.bits << endl;
            cout << "height: " << ecb.height << endl;
            cout << "width: " << ecb.width << endl;
            cout << "s_b info:\n" << "Type: " << ecb.sb_info.type;
            cout << ", level: " << ecb.sb_info.level << ", step size: ";
            cout << fixed << setprecision(4) << ecb.sb_info.step_size << endl;
            BitPlaneDecoder my_bpd(ecb);
            // cout << "11";
            my_bpd.recover_codeblock();
            // cout << "12";
            code_block cb;
            // cout << "13";
            cb = my_bpd.get_code_block();
            // cout << "14";
            decoded_blocks.push_back(cb);
            // cout << "15\n";
        }
        // cout << "16";
        CodeBlocks inverse_cb(code_blocks);
        // cout << "18";
        vector<vector<pair<uint16_t, int8_t>>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
        // cout << "19";
        vector<subband_info> recon_bands = inverse_cb.get_band_info();

        // cout << "20";
        DeQuantizer dq(recon_quant_coeffs, recon_bands, 3);
        // cout << "21";
        vector<vector<double>> coeffs = dq.get_coeffs();
        // cout << "22";
        IDWT i(coeffs, t.anchor, 3);
        // cout << "23";
        vector<vector<double>> t_dat = i.get_image();
        // cout << "24";
        t.tile_data = t_dat;
        // cout << "25\n";
    }
    
    cout <<  "I got out of the loop" << endl;
    Tiles reconstructed(my_tiles);
    vector<vector<double>> recon_scaled = reconstructed.get_image();
    vector<vector<int>> recon = de_scale_image(recon_scaled, 8);

    saveVectorAsJpeg(recon, "output_city.jpg");    

    return 0;
}