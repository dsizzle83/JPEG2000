#include <iostream>
#include <cmath>
#include <vector>


#include "../include/my_struct.h"
#include "../include/tiling.h"

using namespace std;


Tiles::Tiles(vector<vector<double>> &scaled_image){
    image = scaled_image;
    max_height = 512;
    max_width = 512;
    image_tiles = break_image_into_tiles(image);
}

Tiles::Tiles(vector<vector<double>> &scaled_image, size_t rows, size_t cols){
    image = scaled_image;
    max_height = rows;
    max_width = cols;
    image_tiles = break_image_into_tiles(image);
}

Tiles::Tiles(vector<tile> &tile_vector){
    image_tiles = tile_vector;
    max_height = 512;
    max_width = 512;
    image = reconstruct_image_from_tiles(image_tiles);
}

Tiles::Tiles(vector<tile> &tile_vector, size_t rows, size_t cols){
    image_tiles = tile_vector;
    max_height = rows;
    max_width = cols;
    image = reconstruct_image_from_tiles(image_tiles);
}

vector<tile> Tiles::break_image_into_tiles(const vector<vector<double>> &img){
    vector<tile> tile_vector;
    point anchor = {0, 0};
    while(anchor.y < static_cast<int>(img.size())){
        anchor.x = 0;
        while(anchor.x < static_cast<int>(img[0].size())){
            tile temp_tile;
            temp_tile.anchor = anchor;
            temp_tile.tile_data = copy_tile_data_from_image(img, anchor, max_height, max_width);
            temp_tile.height = temp_tile.tile_data.size();
            temp_tile.width = temp_tile.tile_data[0].size();
            tile_vector.push_back(temp_tile);
            anchor.x += max_width;
        }
        anchor.y += max_height;
    }
    return tile_vector;
}

vector<vector<double>> Tiles::copy_tile_data_from_image(const vector<vector<double>> &img, point anchor, size_t max_rows, size_t max_cols){
    vector<vector<double>> data;
    for(int i=anchor.y; i<anchor.y + static_cast<int>(max_rows); i++){
        if(i >= static_cast<int>(img.size())) break;
        vector<double> temp_row;
        for(int j=anchor.x; j<anchor.x + static_cast<int>(max_cols); j++){
            if(j >= static_cast<int>(img[0].size())) break;
            temp_row.push_back(img[i][j]);
        }
        data.push_back(temp_row);
    }
    return data;
}

vector<vector<double>> Tiles::reconstruct_image_from_tiles(const vector<tile> &tile_vector){
    point bot_r = find_bottom_right(tile_vector);
    vector<vector<double>> de_tiled_image(bot_r.y, vector<double>(bot_r.x, 0));

    for(tile t : tile_vector){
        de_tiled_image = append_tile_data_to_image(de_tiled_image, t);
    }
    return de_tiled_image;
}

point Tiles::find_bottom_right(const vector<tile> &tile_vector){
    int max_x = 0;
    int max_y = 0;
    for(const tile &t : tile_vector){
        if(t.anchor.y + static_cast<int>(t.height) > max_y) max_y = t.anchor.y + t.height;
        if(t.anchor.x + static_cast<int>(t.width) > max_x) max_x = t.anchor.x + t.width;
    }
    point bottom_right = {max_x, max_y};
    return bottom_right;
}

vector<vector<double>> Tiles::append_tile_data_to_image(vector<vector<double>> &a, const tile &t){
    for(int i=t.anchor.y; i<t.anchor.y + static_cast<int>(t.height); i++){
        for(int j=t.anchor.x; j<t.anchor.x + static_cast<int>(t.width); j++){
            a[i][j] = t.tile_data[i-t.anchor.y][j-t.anchor.x];
        }
    }
    return a;
}