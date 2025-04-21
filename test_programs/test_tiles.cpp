#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>

#include "../include/my_struct.h"
#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"

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

int main(){
    // Read in the image and get the rows and columns
    string filename = "./test_programs/animal-9347331.jpg";
    cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    int image_rows = img.rows;
    int image_cols = img.cols;

    auto imgVector = readJpegToVector(img);
    vector<double> scaled_image = scale_image(imgVector, 8, image_rows, image_cols);
    Tiles my_tile(scaled_image, image_rows, image_cols);
    vector<tile> my_tiles = my_tile.get_tiles();

    for(tile &t : my_tiles){
        FDWT f(t.tile_data, t.anchor, 3, t.height, t.width);
        vector<double> transformed_coeffs = f.get_transformed();
        Quantizer q(transformed_coeffs, 3, t.height, t.width);
        vector<pair<uint16_t, int8_t>> q_coeffs = q.get_quant_coeffs();
        DeQuantizer dq(q_coeffs, q.get_band_info(), 3, t.height, t.width);
        vector<double> coeffs = dq.get_coeffs();
        IDWT i(coeffs, t.anchor, 3, t.height, t.width);
        t.tile_data = i.get_image();
    }

    // // This code is used to view each image as a tile
    // int tile_num = 0;
    // for(tile &t : my_tiles){
    //     string temp_file = "temp_file" + to_string(tile_num) + "_x" +
    //         to_string(t.anchor.x) + "_y" + to_string(t.anchor.y) + ".jpg";
    //     vector<double> temp = t.tile_data;  
    //     vector<int> temp_int = de_scale_image(temp, 8, t.height, t.width);
    //     saveVectorAsJpeg(temp_int, temp_file, t.height, t.width);
    //     tile_num++;
    // }

    Tiles reconstructed(my_tiles);
    vector<double> recon_scaled = reconstructed.get_image();
    vector<int> recon = de_scale_image(recon_scaled, 8, image_rows, image_cols);

    saveVectorAsJpeg(recon, "./test_programs/output_ram.jpg", image_rows, image_cols);
    

    return 0;
}