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

int main(){
    string filename = "animal-9347331.jpg";
    auto imgVector = readJpegToVector(filename);
    vector<vector<double>> scaled_image = scale_image(imgVector, 8);
    Tiles my_tile(scaled_image);
    vector<tile> my_tiles = my_tile.get_tiles();

    for(tile &t : my_tiles){
        FDWT f(t.tile_data, t.anchor, 3);
        vector<vector<double>> transformed_coeffs = f.get_transformed();
        Quantizer q(transformed_coeffs, 3);
        vector<vector<pair<uint16_t, bool>>> q_coeffs = q.get_quant_coeffs();
        DeQuantizer dq(q_coeffs, q.get_band_info(), 3);
        vector<vector<double>> coeffs = dq.get_coeffs();
        IDWT i(coeffs, t.anchor, 3);
        t.tile_data = i.get_image();
    }

    Tiles reconstructed(my_tiles);
    vector<vector<double>> recon_scaled = reconstructed.get_image();
    vector<vector<int>> recon = de_scale_image(recon_scaled, 8);

    saveVectorAsJpeg(recon, "output_ram.jpg");
    

    return 0;
}