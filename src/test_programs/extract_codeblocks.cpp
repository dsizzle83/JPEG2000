#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "../include/my_struct.h"
#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"
#include "../include/code_block.h"

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

void writeCodeBlocks(const std::vector<code_block>& blocks, const std::string& filename, tile &t) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    // Iterate through each code_block in the vector.
    for (size_t idx = 0; idx < blocks.size(); ++idx) {
        const code_block &cb = blocks[idx];
        outFile << "Tile Anchor: (" << t.anchor.x << ", " << t.anchor.y << ")\n" ;
        outFile << "Tile Dimensions: " << t.height << " rows x " << t.width << " columns\n";
        outFile << "CodeBlock " << idx << "\n";
        outFile << "Anchor: (" << cb.anchor.x << ", " << cb.anchor.y << ")\n";
        outFile << "Subband Type: " << cb.s_b.type << "\n";
        outFile << "Subband Top Left: (" << cb.s_b.top_l.x << ", " << cb.s_b.top_l.y << ")\n";
        outFile << "Subband Bottom Right: (" << cb.s_b.bot_r.x << ", " << cb.s_b.bot_r.y << ")\n";
        outFile << "Step Size: " << cb.s_b.step_size << "\n";
        outFile << "Number of bits: " << cb.num_bits << "\n";
        outFile << "Sign Data:\n";

        // Writing the nested vector 'sign_data'
        for (const auto &row : cb.sign_data) {
            for (bool val : row) {
                // Write 1 or 0 for true/false
                outFile << (val ? "1 " : "0 ");
            }
            outFile << "\n";
        }
        outFile << "-----------------------------\n";
        for(const auto&bp : cb.bit_planes){
            outFile << "Bitplane: " << bp.bit_level << "\n";
            for(const auto &row: bp.plane_data){
                for(bool val : row){
                    outFile << (val ? "1 " : "0 ");
                }
                outFile << "\n";
            }
            outFile << "-----------------------------\n";
        }
    }

    outFile.close();
}

int main(){
    string filename = "animal-9347331.jpg";
    auto imgVector = readJpegToVector(filename);
    vector<vector<double>> scaled_image = scale_image(imgVector, 8);
    Tiles my_tile(scaled_image);
    vector<tile> my_tiles = my_tile.get_tiles();
    bool do_it = true;

    for(tile &t : my_tiles){
        FDWT f(t.tile_data, t.anchor, 3);
        vector<vector<double>> transform_coeffs = f.get_transformed();
        Quantizer q(transform_coeffs, 3);
        
        vector<vector<pair<uint16_t, bool>>> q_coeffs = q.get_quant_coeffs();
        vector<subband_info> bands = q.get_band_info();

        for(auto &band : bands){
            if(band.type == 0){
                cout << "LL Sign Coeffs:\n";
                for(int i=band.top_l.y; i < band.bot_r.y; i++){
                    for(int j= band.top_l.x; j < band.bot_r.x; j++){
                        cout << (q_coeffs[i][j].second ? "1 " : "0 ");
                    }
                    cout << "\n";
                }
            }
        }
        
        CodeBlocks my_cb(q_coeffs, bands);
        cout << "Done with making code blocks\n";
        
        vector<code_block> code_blocks = my_cb.get_code_blocks();
        if(do_it){
            string cb_file = "Codeblocks.txt";
            writeCodeBlocks(code_blocks, cb_file, t);
            do_it = false;
        }
        CodeBlocks inverse_cb(code_blocks);

        vector<vector<pair<uint16_t, bool>>> recon_quant_coeffs = inverse_cb.get_quant_coeffs();
        vector<subband_info> recon_bands = inverse_cb.get_band_info();

        DeQuantizer dq(recon_quant_coeffs, recon_bands, 3);
        vector<vector<double>> coeffs = dq.get_coeffs();
        IDWT i(coeffs, t.anchor, 3);
        vector<vector<double>> t_dat = i.get_image();
        t.tile_data = t_dat;
    }
    
    cout <<  "I got out of the loop" << endl;
    Tiles reconstructed(my_tiles);
    vector<vector<double>> recon_scaled = reconstructed.get_image();
    vector<vector<int>> recon = de_scale_image(recon_scaled, 8);

    saveVectorAsJpeg(recon, "output_ram.jpg");    

    return 0;
}