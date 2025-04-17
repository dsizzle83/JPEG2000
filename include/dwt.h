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
    vector<vector<double>> image;
    vector<vector<double>> transformed;
    int rows;
    int cols;
    point origin;
    point extent;

public:
    FDWT(vector<vector<double>> &img, point o, int lvls);
    vector<vector<int>> get_transformed_int();
    vector<vector<double>> get_transformed();
    vector<vector<double>> multi_level_dwt(vector<vector<double>> &img, int lvls);
    vector<vector<double>> int_to_double(vector<vector<int>> &int_vec);
    vector<vector<int>> double_to_int(vector<vector<double>> &double_vec);
    vector<vector<double>> sd2d(vector<vector<double>> &a);
    vector<vector<double>> ver_sd(vector<vector<double>> &a);
    vector<double> hor_to_vert(vector<vector<double>> &a, int u);
    vector<vector<double>> replace_column(vector<vector<double>> &a, vector<double> y, int u);
    vector<vector<double>> hor_sd(vector<vector<double>> &a);
    vector<vector<double>> deint2(vector<vector<double>> &a);
    vector<double> sd1d(vector<double> &x, int i_0);
    vector<double> ext1d(vector<double> &x, int extend_left, int extend_right);
    vector<double> filt1d(vector<double> &x, int i_0, int i_1);
    vector<double> trim_vector(vector<double> &y, int ext_left, int ext_right);
    void print_2D_vector(vector<vector<double>> &a);
    void print_2D_vector(vector<vector<int>> &a);
    void print_vector(vector<double> &a);
    void print_vector(vector<int> &a);
};

class IDWT{
private:
    int level;
    vector<vector<double>> transformed;
    vector<vector<double>> image;
    int rows;
    int cols;
    point origin;
    point extent;

public:
    IDWT(vector<vector<double>> &t, point o, int lvl);
    vector<vector<double>> get_image();
    vector<vector<double>> multilevel_idwt(vector<vector<double>> &t, int lvl);
    vector<vector<double>> sr2d(vector<vector<double>> &a);
    vector<vector<double>> int_to_double(vector<vector<int>> &int_vec);
    vector<vector<int>> double_to_int(vector<vector<double>> &double_vec);
    vector<vector<double>> int2d(vector<vector<double>> &a);
    vector<vector<double>> hor_sr(vector<vector<double>> &a);
    vector<vector<double>> ver_sr(vector<vector<double>> &a);
    vector<double> hor_to_vert(vector<vector<double>> &a, int u);
    vector<vector<double>> replace_column(vector<vector<double>> &a, vector<double> &y, int u);
    vector<double> sr1d(vector<double> &y, int i_0);
    vector<double> ext1d(vector<double> &x, int extend_left, int extend_right);
    vector<double> filt1d(vector<double> &y, int i_0, int i_1);
    vector<double> trim_vector(vector<double> &y, int ext_left, int ext_right);
    void print_2D_vector(vector<vector<double>> &a);
    void print_2D_vector(vector<vector<int>> &a);
    void print_vector(vector<double> &a);
    void print_vector(vector<int> &a);
};

#endif