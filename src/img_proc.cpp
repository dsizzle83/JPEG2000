#include "../include/img_proc.h"

using namespace std;

ImagePreProcessor::ImagePreProcessor(string filename, bool color){
    // To best emulate HDMI inputs, RGB is preferred
    in_color = color;
    if(!in_color){
        image = cv::imread(filename, cv::IMREAD_GRAYSCALE);
        comp_1 = read_gray_image_to_vector(image);
        bit_depth = image.elemSize() * 8/ image.channels();
        rows = image.rows;
        cols = image.cols;
        scaled_comp_1 = scale_luminance(comp_1, bit_depth, rows, cols);
    }
    else{
        image = cv::imread(filename, cv::IMREAD_COLOR);
        bit_depth = image.elemSize() * 8 / image.channels();
        rows = image.rows;
        cols = image.cols;
        if(image.channels() == 3){
            // Sucessfully read a color image
            extract_components(image);
            color_convert_and_scale();
        }
        else if(image.channels() == 1){
            // Loaded a gray image instead of a color image
            cout << "Image read as grayscale!\n";
            comp_1 = read_gray_image_to_vector(image);
            scaled_comp_1 = scale_luminance(comp_1, bit_depth, rows, cols);
        }
        else{
            cout << "Wrong number of channels: " << (int)image.channels() << endl;
        }

    }
}

vector<int> ImagePreProcessor::read_gray_image_to_vector(cv::Mat &img) {
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

vector<double> ImagePreProcessor::scale_chrominance(vector<double> &img, int bits, int height, int width){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<double> scaled_image(img.size(), 0);

    for(size_t i=0; i<height; i++){
        for(size_t j=0; j<width; j++){
            scaled_image[i*width + j] = static_cast<double>(img[i*width + j])/ dynamic_range;
        }
    }
    return scaled_image;
}

vector<double> ImagePreProcessor::scale_luminance(vector<double> &img, int bits, int height, int width){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<double> scaled_image(img.size(), 0);

    for(size_t i=0; i<height; i++){
        for(size_t j=0; j<width; j++){
            scaled_image[i*width + j] = (img[i*width + j])/ dynamic_range - 0.5;
        }
    }
    return scaled_image;
}

vector<double> ImagePreProcessor::scale_luminance(vector<int> &img, int bits, int height, int width){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<double> scaled_image(img.size(), 0);

    for(size_t i=0; i<height; i++){
        for(size_t j=0; j<width; j++){
            scaled_image[i*width + j] = (static_cast<double>(img[i*width + j]))/ dynamic_range - 0.5;
        }
    }
    return scaled_image;
}

void ImagePreProcessor::extract_components(cv::Mat &img){
    vector<int>R(img.rows * img.cols, 0);
    vector<int>G(img.rows * img.cols, 0);
    vector<int>B(img.rows * img.cols, 0);

    for(int i=0; i<img.rows; i++){
        for(int j=0; j<img.cols; j++){
            B[i*img.cols + j] = img.at<cv::Vec3b>(i, j)[0];
            G[i*img.cols + j] = img.at<cv::Vec3b>(i, j)[1];
            R[i*img.cols + j] = img.at<cv::Vec3b>(i, j)[2];
        }
    }
    comp_0 = R;
    comp_1 = G;
    comp_2 = B;
}

void ImagePreProcessor::color_convert_and_scale(){
    vector<double> Y(rows*cols, 0);
    vector<double> Cb(rows*cols, 0);
    vector<double> Cr(rows*cols, 0);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            Y[i*cols + j] = 0.299 * static_cast<double>(comp_0[i*cols + j]) +
                0.587 * static_cast<double>(comp_1[i*cols + j]) + 
                0.114 * static_cast<double>(comp_2[i*cols + j]);
            Cb[i*cols + j] = -0.16875 * static_cast<double>(comp_0[i*cols + j]) +
                -0.33126 * static_cast<double>(comp_1[i*cols + j]) + 
                0.5 * static_cast<double>(comp_2[i*cols + j]);
            Cr[i*cols + j] = 0.5 * static_cast<double>(comp_0[i*cols + j]) +
                -0.41869 * static_cast<double>(comp_1[i*cols + j]) + 
                -0.08131 * static_cast<double>(comp_2[i*cols + j]);
        }
    }
    scaled_comp_0 = scale_luminance(Y, bit_depth, rows, cols);
    scaled_comp_1 = scale_chrominance(Cb, bit_depth, rows, cols);
    scaled_comp_2 = scale_chrominance(Cr, bit_depth, rows, cols);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ImagePostProcessor::ImagePostProcessor(vector<double> recon_comp_0, int bits, int height, int width){
    // If only one component is given, only generate one image
    bit_depth = bits;
    rows = height;
    cols = width;
    scaled_comp_0 = recon_comp_0;
    comp_0 = de_scale_luminance(scaled_comp_0, bit_depth, rows, cols);
}
ImagePostProcessor::ImagePostProcessor(vector<double> recon_comp_0, vector<double> recon_comp_1, vector<double> recon_comp_2, int bits, int height, int width){
    bit_depth = bits;
    rows = height;
    cols = width;
    scaled_comp_0 = recon_comp_0;
    scaled_comp_1 = recon_comp_1;
    scaled_comp_2 = recon_comp_2;
    color_convert_and_descale();
    image = combine_components(comp_0, comp_1, comp_2, bit_depth, rows, cols);
}

