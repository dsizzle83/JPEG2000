#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "../include/dwt.h"
#include "../include/my_struct.h"
#include "../include/quantizer.h"

using namespace std;

// This block is provided by JPEG2000 standard to test DWT
vector<double> example_block = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    3, 3, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12,
    4, 4, 4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 12,
    5, 5, 5, 5, 6, 7, 7, 8, 9, 10, 11, 12, 13,
    6, 6, 6, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13,
    7, 7, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 13,
    8, 8, 8, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14,
    9, 9, 9, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15,
    10, 10, 10, 10, 10, 11, 11, 12, 12, 13, 14, 14, 15,
    11, 11, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 16,
    12, 12, 12, 12, 12, 13, 13, 13, 14, 15, 15, 16, 16,
    13, 13, 13, 13, 13, 13, 14, 14, 15, 15, 16, 17, 17,
    14, 14, 14, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18,
    15, 15, 15, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19,
    16, 16, 16, 16, 16, 16, 17, 17, 17, 18, 18, 19, 20
};

int main(){
    point o;
    o.x = 0;
    o.y = 0;
    int height = 17;
    int width = 13;

    FDWT my_fdwt(example_block, o, 2, height, width);
    cout << "Coefficients: " << endl;
    vector<double> transformed = my_fdwt.get_transformed();
    my_fdwt.print_2D_vector(transformed, height, width);

    Quantizer my_q(transformed, 2, height, width);
    my_q.print_quantized();
    cout << fixed << setprecision(4);
    my_q.print_band_info();
    vector<pair<uint16_t, int8_t>> q_coeffs = my_q.get_quant_coeffs();
    vector<subband_info> q_band_info = my_q.get_band_info();
    DeQuantizer dq(q_coeffs, q_band_info, 2, height, width);
    cout << "De-Quantized Coefficients" << endl;
    vector<double> dq_coeffs = dq.get_coeffs();
    vector<int> rounded_coeffs;
    for(double val : dq_coeffs){
        rounded_coeffs.push_back((int)round(val));
    }
    dq.print_2D_vector(rounded_coeffs);
    IDWT my_idwt(dq_coeffs, o, 2, height, width);
    cout << "Reconstructed Image" << endl;
    vector<double> reconstructed = my_idwt.get_image();
    my_idwt.print_2D_vector(reconstructed, height, width);

    int error = 0;
    int element_count = 0;

    for(int i=0; i<reconstructed.size(); i++){
        int diff = example_block[i] - reconstructed[i];
        error += diff * diff;
        element_count++;
    }

    double MSE = static_cast<double>(error) / (static_cast<double>(element_count));

    cout << "Total MSE: " << MSE << endl;

    return 0;
}
