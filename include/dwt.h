#ifndef DWT_H
#define DWT_H

#include <iostream>
#include <vector>
#include <cmath>
#include <deque>
#include <algorithm>
#include <iomanip>
#include "my_struct.h"

using namespace std;

// DWT Constants
const double A = -1.586134342059924;
const double B = -0.052980118572961;
const double G = 0.882911075530934;
const double D = 0.443506852043971;
const double K = 1.230174104914001;

class FDWT{
private:
    int levels;
    vector<double> image;
    vector<double> transformed;
    int rows;
    int cols;
    point origin;
    point extent;

public:
    FDWT(vector<double> &img, point o, int lvls, int height, int width);
    vector<int> get_transformed_int();
    vector<double> get_transformed();
    vector<double> multi_level_dwt(vector<double> &img, int lvls);
    vector<double> int_to_double(vector<int> &int_vec);
    vector<int> double_to_int(vector<double> &double_vec);
    vector<double> sd2d(vector<double> &a, int height, int width);
    vector<double> ver_sd(vector<double> &a, int height, int width);
    vector<double> hor_to_vert(vector<double> &a, int u, int height, int width);
    vector<double> replace_column(vector<double> &a, vector<double> y, int u, int height, int width);
    vector<double> hor_sd(vector<double> &a, int height, int width);
    vector<double> deint2(vector<double> &a, int height, int width);
    vector<double> sd1d(vector<double> &x, int i_0);
    vector<double> ext1d(vector<double> &x, int extend_left, int extend_right);
    vector<double> filt1d(vector<double> &x, int i_0, int i_1);
    vector<double> trim_vector(vector<double> &y, int ext_left, int ext_right);
    void print_2D_vector(vector<double> &a, int height, int width);
    void print_2D_vector(vector<int> &a, int height, int width);
    void print_vector(vector<double> &a);
    void print_vector(vector<int> &a);
};

class IDWT{
private:
    int level;
    vector<double> transformed;
    vector<double> image;
    int rows;
    int cols;
    point origin;
    point extent;

public:
    IDWT(vector<double> &t, point o, int lvl, int height, int width);
    vector<double> get_image();
    vector<double> multilevel_idwt(vector<double> &t, int lvl);
    vector<double> sr2d(vector<double> &a, int height, int width);
    vector<double> int_to_double(vector<int> &int_vec);
    vector<int> double_to_int(vector<double> &double_vec);
    vector<double> int2d(vector<double> &a, int height , int width);
    vector<double> hor_sr(vector<double> &a, int height, int width);
    vector<double> ver_sr(vector<double> &a, int height, int width);
    vector<double> hor_to_vert(vector<double> &a, int u, int height, int width);
    vector<double> replace_column(vector<double> &a, vector<double> &y, int u, int height, int width);
    vector<double> sr1d(vector<double> &y, int i_0);
    vector<double> ext1d(vector<double> &x, int extend_left, int extend_right);
    vector<double> filt1d(vector<double> &y, int i_0, int i_1);
    vector<double> trim_vector(vector<double> &y, int ext_left, int ext_right);
    void print_2D_vector(vector<double> &a, int height, int width);
    void print_2D_vector(vector<int> &a, int height, int width);
    void print_vector(vector<double> &a);
    void print_vector(vector<int> &a);
};

#endif