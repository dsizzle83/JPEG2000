#include "../include/dwt.h"

// This block is provided by JPEG2000 standard to test DWT
vector<vector<double>> example_block = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {3, 3, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12},
    {4, 4, 4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 12},
    {5, 5, 5, 5, 6, 7, 7, 8, 9, 10, 11, 12, 13},
    {6, 6, 6, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13},
    {7, 7, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 13},
    {8, 8, 8, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14},
    {9, 9, 9, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15},
    {10, 10, 10, 10, 10, 11, 11, 12, 12, 13, 14, 14, 15},
    {11, 11, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 16},
    {12, 12, 12, 12, 12, 13, 13, 13, 14, 15, 15, 16, 16},
    {13, 13, 13, 13, 13, 13, 14, 14, 15, 15, 16, 17, 17},
    {14, 14, 14, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18},
    {15, 15, 15, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19},
    {16, 16, 16, 16, 16, 16, 17, 17, 17, 18, 18, 19, 20}
};


int main(){
    point tile_origin = {0, 0};
    FDWT my_dwt(example_block, tile_origin, 2);
    vector<vector<double>> transformed = my_dwt.get_transformed();

    cout << "Transformed: " << endl;
    for(const auto& row: transformed){
        for(int val : row){
            cout << val << " ";
        }
        cout << endl;
    }

    IDWT my_idwt(transformed, tile_origin, 2);
    vector<vector<double>> reconstructed = my_idwt.get_image();

    cout << "Reconstructed: " << endl;
    for(const auto& row: reconstructed){
        for(double val : row){
            cout << val << " ";
        }
        cout << endl;
    }

    int error = 0;
    int element_count = 0;

    for(int i=0; i<reconstructed.size(); i++){
        for(int j=0; j<reconstructed[0].size(); j++){
            int diff = example_block[i][j] - reconstructed[i][j];
            error += diff * diff;
            element_count++;
        }
    }

    double MSE = static_cast<double>(error) / (static_cast<double>(element_count));

    cout << "Total MSE: " << MSE << endl;


    return 0;
}