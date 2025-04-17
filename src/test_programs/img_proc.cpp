#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include "../include/dwt.h"

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

void saveAsBmp(const std::vector<std::vector<int>>& imgVector, const std::string& filename) {
    if (imgVector.empty()) {
        cerr << "Error: Image vector is empty\n";
        return;
    }

    int rows = imgVector.size(), cols = imgVector[0].size();
    cv::Mat img(rows, cols, CV_8U);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            img.at<uchar>(i, j) = imgVector[i][j];

    cv::imwrite(filename, img);
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

int main() {
    string filename = "animal-9347331.jpg";
    auto imgVector = readJpegToVector(filename);

    cout << "Image size: " << imgVector.size() << "x" << imgVector[0].size() << endl;

    vector<vector<double>> scaled_image(imgVector.size(), vector<double>(imgVector[0].size(), 0));

    for(size_t i=0; i<imgVector.size(); i++){
        for(size_t j=0; j<imgVector[0].size(); j++){
            scaled_image[i][j] = static_cast<double>(imgVector[i][j])/ 256.0 - 0.5;
        }
    }
    cout << "Done Scaling" << endl;

    point origin = {0, 0};
    FDWT my_fdwt(scaled_image, origin, 3);
    
    cout << "Done with Forward DWT" << endl;

    vector<vector<double>> transformed = my_fdwt.get_transformed();


    vector<vector<int>> scaled_transform;
    for(auto&row : transformed){
        vector<int> temp_row;
        for(double val : row){
            val = (val + 0.5) * 256;
            if(val < 0) val = 0;
            else if(val > 255) val = 255;
            temp_row.push_back(static_cast<int>(val));
        }
        scaled_transform.push_back(temp_row);
    }
    
    // displayVector(scaled_transform);

    IDWT my_idwt(transformed, origin, 3);
    cout << "Done with Inverse DWT" << endl;
    vector<vector<double>> reconstructed = my_idwt.get_image();

    vector<vector<int>> scaled_reconstructed;
    for(auto&row : reconstructed){
        vector<int> temp_row;
        for(double val : row){
            val = (val + 0.5) * 256;
            if(val < 0) val = 0;
            else if(val > 255) val = 255;
            temp_row.push_back(static_cast<int>(val));
        }
        scaled_reconstructed.push_back(temp_row);
    }
    displayVector(scaled_reconstructed);
    



    return 0;
}