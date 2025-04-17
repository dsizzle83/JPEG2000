#include <vector>
#include <deque>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iterator>
#include "../include/my_struct.h"
#include "../include/mq_coder.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"
#include "../include/code_block.h"
#include "../include/bp_coder.h"

using namespace std;

vector<vector<uint8_t>> s_d = {
{0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
{1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1},
{0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0},
{1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0},
{1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1},
{1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
{1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0},
{1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0},
{1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0},
{0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1},
{0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1},
{0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1},
{0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1},
{0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1},
{0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0},
{1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}};

vector<vector<uint8_t>> bp_7 = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};

vector<vector<uint8_t>> bp_6 = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, 
{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, 
{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0}, 
{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}};

vector<vector<uint8_t>> bp_5 = {
{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0}, 
{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0}, 
{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0}, 
{0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1}, 
{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0}, 
{0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0}, 
{1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0}, 
{0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1}, 
{1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1}};

vector<vector<uint8_t>> bp_4 = {
{0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1}, 
{0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, 
{0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1}, 
{0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0}, 
{0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1}, 
{0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1}, 
{0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, 
{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}, 
{0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0}, 
{0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1}, 
{0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1}, 
{0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0}, 
{0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1}, 
{0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1}, 
{0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0}};

vector<vector<uint8_t>> bp_3 = {
{1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0}, 
{0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0}, 
{1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1}, 
{1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0}, 
{0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0}, 
{1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, 
{1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0}, 
{0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0}, 
{1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0}, 
{1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0}, 
{1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1}, 
{1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0}, 
{1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1}, 
{0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1}, 
{0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1}};

vector<vector<uint8_t>> bp_2 = {
{1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0}, 
{0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1}, 
{1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0}, 
{0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, 
{0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0}, 
{1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0}, 
{1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0}, 
{1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0}, 
{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1}, 
{0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0}, 
{1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1}, 
{1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0}, 
{0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1}, 
{1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0}, 
{1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0}, 
{0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1}};

vector<vector<uint8_t>> bp_1 = {
{0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0}, 
{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1}, 
{1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0}, 
{1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1}, 
{1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1}, 
{1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0}, 
{1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1}, 
{0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1}, 
{1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0}, 
{0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1}, 
{0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1}, 
{0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1}, 
{1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0}, 
{0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0}, 
{1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1}, 
{0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0}};

vector<vector<uint8_t>> bp_0 = {
{0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}, 
{1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1}, 
{0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1}, 
{1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1}, 
{1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0}, 
{1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1}, 
{1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0}, 
{1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1}, 
{1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1}, 
{1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1}, 
{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0}, 
{1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1}, 
{1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0}, 
{1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1}, 
{0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0}, 
{0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0}};


code_block make_code_block(){
    code_block test;
    test.anchor = {0,0};

    subband_info s_b;
    s_b.type = 1;
    s_b.step_size = 0.000467694;
    s_b.bit_depth = 8;
    test.s_b = s_b;
    test.num_bits = 8;
    vector<vector<int8_t>> sign_data(16, vector<int8_t>(16, 0));
    for(int i=0; i<16; i++){
        for(int j=0; j<16; j++){
            if(s_d[i][j] == 0) sign_data[i][j] = -1;
            else sign_data[i][j] = 1;
        }
    }
    test.sign_data = sign_data;

    bit_plane bit_plane_7;
    bit_plane_7.bit_level = 7;
    bit_plane_7.plane_data = bp_7;

    test.bit_planes.push_back(bit_plane_7);

    bit_plane bit_plane_6;
    bit_plane_6.bit_level = 6;
    bit_plane_6.plane_data = bp_6;

    test.bit_planes.push_back(bit_plane_6);

    bit_plane bit_plane_5;
    bit_plane_5.bit_level = 5;
    bit_plane_5.plane_data = bp_5;

    test.bit_planes.push_back(bit_plane_5);

    bit_plane bit_plane_4;
    bit_plane_4.bit_level = 4;
    bit_plane_4.plane_data = bp_4;

    test.bit_planes.push_back(bit_plane_4);

    bit_plane bit_plane_3;
    bit_plane_3.bit_level = 3;
    bit_plane_3.plane_data = bp_3;

    test.bit_planes.push_back(bit_plane_3);

    bit_plane bit_plane_2;
    bit_plane_2.bit_level = 2;
    bit_plane_2.plane_data = bp_2;

    test.bit_planes.push_back(bit_plane_2);

    bit_plane bit_plane_1;
    bit_plane_1.bit_level = 1;
    bit_plane_1.plane_data = bp_1;

    test.bit_planes.push_back(bit_plane_1);

    bit_plane bit_plane_0;
    bit_plane_0.bit_level = 0;
    bit_plane_0.plane_data = bp_0;

    test.bit_planes.push_back(bit_plane_0);

    return test; 
}

vector<vector<uint8_t>> get_bit_plane(vector<bit_plane> planes, int p){
    vector<vector<uint8_t>> out_plane;
    for(auto &bp : planes){
        if(bp.bit_level == p){
            out_plane = bp.plane_data;
            break;
        }
    }
    return out_plane;
}

void compare_2D_vectors(vector<vector<uint8_t>> &vec1, vector<vector<uint8_t>> &vec2){
    size_t rows1 = vec1.size();
    size_t rows2 = vec2.size();
    if(rows1 == rows2) cout << "Rows Match! \n";
    else return;
    size_t cols1 = vec1[0].size();
    size_t cols2 = vec2[0].size();
    if(cols1 == cols2) cout << "Columns Match!\n";
    else return;

    bool same = true;
    for(size_t i=0; i<rows1; i++){
        for(size_t j=0; j<cols1; j++){
            if(vec1[i][j] != vec2[i][j]){
                cout << "Vectors do not Match!\n";
                same = false;
                break;
            }
        }
        if(!same) break;
    }
    if(same) cout << "Vectors Match!\n";
}  

void compare_2D_vectors(vector<vector<int8_t>> &vec1, vector<vector<int8_t>> &vec2){
    size_t rows1 = vec1.size();
    size_t rows2 = vec2.size();
    if(rows1 == rows2) cout << "Rows Match! \n";
    else return;
    size_t cols1 = vec1[0].size();
    size_t cols2 = vec2[0].size();
    if(cols1 == cols2) cout << "Columns Match!\n";
    else return;

    bool same = true;
    for(size_t i=0; i<rows1; i++){
        for(size_t j=0; j<cols1; j++){
            if(vec1[i][j] != vec2[i][j]){
                cout << "Vectors do not Match!\n";
                same = false;
                break;
            }
        }
        if(!same) break;
    }
    if(same) cout << "Vectors Match!\n";
}  

void compare_sign_data(vector<vector<int8_t>> &vec1, vector<vector<int8_t>> &vec2){
    size_t rows1 = vec1.size();
    size_t rows2 = vec2.size();
    if(rows1 == rows2) cout << "Rows Match! \n";
    else return;
    size_t cols1 = vec1[0].size();
    size_t cols2 = vec2[0].size();
    if(cols1 == cols2) cout << "Columns Match!\n";
    else return;

    bool same = true;
    for(size_t i=0; i<rows1; i++){
        for(size_t j=0; j<cols1; j++){
            if(vec1[i][j] != vec2[i][j]){
                if(vec1[i][j] == 0 || vec2[i][j] == 0){
                    cout << "Uncoded sign data at location: (";
                    cout << j << ", " << i << ")\n";
                }
                else{
                    cout << "Vectors do not Match!\n";
                    same = false;
                }
                break;
            }
        }
        if(!same) break;
    }
    if(same) cout << "Vectors Match!\n";
}  


void compare_code_blocks(code_block &cb1, code_block &cb2){
    vector<vector<int8_t>> test1;
    vector<vector<int8_t>> test2;

    test1 = cb1.sign_data;
    test2 = cb2.sign_data;
    cout << "Testing Sign Data: \n";
    compare_sign_data(test1, test2);

    cout << "Testing Bit Planes: \n";
    vector<vector<uint8_t>> testu1;
    vector<vector<uint8_t>> testu2;
    for(int i=cb1.num_bits - 1; i>=0; i--){
        testu1.clear();
        testu1 = get_bit_plane(cb1.bit_planes, i);
        testu2.clear();
        testu2 = get_bit_plane(cb2.bit_planes, i);

        cout << "Bit Level: " << i << "\n";
        compare_2D_vectors(testu1, testu2);
    }
}

int main() {
    code_block test_block = make_code_block();

    BitPlaneEncoder bpe(test_block);
    bpe.encode_code_block();
    deque<vector<uint8_t>> cw = bpe.get_code_stream();
    
    deque<int> length_vec = bpe.get_lengths();
    bpe.print_sign_state();

    cout << dec << endl;
    int height = bpe.get_rows();
    int width = bpe.get_cols();
    
    
    int bits = test_block.num_bits;
    subband_info s_b = test_block.s_b; 
    point anchor = test_block.anchor;
    deque<int> L_z = bpe.get_lengths();

    BitPlaneDecoder bpd(cw, height, width, bits, s_b, anchor, L_z);

    bpd.recover_codeblock();

    code_block recon_cb = bpd.get_code_block();
    bpd.print_sign_state();

    compare_code_blocks(test_block, recon_cb);
    
    
    return 0;
}
