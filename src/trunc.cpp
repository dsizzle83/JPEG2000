#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>

#include "../include/my_struct.h"
#include "../include/bp_coder_rd.h"
#include "../include/code_block.h"

using namespace std;

class Trunc{
private:
    vector<coded_block> encoded_data;
    vector<pair<int, double>> trunc_point_data;
    vector<quality_layer> layers;
public:
    Trunc(){
    }

    int solve_delta_l(coded_block &ecb, int start, int finish){
        int l_diff = 0;
        for(int i=finish; i>start; i--){
            l_diff += ecb.rate_dis[i].first;
        }
        return l_diff;
    }
    
    vector<int> compute_trunc_points(coded_block &ecb){
        // For each code block, do the convex hull analysis using the distortion and length information
        vector<int> trunc_points;
        // Always include the first pass
        trunc_points.push_back(0);
        int coding_passes = static_cast<int>(ecb.rate_dis.size());
        double lambda_zero = INFINITY;
        double lambda_last = lambda_zero;
        int h_last = 0;
        for(int i=1; i<coding_passes; i++){
            double delta_dis = ecb.rate_dis[h_last].second - ecb.rate_dis[i].second;
            int delta_len = solve_delta_l(ecb, h_last, i);

            if(delta_dis > 0){
                while(delta_dis >= lambda_last * static_cast<double>(delta_len)){
                    trunc_points.pop_back();
                    h_last = trunc_points.back();
                    delta_dis = ecb.rate_dis[h_last].second - ecb.rate_dis[i].second;
                    delta_len = solve_delta_l(ecb, h_last, i);
                }
                h_last = i + 1;
                trunc_points.push_back(h_last);
                lambda_last = delta_dis / (static_cast<double>(delta_len));
            }
        }
        return trunc_points;
    }

    vector<pair<int, double>> get_data_from_trunc_points(vector<int> trunc_points, coded_block &ecb){
        vector<pair<int, double>> trunc_data;
        pair<int, double> first_pass = ecb.rate_dis[0]; // Always include the first pass
        trunc_data.push_back(first_pass);
        for(int i=1; i<static_cast<int>(trunc_points.size()); i++){
            if(trunc_points[i] != 0){
                double dist = ecb.rate_dis[trunc_points[i]].second; // Use the total distortion
                int total_length = 0;
                for(int j=0; j < trunc_points[i]; j++){
                    total_length += ecb.rate_dis[j].first;
                }
                trunc_data.push_back(make_pair(total_length, dist));
            }
        }
    }

    void add_coded_block(coded_block &ecb){
        encoded_data.push_back(ecb);
        
    }
};

int main(){
    return 0;
}