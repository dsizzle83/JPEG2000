#ifndef IMG_PROC_H
#define IMG_PROC_H

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>

#include "../include/my_struct.h"

using namespace std;

class ImagePreProcessor{
private:
    cv::Mat image;
    vector<int> comp_0;
    vector<int> comp_1;
    vector<int> comp_2;
    bool in_color;
    vector<double> scaled_comp_0;
    vector<double> scaled_comp_1;
    vector<double> scaled_comp_2;
    int bit_depth;
    int rows;
    int cols;

public:
    ImagePreProcessor(string filename, bool color);
    cv::Mat get_image(){ return image; }
    vector<int> get_comp_0(){ return comp_0; }
    vector<int> get_comp_1(){ return comp_1; }
    vector<int> get_comp_2(){ return comp_2; }
    vector<double> get_scaled_comp_0(){ return scaled_comp_0; }
    vector<double> get_scaled_comp_1(){ return scaled_comp_1; }
    vector<double> get_scaled_comp_2(){ return scaled_comp_2; }
    int get_bit_depth(){ return bit_depth; }
    int get_rows() { return rows; }
    int get_cols(){ return cols; }
    vector<int> read_gray_image_to_vector(cv::Mat &img);
    vector<double> scale_chrominance(vector<double> &img, int bits, int height, int width);
    vector<double> scale_luminance(vector<double> &img, int bits, int height, int width);
    vector<double> scale_luminance(vector<int> &img, int bits, int height, int width);
    void extract_components(cv::Mat &img);
    void color_convert_and_scale();
};

class ImagePostProcessor{
private:
    cv::Mat image;
    vector<int> comp_0;
    vector<int> comp_1;
    vector<int> comp_2;
    vector<double> scaled_comp_0;
    vector<double> scaled_comp_1;
    vector<double> scaled_comp_2;
    int bit_depth;
    int rows;
    int cols;

public:
    ImagePostProcessor(vector<double> recon_comp_0, int bits, int height, int width);
    ImagePostProcessor(vector<double> recon_comp_0, vector<double> recon_comp_1, vector<double> recon_comp_2, int bits, int height, int width);
    vector<int> get_comp_0(){ return comp_0; }
    vector<int> get_comp_1(){ return comp_1; }
    vector<int> get_comp_2(){ return comp_2; }
    vector<double> get_scaled_comp_0(){ return scaled_comp_0; }
    vector<double> get_scaled_comp_1(){ return scaled_comp_1; }
    vector<double> get_scaled_comp_2(){ return scaled_comp_2; }
    int get_bit_depth(){ return bit_depth; }
    int get_rows() { return rows; }
    int get_cols(){ return cols; }
    cv::Mat get_image(){ return image; }
    vector<int> read_gray_image_to_vector(cv::Mat &img);
    vector<int> de_scale_luminance(vector<double> &scaled_image, int bits, int rows, int cols);
    vector<int> de_scale_chrominance(vector<double> &scaled_image, int bits, int rows, int cols);
    void color_convert_and_descale();
    vector<int> double_to_int(vector<double> double_vec);
    cv::Mat combine_components(const vector<int>& red, const vector<int>& green, const vector<int>& blue, int bits, int height, int width);
    cv::Mat create_cv2_image(const vector<int>& gray, int height, int width, int bits);
    void save_image(const string& filename);
};

#endif