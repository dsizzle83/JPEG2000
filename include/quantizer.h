#ifndef QUANTIZER_H
#define QUANTIZER_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "my_struct.h"

using namespace std;

class Quantizer{
private:
    vector<vector<double>> coeffs;
    int levels;
    vector<subband_info> band_info;
    vector<vector<pair<uint16_t, int8_t>>> quant_coeffs;
    int max_bits = 10;

public:
    Quantizer(vector<vector<double>> &dwt_coeffs, int lvls);
    vector<subband_info> get_band_info(){ return band_info; }
    vector<vector<pair<uint16_t, int8_t>>> get_quant_coeffs(){ return quant_coeffs; }
    double get_max(vector<vector<double>> &a);
    double calc_step_size_std_dev(vector<vector<double>> &subband);
    double calc_base_step_size(vector<vector<double>> &a, int n);
    double calc_step_size(double base_step_size, int level);
    int calc_exponent(double step_size);    
    int calc_mantissa(double step_size, int exponent);    
    double calc_bit_depth(double step_size, vector<vector<double>> &a);
    subband_info make_bands_from_middle(point bot_r, point middle, int type, int level);
    subband_info quantize_bands(subband_info b, vector<vector<double>> &a, double base_step_size);
    void process_subbands(vector<vector<double>> &a, int lvls);
    void print_quantized();
    void print_subband_info(subband_info b);
    void print_band_info();
};

class DeQuantizer{
private:
    vector<vector<double>> coeffs;
    int levels;
    vector<subband_info> band_info;
    vector<vector<pair<uint16_t, int8_t>>> quant_coeffs;

public:
    DeQuantizer(vector<vector<pair<uint16_t, int8_t>>> &quantized_coeffs, vector<subband_info> bands, int lvls);
    vector<subband_info> get_band_info(){ return band_info; }
    vector<vector<pair<uint16_t, int8_t>>> get_quant_coeffs(){ return quant_coeffs; }
    vector<vector<double>> get_coeffs(){ return coeffs;}
    vector<vector<double>> process_quant_coeffs(vector<vector<pair<uint16_t, int8_t>>> &q, vector<subband_info> b, int lvls);
    void print_2D_vector(vector<vector<double>> &a);
    void print_quantized();
    void print_subband_info(subband_info b);
    void print_band_info();
};

#endif