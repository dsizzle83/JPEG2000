#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "../include/dwt.h"
#include "../include/my_struct.h"

using namespace std;

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

class Quantizer{
private:
    vector<vector<double>> coeffs;
    int levels;
    vector<subband_info> band_info;
    vector<vector<pair<uint16_t, bool>>> quant_coeffs;
    int max_bits = 10;

public:
    Quantizer(vector<vector<double>> dwt_coeffs, int lvls){
        coeffs = dwt_coeffs;
        levels = lvls;
        // initialize the 2-D paired vector
        vector<vector<pair<uint16_t, bool>>> q(dwt_coeffs.size(), vector<pair<uint16_t, bool>>(dwt_coeffs[0].size(), make_pair(0, false)));
        quant_coeffs = q;
        process_subbands(coeffs, levels);
    }

    vector<subband_info> get_band_info(){ return band_info; }
    vector<vector<pair<uint16_t, bool>>> get_quant_coeffs(){ return quant_coeffs; }

    double get_max(vector<vector<double>> a){
        double max = 0.0;
        for(auto&row : a){
            for(double val : row){
                if(abs(val) > max) max = abs(val);
            }
        }
        return max;
    }

    double calc_step_size_std_dev(vector<vector<double>> subband){
        double sum = 0.0;
        double num_elements = 0.0;
        for(auto&row : subband){
            for(double val : row){
                sum += val;
                num_elements += 1;
            }
        }
        double mean = sum / num_elements;
        
        double square_diff_sum = 0.0;
        for(auto&row : subband){
            for(double val : row){
                square_diff_sum += pow(val - mean, 2);
            }
        }
        
        double sigma = sqrt(square_diff_sum / num_elements);
        double step_b = (2 * sigma) / sqrt(3);

        return step_b;
    }

    double calc_base_step_size(vector<vector<double>> a, int n){
        // The base step size is the size required to represent the max
        // value of all the coefficients in n bits
        double max = get_max(a);
        double base_step_size = max/(pow(2, n) - 1);
        return base_step_size;
    }

    double calc_step_size(double base_step_size, int level){
        return base_step_size * sqrt(pow(2, levels - level));
    }

    int calc_exponent(double step_size){
        return floor(log2(step_size));
    }
    
    int calc_mantissa(double step_size, int exponent){
        return round(((step_size / (pow(2, exponent))) - 1) * pow(2, 11));
    }

    double calc_bit_depth(double step_size, vector<vector<double>> a){
        double max = get_max(a);
        return ceil(log2(max / step_size));
    }

    subband_info make_bands_from_middle(point bot_r, point middle, int type, int level){
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

    subband_info quantize_bands(subband_info b, vector<vector<double>> a, double base_step_size){
        int rows, cols;
        rows = b.bot_r.y - b.top_l.y;
        cols = b.bot_r.x - b.top_l.x;
        vector<vector<double>> a_b(rows, vector<double>(cols, 0));
        vector<vector<pair<uint16_t, bool>>> q;

        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                a_b[i][j] = a[i + b.top_l.y][j + b.top_l.x];
            }
        }

        b.step_size = calc_step_size(base_step_size, b.level);
        b.expo = calc_exponent(b.step_size);
        b.mant = calc_mantissa(b.step_size, b.expo);
        b.bit_depth = calc_bit_depth(b.step_size, a_b);
        if(b.bit_depth > 15){
            cout << "Bit depth is too High! You used: " << b.bit_depth << endl;
        }