vector<int> ImagePostProcessor::read_gray_image_to_vector(cv::Mat &img) {
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

vector<int> ImagePostProcessor::de_scale_luminance(vector<double> &scaled_image, int bits, int rows, int cols){
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

vector<int> ImagePostProcessor::de_scale_chrominance(vector<double> &scaled_image, int bits, int rows, int cols){
    double dynamic_range = pow(2.0, static_cast<double>(bits));
    vector<int> de_scaled_image(scaled_image.size(), 0);

    for(size_t i=0; i<rows; i++){
        for(size_t j=0; j<cols; j++){
            double val = scaled_image[i*cols + j];
            if(val <= -0.5) val = -0.5;
            else if(val >= 0.5) val = 0.5;
            val *= dynamic_range;
            if(val < -(dynamic_range / 2.0)) val = -(dynamic_range / 2.0);
            else if(val > (dynamic_range/2.0 - 1)) val = dynamic_range/2.0 - 1;
            int int_val = round(val);
            de_scaled_image[i*cols + j] = val;
        }
    }
    return de_scaled_image;
}

void ImagePostProcessor::color_convert_and_descale(){
    vector<double> R(rows*cols, 0);
    vector<double> G(rows*cols, 0);
    vector<double> B(rows*cols, 0);

    comp_0 = de_scale_luminance(scaled_comp_0, bit_depth, rows, cols);
    comp_1 = de_scale_chrominance(scaled_comp_1, bit_depth, rows, cols);
    comp_2 = de_scale_chrominance(scaled_comp_2, bit_depth, rows, cols);

    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            R[i*cols + j] = static_cast<double>(comp_0[i*cols + j]) +
                            1.402 * static_cast<double>(comp_2[i*cols + j]);
            G[i*cols + j] = static_cast<double>(comp_0[i*cols + j]) +
                            -0.34413 * static_cast<double>(comp_1[i*cols + j]) + 
                            -0.71414 * static_cast<double>(comp_2[i*cols + j]);
            B[i*cols + j] = static_cast<double>(comp_0[i*cols + j]) +
                            1.772 * static_cast<double>(comp_1[i*cols + j]);
        }
    }
    vector<int>().swap(comp_0);
    vector<int>().swap(comp_1);
    vector<int>().swap(comp_2);

    comp_0 = double_to_int(R);
    comp_1 = double_to_int(G);
    comp_2 = double_to_int(B);        
}

vector<int> ImagePostProcessor::double_to_int(vector<double> double_vec){
    vector<int> int_vec(double_vec.size(), 0);
    for(size_t i=0; i<double_vec.size(); i++){
        int_vec[i] = static_cast<int>(round(double_vec[i]));
    }
    return int_vec;
}

cv::Mat ImagePostProcessor::combine_components(const vector<int>& red, const vector<int>& green, const vector<int>& blue, int bits, int height, int width) {
    if (red.size() != green.size() || red.size() != blue.size() || red.size() != height * width) {
        std::cerr << "Error: Vectors must be the same size and match image dimensions.\n";
        return cv::Mat();
    }

    int maxVal = (1 << bits) - 1;

    // Use 8-bit or 16-bit format based on `bits`
    int type = (bits > 8) ? CV_16UC3 : CV_8UC3;
    cv::Mat image(height, width, type);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx = i * width + j;
            int r = std::clamp(red[idx], 0, maxVal);
            int g = std::clamp(green[idx], 0, maxVal);
            int b = std::clamp(blue[idx], 0, maxVal);

            if (bits > 8) {
                image.at<cv::Vec3w>(i, j) = cv::Vec3w(b, g, r);  // 16-bit: Vec3w = Vec<uint16_t, 3>
            } else {
                image.at<cv::Vec3b>(i, j) = cv::Vec3b(b, g, r);  // 8-bit: Vec3b = Vec<uchar, 3>
            }
        }
    }

    return image;
}

cv::Mat ImagePostProcessor::create_cv2_image(const vector<int>& gray, int height, int width, int bits) {
    if (gray.size() != height * width) {
        std::cerr << "Error: Vector size must match image dimensions.\n";
        return cv::Mat();
    }

    int maxVal = (1 << bits) - 1;
    int type = (bits > 8) ? CV_16UC1 : CV_8UC1;
    cv::Mat image(height, width, type);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx = i * width + j;
            int val = std::clamp(gray[idx], 0, maxVal);

            if (bits > 8)
                image.at<uint16_t>(i, j) = val;
            else
                image.at<uchar>(i, j) = static_cast<uchar>(val);
        }
    }

    return image;
}

void ImagePostProcessor::save_image(const string& filename) {
    if (image.empty()) {
        std::cerr << "Error: Image vector is empty\n";
        return;
    }
    cv::imwrite(filename, image);
}