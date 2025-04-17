#include <iostream>
#include <cmath>
#include <vector>


#include "../include/my_struct.h"
#include "../include/tiling.h"

using namespace std;


Tiles::Tiles(vector<double> &scaled_image, size_t img_rows, size_t img_cols){
    image = scaled_image;
    img_height = img_rows;
    img_width = img_cols;
    // Check if the dimensions of the input image match the length of the 1-D vector
    if(image.size() != (img_rows * img_cols)){
        cout << "Image size does not match\n";
        cout << "Total elements: " << image.size();
        cout << ", Rows: " << img_rows << ", Columns: " << img_cols << endl;
    }
    max_height = 512;
    max_width = 512;
    image_tiles = break_image_into_tiles(image);
}

Tiles::Tiles(vector<double> &scaled_image, size_t img_rows, size_t img_cols, size_t tile_rows, size_t tile_cols){
    image = scaled_image;
    img_height = img_rows;
    img_width = img_cols;
    // Check if the dimensions of the input image match the length of the 1-D vector
    if(image.size() != (img_rows * img_cols)){
        cout << "Image size does not match\n";
        cout << "Total elements: " << image.size();
        cout << ", Rows: " << img_rows << ", Columns: " << img_cols << endl;
    }
    max_height = tile_rows;
    max_width = tile_cols;
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

vector<tile> Tiles::break_image_into_tiles(const vector<double> &img){
    vector<tile> tile_vector;
    point anchor = {0, 0};
    while(anchor.y < img_height){
        anchor.x = 0;
        while(anchor.x < img_width){
            tile temp_tile;
            temp_tile.anchor = anchor;
            temp_tile.tile_data = copy_tile_data_from_image(img, anchor, max_height, max_width);
            
            // Test if there is a full sized tile and adjust tile height accordingly
            if(temp_tile.anchor.y > (img_height - max_height)){
                temp_tile.height = img_height - temp_tile.anchor.y;    
            }
            else temp_tile.height = max_height;

            // Test if there is a full sized tile and adjust tile width accordingly
            if(temp_tile.anchor.x > (img_width - max_width)){
                temp_tile.width = img_width - temp_tile.anchor.x;    
            }
            else temp_tile.width = max_width;


            tile_vector.push_back(temp_tile);
            anchor.x += max_width;
        }
        anchor.y += max_height;
    }
    return tile_vector;
}

vector<double> Tiles::copy_tile_data_from_image(const vector<double> &img, point anchor, size_t max_rows, size_t max_cols){
    vector<double> data;
        for(int i=anchor.y; i<anchor.y + static_cast<int>(max_rows); i++){
        if(i >= static_cast<int>(img_height)) break;
        vector<double> temp_row;
        for(int j=anchor.x; j<anchor.x + static_cast<int>(max_cols); j++){
            if(j >= static_cast<int>(img_width)) break;
            temp_row.push_back(img[i*img_width + j]);
        }
        data.insert(data.end(), temp_row.begin(), temp_row.end());
    }
    return data;
}

vector<double> Tiles::reconstruct_image_from_tiles(const vector<tile> &tile_vector){
    point bot_r = find_bottom_right(tile_vector);
    img_height = bot_r.y;
    img_width = bot_r.x;
    vector<double> de_tiled_image(bot_r.x * bot_r.y, 0);

    for(tile t : tile_vector){
        append_tile_data_to_image(de_tiled_image, t);
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

vector<double> Tiles::append_tile_data_to_image(vector<double> &a, tile &t){
    for(int i=t.anchor.y; i<t.anchor.y + static_cast<int>(t.height); i++){
        for(int j=t.anchor.x; j<t.anchor.x + static_cast<int>(t.width); j++){
            int tile_index;
            tile_index = (i-t.anchor.y) * t.width + (j - t.anchor.x);
            if(tile_index >= (t.height * t.width)){
                cout << "Accessed beyond tile data!\n";
                cout << "tile height: " << t.height << ", tile width: " << t.width;
                cout << "tile_index: " << tile_index << endl;
            }
            a[i*img_width + j] = t.tile_data[tile_index];
        }
    }
    return a;
}