        for(auto&row : a_b){
            vector<pair<uint16_t, bool>> temp_row;
            for(double val : row){
                bool x = (val >= 0)? true : false;
                double mag = abs(val);
                int u = 0;
                for(int i=b.bit_depth; i>0; i--){
                    double thresh = b.step_size * pow(2, i-1);
                    if(mag >= thresh){
                        mag -= thresh;
                        u |= (0x1 << (i - 1));
                    }
                }
                temp_row.push_back(make_pair(u, x));
            }
            q.push_back(temp_row);
        }
        for(int i=b.top_l.y; i<b.bot_r.y; i++){
            for(int j=b.top_l.x; j< b.bot_r.x; j++){
                quant_coeffs[i][j] = q[i-b.top_l.y][j-b.top_l.x];
            }
        }
        return b;        
    }

    void process_subbands(vector<vector<double>> a, int lvls){
        double base_step_size = calc_base_step_size(a, max_bits);
        cout << "Base Step Size: " << base_step_size << endl;

        point bot_r, middle;
        // non-inclusive boundary for bottom
        bot_r.x = a[0].size();
        bot_r.y = a.size();
        for(int i=0; i<lvls; i++){
            middle.x = (bot_r.x + 1) / 2;
            middle.y = (bot_r.y + 1) / 2;
            subband_info hl, lh, hh;
            hl = make_bands_from_middle(bot_r, middle, 1, i + 1);
            lh = make_bands_from_middle(bot_r, middle, 2, i + 1);
            hh = make_bands_from_middle(bot_r, middle, 3, i + 1);
            hl = quantize_bands(hl, a, base_step_size);
            lh = quantize_bands(lh, a, base_step_size);
            hh = quantize_bands(hh, a, base_step_size);
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
        ll = quantize_bands(ll, a, base_step_size);
        band_info.push_back(ll);
    }
    
    void print_quantized(){
        cout << "Magnitudes:" << endl;
        for(auto&row : quant_coeffs){
            for(auto val : row){
                cout << val.first << " ";
            }
            cout << endl;
        }

        cout << "Signs:" << endl;
        for(auto&row : quant_coeffs){
            for(auto val : row){
                cout << val.second << " ";
            }
            cout << endl;
        }
    }

    void print_subband_info(subband_info b){
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

    void print_band_info(){
        for(auto&val : band_info){
            print_subband_info(val);
        }
    }

};

class DeQuantizer{
private:
    vector<vector<double>> coeffs;
    int levels;
    vector<subband_info> band_info;
    vector<vector<pair<uint16_t, bool>>> quant_coeffs;

public:
    DeQuantizer(vector<vector<pair<uint16_t, bool>>> quantized_coeffs, vector<subband_info> bands, int lvls){
        levels = lvls;
        quant_coeffs = quantized_coeffs;
        band_info = bands;
        coeffs = process_quant_coeffs(quant_coeffs, band_info, levels);
    }

    vector<subband_info> get_band_info(){ return band_info; }
    vector<vector<pair<uint16_t, bool>>> get_quant_coeffs(){ return quant_coeffs; }
    vector<vector<double>> get_coeffs(){ return coeffs;}

    vector<vector<double>> process_quant_coeffs(vector<vector<pair<uint16_t, bool>>> q, vector<subband_info> b, int lvls){
        size_t rows = q.size();
        size_t cols = q[0].size();
        vector<vector<double>> dwt_coeffs(rows, vector<double>(cols, 0));
        point top_l, bot_r;

        for(auto&subband : b){
            top_l = subband.top_l;
            bot_r = subband.bot_r;
            for(int i=top_l.y; i<bot_r.y; i++){
                for(int j=top_l.x; j<bot_r.x; j++){
                    double y;
                    double sign = q[i][j].second ? 1.0 : -1.0;
                    y = sign * static_cast<double>(q[i][j].first) + 0.5;
                    y *= subband.step_size;
                    dwt_coeffs[i][j] = y;
                }
            }
        }
        return dwt_coeffs;
    }

    void print_2D_vector(vector<vector<double>> a){
        cout << fixed << setprecision(2);
        for(const auto&row : a){
            for(double val : row){
                cout << val << "\t";
            }
            cout << endl;
        }
    }
    
    void print_quantized(){
        cout << "Magnitudes:" << endl;
        for(auto&row : quant_coeffs){
            for(auto val : row){
                cout << val.first << " ";
            }
            cout << endl;
        }

        cout << "Signs:" << endl;
        for(auto&row : quant_coeffs){
            for(auto val : row){
                cout << val.second << " ";
            }
            cout << endl;
        }
    }

    void print_subband_info(subband_info b){
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

    void print_band_info(){
        for(auto&val : band_info){
            print_subband_info(val);
        }
    }

};



int main(){
    point o;
    o.x = 0;
    o.y = 0;

    FDWT my_fdwt(example_block, o, 2);
    cout << "Coefficients: " << endl;
    my_fdwt.print_2D_vector(my_fdwt.get_transformed());

    vector<vector<double>> dwt_coeffs = my_fdwt.get_transformed();
    Quantizer my_q(dwt_coeffs, 2);
    my_q.print_quantized();
    cout << fixed << setprecision(4);
    my_q.print_band_info();
    DeQuantizer dq(my_q.get_quant_coeffs(), my_q.get_band_info(), 2);
    cout << "De-Quantized Coefficients" << endl;
    dq.print_2D_vector(dq.get_coeffs());
    IDWT my_idwt(dq.get_coeffs(), o, 2);
    cout << "Reconstructed Image" << endl;
    my_idwt.print_2D_vector(my_idwt.get_image());

    return 0;
}
