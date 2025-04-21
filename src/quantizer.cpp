#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "../include/quantizer.h"

using namespace std;

Quantizer::Quantizer(vector<double> &dwt_coeffs, int lvls, int height, int width){
    coeffs = dwt_coeffs;
    levels = lvls;
    rows = height;
    cols = width;
    // initialize the 2-D paired vector
    vector<pair<uint16_t, int8_t>> q(dwt_coeffs.size(), make_pair(0, false));
    quant_coeffs = q;
    process_subbands(coeffs, levels);
}

double Quantizer::get_max(vector<double> &a){
    double max = 0.0;
    for(double val : a){
        if(abs(val) > max) max = abs(val);
    }
    return max;
}

double Quantizer::calc_step_size_std_dev(vector<double> &subband){
    double sum = 0.0;
    double num_elements = 0.0;
    for(double val : subband){
        sum += val;
        num_elements += 1;
    }
    double mean = sum / num_elements;
    
    double square_diff_sum = 0.0;
    
    for(double val : subband){
        square_diff_sum += pow(val - mean, 2);
    }
    
    double sigma = sqrt(square_diff_sum / num_elements);
    double step_b = (2 * sigma) / sqrt(3);

    return step_b;
}

double Quantizer::calc_base_step_size(vector<double> &a, int n){
    // The base step size is the size required to represent the max
    // value of all the coefficients in n bits
    double max = get_max(a);
    double base_step_size = max/(pow(2, n) - 1);
    return base_step_size;
}

double Quantizer::calc_step_size(double base_step_size, int level){
    return base_step_size * sqrt(pow(2, levels - level));
}

int Quantizer::calc_exponent(double step_size){
    return floor(log2(step_size));
}

int Quantizer::calc_mantissa(double step_size, int exponent){
    return round(((step_size / (pow(2, exponent))) - 1) * pow(2, 11));
}

double Quantizer::calc_bit_depth(double step_size, vector<double> &a){
    double max = get_max(a);
    return ceil(log2((max + step_size) / step_size));
}

subband_info Quantizer::make_bands_from_middle(point bot_r, point middle, int type, int level){
    subband_info b;
    b.level = level;
    switch(type){
        case 0:
            b.type = 0;
            b.top_l.x = 0;
            b.top_l.y = 0;
            b.bot_r.x = middle.x;
            b.bot_r.y = middle.y;
            break;
        case 1:
            b.type = 1;
            b.top_l.x = middle.x;
            b.top_l.y = 0;
            b.bot_r.x = bot_r.x;
            b.bot_r.y = middle.y;
            break;
        case 2:
            b.type = 2;
            b.top_l.x = 0;
            b.top_l.y = middle.y;
            b.bot_r.x = middle.x;
            b.bot_r.y = bot_r.y;
            break;
        case 3:
            b.type = 3;
            b.top_l.x = middle.x;
            b.top_l.y = middle.y;
            b.bot_r.x = bot_r.x;
            b.bot_r.y = bot_r.y;
            break;
        default:
            cout << "Invalid subband type! You used: " << type << endl;
            return b;
            break;
    }
    return b;       
}

subband_info Quantizer::quantize_bands(subband_info b, vector<double> &a, double base_step_size, int height, int width){
    int rows, cols;
    rows = b.bot_r.y - b.top_l.y;
    cols = b.bot_r.x - b.top_l.x;
    vector<double> a_b(rows * cols, 0);
    vector<pair<uint16_t, int8_t>> q;

    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            a_b[i*cols + j] = a[(i + b.top_l.y) * width + (j + b.top_l.x)];
        }
    }

    b.step_size = calc_step_size(base_step_size, b.level);
    b.expo = calc_exponent(b.step_size);
    b.mant = calc_mantissa(b.step_size, b.expo);
    b.bit_depth = calc_bit_depth(b.step_size, a_b);
    if(b.bit_depth > 15){
        cout << "Bit depth is too High! You used: " << b.bit_depth << endl;
    }

    for(double val : a_b){
        pair<uint16_t, int8_t> temp_pair;
        int8_t x = (val >= 0)? 1 : -1;
        double mag = abs(val);
        int u = 0;
        for(int i=b.bit_depth; i>0; i--){
            double thresh = b.step_size * pow(2, i-1);
            if(mag >= thresh){
                mag -= thresh;
                u |= (0x1 << (i - 1));
            }
        }
        temp_pair = make_pair(u, x);
        
        q.push_back(temp_pair);
    }

    for(int i=b.top_l.y; i<b.bot_r.y; i++){
        for(int j=b.top_l.x; j< b.bot_r.x; j++){
            quant_coeffs[i* cols + j] = q[(i-b.top_l.y) * (b.bot_r.x - b.top_l.x) + (j-b.top_l.x)];
        }
    }
    return b;        
}

