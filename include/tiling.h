#ifndef TILING_H
#define TILING_H

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

#include "my_struct.h"

using namespace std;

class Tiles{
private:
    vector<vector<double>> image;
    vector<tile> image_tiles;
    size_t max_height;
    size_t max_width;

public:
    Tiles(vector<vector<double>> &scaled_image);
    Tiles(vector<vector<double>> &scaled_image, size_t rows, size_t cols);
    Tiles(vector<tile> &tile_vector);
    Tiles(vector<tile> &tile_vector, size_t rows, size_t cols);
    vector<tile> get_tiles(){ return image_tiles; }
    vector<vector<double>> get_image(){ return image; }
    vector<tile> break_image_into_tiles(const vector<vector<double>> &img);
    vector<vector<double>> copy_tile_data_from_image(const vector<vector<double>> &img, point anchor, size_t max_rows, size_t max_cols);
    vector<vector<double>> reconstruct_image_from_tiles(const vector<tile> &tile_vector);
    point find_bottom_right(const vector<tile> &tile_vector);
    vector<vector<double>> append_tile_data_to_image(vector<vector<double>> &a, const tile &t);
};

#endif