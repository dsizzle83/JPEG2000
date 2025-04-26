#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>

#include "../include/my_struct.h"
// #include "../include/dwt.h"
// #include "../include/quantizer.h"
// #include "../include/tiling.h"
#include "../include/code_block.h"
#include "../include/bp_coder.h"

#include <random>
using namespace std;

bool flipCoin() {
    // Use a random number generator engine
    std::random_device rd;
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  
    // Define a distribution - in this case, a uniform boolean distribution
    std::bernoulli_distribution distrib(0.5); // 0.5 is the probability of true
  
    // Return the result of the distribution, which is a random boolean
    return distrib(gen);
  }

vector<uint8_t> random_bit_plane(int height, int width){
    vector<uint8_t> bp_data(height * width, 0);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            bool random_bit = flipCoin();
            bp_data[i*width + j] = random_bit? 1 : 0;
        }
    }
    return bp_data;
}

vector<int8_t> random_sign_data(int height, int width){
    vector<int8_t> sign_data(height * width, 0);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            bool random_bit = flipCoin();
            sign_data[i*width + j] = random_bit? 1 : -1;
        }
    }
    return sign_data;
}

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
            int int_val = round(val);
            de_scaled_image[i*cols + j] = val;
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

bool compare_subbands(subband_info a, subband_info b){
    bool match = true;
    if(a.bit_depth != b.bit_depth){
        match = false;
        cout << "bit depth does not match\n";
    }
    if((a.bot_r.x != b.bot_r.x) || (a.bot_r.y != b.bot_r.y) || (a.top_l.x != b.top_l.x) || (a.top_l.y != b.top_l.y)){
        match = false;
        cout << "subband dimensions do not match\n";
    }
    if((a.expo != b.expo) || (a.mant != b.mant) || (a.step_size != b.step_size)){
        match = false;
        cout << "quantization parameters do not match\n";
    }
    if((a.level != b.level) || (a.type != b.type)){
        match = false;
        cout << "subband types or levels do not match\n";
    }
    return match;
}

bool compare_vectors(vector<uint8_t> a, vector<uint8_t> b, int height, int width){
    bool match = true;
    if(a.size() != b.size()){
        cout << "Vectors are different sizes!\n";
        match = false;
        return match;
    }
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            if(a[i*width + j] != b[i*width + j]){
                cout << "Vectors do not match at " << i << ", " << j;
                cout << ", a[i][j] = " << (int)a[i*width + j] << ", b[i][j] = " << (int)b[i*width + j] << endl;
                match = false;
            }
        }
    }
    return match;
}

bool compare_vectors(vector<int8_t> a, vector<int8_t> b, int height, int width){
    bool match = true;
    if(a.size() != b.size()){
        cout << "Vectors are different sizes!\n";
        match = false;
        return match;
    }
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            if(a[i*width + j] != b[i*width + j] && ( b[i*width + j] != 0)){
                cout << "Vectors do not match at " << i << ", " << j;
                cout << ", a[i][j] = " << (int)a[i*width + j] << ", b[i][j] = " << (int)b[i*width + j] << endl;
                match = false;
            }
        }
    }
    return match;
}


vector<uint8_t> get_bit_plane(vector<bit_plane> in_planes, int height, int width, int level){
    vector<uint8_t> plane_data(height * width, 0);
    for(auto &bp : in_planes){
        if(bp.bit_level == level){
            plane_data = bp.plane_data;
            break;
        }
    }
    return plane_data;
}

bool compare_code_blocks(code_block a, code_block b){
    bool match = true;
    if((a.anchor.x != b.anchor.x) || (a.anchor.y != b.anchor.y)){
        match = false;
        cout << "Anchors do not match\n";
    }
    if((a.height != b.height) || (a.width != b.width)){
        match = false;
        cout << "Dimensions do not match\n";
    }
    cout << "Testing Subband\n";
    match = compare_subbands(a.s_b, b.s_b);
    cout << "Testing Sign Data\n";
    match = compare_vectors(a.sign_data, b.sign_data, a.height, a.width);

    for(int i=0; i<a.num_bits; i++){
        vector<uint8_t> a_bp = get_bit_plane(a.bit_planes, a.height, a.width, i);
        vector<uint8_t> b_bp = get_bit_plane(b.bit_planes, b.height, b.width, i);
        cout << "Comparing Bitplanes at Bit: " << i << endl;
        match = compare_vectors(a_bp, b_bp, a.height, a.width);
    }
    return match;
}

void print_vector(vector<uint8_t> a, int height, int width){
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            cout << (int)a[i*width + j] << "\t";
        }
        cout << "\n";
    }
}

void print_vector(vector<int8_t> a, int height, int width){
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            cout << (int)a[i*width + j] << "\t";
        }
        cout << "\n";
    }
}

void print_code_block(code_block a){
    cout << "Sign Data: \n";
    print_vector(a.sign_data, a.height, a.width);
    for(auto &bp : a.bit_planes){
        cout << "Bit Plane: " << bp.bit_level << endl;
        print_vector(bp.plane_data, a.height, a.width);
    }
}

int main(){
    int rows = 4;
    int cols = 4;      
    code_block cb;
    cb.anchor = {0, 0};
    cb.num_bits = 5;
    vector<bit_plane> cb_bp;
    bit_plane bp0, bp1, bp2, bp3, bp4;

    bp0.bit_level = 0;
    bp1.bit_level = 1;
    bp2.bit_level = 2;
    bp3.bit_level = 3;
    bp4.bit_level = 4;

    bp0.plane_data = random_bit_plane(rows, cols);
    bp1.plane_data = random_bit_plane(rows, cols);
    bp2.plane_data = random_bit_plane(rows, cols);
    bp3.plane_data = random_bit_plane(rows, cols);
    bp4.plane_data = random_bit_plane(rows, cols);

    cb_bp.push_back(bp0);
    cb_bp.push_back(bp2);
    cb_bp.push_back(bp4);
    cb_bp.push_back(bp3); 
    cb_bp.push_back(bp1);    
    

    cb.bit_planes = cb_bp;
    cb.height = rows;
    cb.width = cols;
    
    subband_info cb_sb;
    cb_sb.bit_depth = 5;
    cb_sb.bot_r = {cols, rows};
    cb_sb.top_l = {0, 0};
    cb_sb.level = 1;
    cb_sb.mant = 1;
    cb_sb.expo = 1;
    cb_sb.step_size = 0.001;
    cb_sb.type = 1;
    cb.s_b = cb_sb;

    cb.sign_data = random_sign_data(rows, cols);

    deque<coded_block> encoded_blocks;
    BitPlaneEncoder my_bpe;
    BitPlaneDecoder my_bpd;

    my_bpe.load_code_block(cb);
    my_bpe.reset_bp_encoder();
    my_bpe.encode_code_block();
    my_bpe.make_coded_block();
    coded_block cb_coded;
    cb_coded = my_bpe.get_coded_block();   


    ///////////////////////////
    // This is the bottom /////
    ///////////////////////////
    code_block decoded_block;

    my_bpd.load_encoded_block(cb_coded);
    my_bpd.reset_bp_decoder();
    my_bpd.recover_codeblock();

    decoded_block = my_bpd.get_code_block();
    cout << "Input Block: \n";
    print_code_block(cb);

    cout << "Output Block: \n";
    print_code_block(decoded_block);

    bool match = compare_code_blocks(cb, decoded_block);

    if(match){
        cout << "Codeblocks Match!\n";
    }
    else{
        cout << "Codeblocks dont match :(\n";
    }

    return 0;
}