void Quantizer::process_subbands(vector<double> &a, int lvls){
    double base_step_size = calc_base_step_size(a, max_bits);

    point bot_r, middle;
    // non-inclusive boundary for bottom
    bot_r.x = cols;
    bot_r.y = cols;
    for(int i=0; i<lvls; i++){
        middle.x = (bot_r.x + 1) / 2;
        middle.y = (bot_r.y + 1) / 2;
        subband_info hl, lh, hh;
        hl = make_bands_from_middle(bot_r, middle, 1, i + 1);
        lh = make_bands_from_middle(bot_r, middle, 2, i + 1);
        hh = make_bands_from_middle(bot_r, middle, 3, i + 1);
        hl = quantize_bands(hl, a, base_step_size, middle.y, (bot_r.x - middle.x));
        lh = quantize_bands(lh, a, base_step_size, (bot_r.y - middle.y), middle.x);
        hh = quantize_bands(hh, a, base_step_size, (bot_r.y - middle.y), (bot_r.x - middle.x));
        band_info.push_back(hl);
        band_info.push_back(lh);
        band_info.push_back(hh);
        bot_r = middle;
    }
    subband_info ll;
    ll.top_l.x = 0;
    ll.top_l.y = 0;
    ll.bot_r = bot_r;
    ll.type = 0;
    ll.level = lvls;
    ll = quantize_bands(ll, a, base_step_size, ll.bot_r.y, ll.bot_r.x);
    band_info.push_back(ll);
}

void Quantizer::print_quantized(){
    cout << "Magnitudes:" << endl;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            cout << (int)quant_coeffs[i*cols + j].first << "\t";
        }
        cout << endl;
    }

    cout << "Signs:" << endl;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            cout << (int)quant_coeffs[i*cols + j].second << "\t";
        }
        cout << endl;
    }
}

void Quantizer::print_subband_info(subband_info b){
    cout << "level: " << b.level << endl;
    cout << "type: " << b.type << endl;
    cout << "step size: " << b.step_size << endl;
    cout << "exponent: " << b.expo << endl;
    cout << "mantissa: " << b.mant << endl;
    cout << "bit_depth: " << b.bit_depth << endl;
    cout << "top left: (" << b.top_l.x << ", " << b.top_l.y << ")" << endl;
    cout << "bottom right: (" << b.bot_r.x << ", " << b.bot_r.y << ")" << endl;
    cout << "----------------------------------------------------------" << endl;

}

void Quantizer::print_band_info(){
    for(auto&val : band_info){
        print_subband_info(val);
    }
}

// Start of DeQuantizer Class
DeQuantizer::DeQuantizer(vector<pair<uint16_t, int8_t>> &quantized_coeffs, vector<subband_info> bands, int lvls, int height, int width){
    levels = lvls;
    rows = height;
    cols = width;
    quant_coeffs = quantized_coeffs;
    band_info = bands;
    coeffs = process_quant_coeffs(quant_coeffs, band_info, levels);
}

vector<double> DeQuantizer::process_quant_coeffs(vector<pair<uint16_t, int8_t>> &q, vector<subband_info> b, int lvls){
    vector<double> dwt_coeffs(rows * cols, 0);
    point top_l, bot_r;

    for(auto&subband : b){
        top_l = subband.top_l;
        bot_r = subband.bot_r;
        for(int i=top_l.y; i<bot_r.y; i++){
            for(int j=top_l.x; j<bot_r.x; j++){
                double y;
                double sign = static_cast<double>(q[i*cols + j].second);
                y = sign * static_cast<double>(q[i*cols + j].first) + 0.5;
                y *= subband.step_size;
                dwt_coeffs[i*cols + j] = y;
            }
        }
    }
    return dwt_coeffs;
}

void DeQuantizer::print_2D_vector(vector<double> &a){
    cout << fixed << setprecision(2);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            cout << a[i*cols + j] << "\t";
        }
        cout << endl;
    }
}

void DeQuantizer::print_quantized(){
    cout << "Magnitudes:" << endl;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            cout << (int)quant_coeffs[i*cols + j].first << "\t";
        }
        cout << endl;
    }

    cout << "Signs:" << endl;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            cout << (int)quant_coeffs[i*cols + j].second << "\t";
        }
        cout << endl;
    }
}

void DeQuantizer::print_subband_info(subband_info b){
    cout << "level: " << b.level << endl;
    cout << "type: " << b.type << endl;
    cout << "step size: " << b.step_size << endl;
    cout << "exponent: " << b.expo << endl;
    cout << "mantissa: " << b.mant << endl;
    cout << "bit_depth: " << b.bit_depth << endl;
    cout << "top left: (" << b.top_l.x << ", " << b.top_l.y << ")" << endl;
    cout << "bottom right: (" << b.bot_r.x << ", " << b.bot_r.y << ")" << endl;
    cout << "----------------------------------------------------------" << endl;

}

void DeQuantizer::print_band_info(){
    for(auto&val : band_info){
        print_subband_info(val);
    }
}