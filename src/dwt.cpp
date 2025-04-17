#include "../include/dwt.h"

using namespace std;

// Start of Forward Discrete Wavelet Transform Class Member Functions
FDWT::FDWT(vector<vector<double>> &img, point o, int lvls){
    levels = lvls;
    rows = img.size();
    cols = img[0].size();
    image = img;
    origin = o;
    extent.x = cols + origin.x;
    extent.y = rows + origin.y;
    transformed = multi_level_dwt(image, levels);
}

vector<vector<int>> FDWT::get_transformed_int(){
    vector<vector<int>> transformed_int;
    transformed_int = double_to_int(transformed);
    return transformed_int;
}

vector<vector<double>> FDWT::get_transformed(){ return transformed;}

vector<vector<double>> FDWT::multi_level_dwt(vector<vector<double>> &img, int lvls){
    int m = rows;
    int n = cols;
    vector<vector<double>> ll;
    vector<vector<double>> a(m, vector<double>(n, 0));

    ll = img;
    for(int i=0; i < lvls; i++){
        vector<vector<double>> a_b;
        a_b = sd2d(ll);
        size_t width = ll[0].size();
        size_t height = ll.size();
        // Copy a_b into a
        for(size_t i=0; i<a_b.size(); i++){
            for(size_t j=0; j<a_b[i].size(); j++){
                a[i][j] = a_b[i][j];
            }
        }
        // Extract ll from a_b
        ll.clear();
        for(size_t i = 0; i < ((height + 1) / 2); i++){
            vector<double> temp_row;
            for(size_t j=0; j < ((width + 1) / 2); j++){
                temp_row.push_back(a_b[i][j]);
            }
            ll.push_back(temp_row);
        }
    }
    return a;
}

vector<vector<double>> FDWT::int_to_double(vector<vector<int>> &int_vec){
    vector<vector<double>> double_vec;
    for(const auto&row : int_vec){
        vector<double> double_row;
        for(int value : row){
            double_row.push_back(static_cast<double>(value));
        }
        double_vec.push_back(double_row);
    }
    return double_vec;
}

vector<vector<int>> FDWT::double_to_int(vector<vector<double>> &double_vec){
    vector<vector<int>> int_vec;
    for(const auto&row : double_vec){
        vector<int> int_row;
        for(double value : row){
            int rounded = round(value);
            int_row.push_back(rounded);
        }
        int_vec.push_back(int_row);
    }
    return int_vec;
}

vector<vector<double>> FDWT::sd2d(vector<vector<double>> &a){
    a = ver_sd(a);
    a = hor_sd(a);
    a = deint2(a);
    return a;
}

vector<vector<double>> FDWT::ver_sd(vector<vector<double>> &a){
    size_t u = 0;
    int i_0 = origin.y;
    while(u < a[0].size()){
        vector<double> x = hor_to_vert(a, u);
        vector<double> y = sd1d(x, i_0);
        a = replace_column(a, y, u);
        u++;
    }
    return a;
}

vector<double> FDWT::hor_to_vert(vector<vector<double>> &a, int u){
    vector<double> out_vec;
    if(u >= static_cast<int>(a[0].size())){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(const auto&row : a){
        out_vec.push_back(row[u]);
    }
    return out_vec;
}

vector<vector<double>> FDWT::replace_column(vector<vector<double>> &a, vector<double> y, int u){
    if(u >= static_cast<int>(a[0].size())){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(size_t i=0; i<y.size(); i++){
        // cout << "y[" << i << "]:" << y[i] << ", a[" << i << "][" << u << "]: " << a[i][u] << endl;
        a[i][u] = y[i];
    }
    // cout << a.size() << endl;
    return a;
}

vector<vector<double>> FDWT::hor_sd(vector<vector<double>> &a){
    size_t v = 0;
    int i_0 = origin.x;
    while(v < a.size()){
        vector<double> x = a[v];
        vector<double> y = sd1d(x, i_0);
        a[v] = y;
        v++;
    }
    return a;
}

vector<vector<double>> FDWT::deint2(vector<vector<double>> &a){
    vector<vector<double>> ll;
    vector<vector<double>> lh;
    vector<vector<double>> hl;
    vector<vector<double>> hh;

    vector<vector<double>> combined;
    combined.reserve(a.size());

    for(size_t i=0; i<a.size(); i++){
        if((i % 2) == 0){
            vector<double> temp_ll_row;
            vector<double> temp_hl_row;
            for(size_t j=0; j<a[i].size(); j++){
                if((j % 2) == 0){
                    temp_ll_row.push_back(a[i][j]);
                }
                else{
                    temp_hl_row.push_back(a[i][j]);
                }
            }
            ll.push_back(temp_ll_row);
            hl.push_back(temp_hl_row);
        }
        else{
            vector<double> temp_lh_row;
            vector<double> temp_hh_row;
            for(size_t j=0; j<a[i].size(); j++){
                if((j % 2) == 0){
                    temp_lh_row.push_back(a[i][j]);
                }
                else{
                    temp_hh_row.push_back(a[i][j]);
                }
            }
            lh.push_back(temp_lh_row);
            hh.push_back(temp_hh_row);
        }            
    }
    for(size_t i=0; i<ll.size(); i++){
        vector<double> temp_row;
        temp_row.insert(temp_row.end(), ll[i].begin(), ll[i].end());
        temp_row.insert(temp_row.end(), hl[i].begin(), hl[i].end());
        combined.push_back(temp_row);
    }
    for(size_t i=0; i<lh.size(); i++){
        vector<double> temp_row;
        temp_row.insert(temp_row.end(), lh[i].begin(), lh[i].end());
        temp_row.insert(temp_row.end(), hh[i].begin(), hh[i].end());
        combined.push_back(temp_row);
    }
    return combined;
}

vector<double> FDWT::sd1d(vector<double> &x, int i_0){
    // Use absolute indexes to determine extension
    int i_1 = x.size() + i_0;
    int extend_left = 4, extend_right = 4;

    vector<double> x_ext = ext1d(x, extend_left, extend_right);

    // Use relative indexes for the below equations
    i_1 = (i_1 - i_0) + extend_left;
    i_0 = extend_left;
    

    vector<double> y_ext = filt1d(x_ext, i_0, i_1);
    vector<double> y = trim_vector(y_ext, extend_left, extend_right);
    return y;
}

vector<double> FDWT::ext1d(vector<double> &x, int extend_left, int extend_right){
    vector<double> x_ext;
    for(int i=extend_left; i > 0; i--){
        x_ext.push_back(x[i]);
    }
    x_ext.insert(x_ext.end(), x.begin(), x.end());
    for(int i=0; i < extend_right; i++){
        x_ext.push_back(x[x.size() - 2 - i]);
    }
    return x_ext;
}

vector<double> FDWT::filt1d(vector<double> &x, int i_0, int i_1){
    vector<double> y(x.size(), 0);

    // Step 1
    for(int n = ((i_0+1)/2 - 2); n < ((i_1+1)/2 + 1); n++){
        y[2*n + 1] = x[2*n + 1] + A * (x[2*n] + x[2*n + 2]);
    }
    
    // Step 2
    for(int n=((i_0+1)/2 - 1); n < ((i_1+1)/2 + 1); n++){
        y[2*n] = x[2*n] + B * (y[2*n - 1] + y[2*n + 1]);
    }
    
    // Step 3
    for(int n=((i_0+1)/2 - 1); n<((i_1+1)/2); n++){
        y[2*n + 1] = y[2*n + 1] + G * (y[2*n] + y[2*n + 2]);
    }
    
    // Step 4
    for(int n=((i_0+1)/2); n<((i_1+1)/2); n++){
        y[2*n] = y[2*n] + D * (y[2*n - 1] + y[2*n + 1]);
    }
    
    // Step 5
    for(int n=((i_0+1)/2); n<((i_1+1)/2); n++){
        y[2*n +1] = K * y[2*n + 1];
    }

    // Step 6
    for(int n=((i_0+1)/2); n<((i_1+1)/2); n++){
        y[2*n] = (1.0/K) * y[2*n];
    }
    return y;
}

vector<double> FDWT::trim_vector(vector<double> &y, int ext_left, int ext_right){
    deque<double> y_deq;
    copy(y.begin(), y.end(), back_inserter(y_deq));
    for(int i=0; i < ext_left; i++){
        y_deq.pop_front();
    }
    for(int i=0; i < ext_right; i++){
        y_deq.pop_back();
    }
    vector<double> out_y;
    copy(y_deq.begin(), y_deq.end(), back_inserter(out_y));

    return out_y;
}

void FDWT::print_2D_vector(vector<vector<double>> &a){
    cout << fixed << setprecision(2);
    for(const auto&row : a){
        for(double val : row){
            cout << val << "\t";
        }
        cout << endl;
    }
}

void FDWT::print_2D_vector(vector<vector<int>> &a){
    for(const auto&row : a){
        for(int val : row){
            cout << val << "\t";
        }
        cout << endl;
    }
}

void FDWT::print_vector(vector<double> &a){
    for(const auto&val : a){
        cout << val << ", ";
    }
    cout << endl;
}

void FDWT::print_vector(vector<int> &a){
    for(const auto&val : a){
        cout << val << ", ";
    }
    cout << endl;
}

// Start of Inverse Discrete Wavelet Transform Class Member Functions
IDWT::IDWT(vector<vector<double>> &t, point o, int lvl){
    level = lvl;
    origin = o;
    transformed = t;
    rows = transformed.size();
    cols = transformed[0].size();
    extent.x = cols + origin.x;
    extent.y = rows + origin.y;
    image = multilevel_idwt(transformed, level);
}

vector<vector<double>> IDWT::get_image(){
    return image;
}

vector<vector<double>> IDWT::multilevel_idwt(vector<vector<double>> &t, int lvl){
    
    for(int i=lvl; i > 0; i--){
        point subband_boundary;
        subband_boundary.x = min(cols, (cols + 1) / (1 << (i-1)));
        subband_boundary.y = min(rows, (rows + 1) / (1 << (i-1)));

        vector<vector<double>> subband(subband_boundary.y, vector<double>(subband_boundary.x, 0));
        for(int i=0; i<subband_boundary.y; i++){
            for(int j=0; j<subband_boundary.x; j++){
                subband[i][j] = t[i][j];
            }
        }
        subband = sr2d(subband);
        for(int i=0; i<subband_boundary.y; i++){
            for(int j=0; j<subband_boundary.x; j++){
                t[i][j] = subband[i][j];
            }
        }
    }
    return t;
}

vector<vector<double>> IDWT::sr2d(vector<vector<double>> &a){
    a = int2d(a);
    a = hor_sr(a);
    a = ver_sr(a);
    return a;
}

vector<vector<double>> IDWT::int_to_double(vector<vector<int>> &int_vec){
    vector<vector<double>> double_vec;
    for(const auto&row : int_vec){
        vector<double> double_row;
        for(int value : row){
            double_row.push_back(static_cast<double>(value));
        }
        double_vec.push_back(double_row);
    }
    return double_vec;
}

vector<vector<int>> IDWT::double_to_int(vector<vector<double>> &double_vec){
    vector<vector<int>> int_vec;
    for(const auto&row : double_vec){
        vector<int> int_row;
        for(double value : row){
            int rounded = round(value);
            int_row.push_back(rounded);
        }
        int_vec.push_back(int_row);
    }
    return int_vec;
}

vector<vector<double>> IDWT::int2d(vector<vector<double>> &a){
    int m = a.size();
    int n = a[0].size();
    vector<vector<double>> out_vector(m, vector<double>(n, 0));

    point boundary;  // Relative boundary
    boundary.x = (n + 1) / 2;
    boundary.y = (m + 1) / 2;
    
    // LL
    // cout << "LL: " << endl;
    for(int i=0; i < boundary.y; i++){
        for(int j=0; j < boundary.x; j++){
            out_vector[2*i][2*j] = a[i][j];
        }
    }
    //HL
    for (int i=0; i<boundary.y; i++){
        for(int j=boundary.x; j<n; j++){
            out_vector[2*i][2*(j-boundary.x) + 1] = a[i][j];
        }
    }
    //LH
    for(int i=boundary.y; i<m; i++){
        for(int j=0; j<boundary.x; j++){
            out_vector[2*(i-boundary.y) + 1][2*j] = a[i][j];
        }
    }
    //HH
    for(int i=boundary.y; i<m; i++){
        for(int j=boundary.x; j<n; j++){
            out_vector[2*(i-boundary.y) + 1][2*(j-boundary.x) + 1] = a[i][j];
        }
    }
    return out_vector;
}
vector<vector<double>> IDWT::hor_sr(vector<vector<double>> &a){
    size_t v = 0;
    int i_0 = origin.x;
    while(v < a.size()){
        vector<double> y = a[v];
        vector<double> x = sr1d(y, i_0);
        a[v] = x;
        v++;
    }
    return a;
}
vector<vector<double>> IDWT::ver_sr(vector<vector<double>> &a){
    size_t u = 0;
    int i_0 = origin.y;
    while(u < a[0].size()){
        vector<double> x = hor_to_vert(a, u);
        vector<double> y = sr1d(x, i_0);
        a = replace_column(a, y, u);
        u++;
    }
    return a;
}

vector<double> IDWT::hor_to_vert(vector<vector<double>> &a, int u){
    vector<double> out_vec;
    if(u >= static_cast<int>(a[0].size())){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(const auto&row : a){
        out_vec.push_back(row[u]);
    }
    return out_vec;
}

vector<vector<double>> IDWT::replace_column(vector<vector<double>> &a, vector<double> &y, int u){
    if(u >= static_cast<int>(a[0].size())){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(size_t i=0; i<y.size(); i++){
        // cout << "y[" << i << "]:" << y[i] << ", a[" << i << "][" << u << "]: " << a[i][u] << endl;
        a[i][u] = y[i];
    }
    // cout << a.size() << endl;
    return a;
}

vector<double> IDWT::sr1d(vector<double> &y, int i_0){
    // Use absolute indexes to determine extension
    int i_1 = y.size() + i_0;
    int extend_left = 4, extend_right = 4;
    vector<double> y_ext = ext1d(y, extend_left, extend_right);

    // Use relative indexes for the below equations
    i_1 = (i_1 - i_0) + extend_left;
    i_0 = extend_left;
    vector<double> x_ext = filt1d(y_ext, i_0, i_1);
    vector<double> x = trim_vector(x_ext, extend_left, extend_right);

    return x;
}

vector<double> IDWT::ext1d(vector<double> &x, int extend_left, int extend_right){
    vector<double> x_ext;
    for(int i=extend_left; i > 0; i--){
        x_ext.push_back(x[i]);
    }
    x_ext.insert(x_ext.end(), x.begin(), x.end());
    for(int i=0; i < extend_right; i++){
        x_ext.push_back(x[x.size() - 2 - i]);
    }
    return x_ext;
}

vector<double> IDWT::filt1d(vector<double> &y, int i_0, int i_1){
    vector<double> x(y.size(), 0);
    
    // Step 1
    for(int n=((i_0 / 2) - 1); n < ((i_1 / 2) + 2); n++){
        x[2*n] = K * y[2*n];
    }


    // Step 2
    for(int n=((i_0 / 2) - 2); n < ((i_1 / 2) + 2); n++){
        x[2*n + 1] = (1.0/K) * y[2*n + 1];
    }


    // Step 3
    for(int n=((i_0 / 2) - 1); n < ((i_1 / 2) + 2); n++){
        x[2*n] = x[2*n] - D * (x[2*n - 1] + x[2*n + 1]);
    }

    // Step 4
    for(int n=((i_0 / 2) - 1); n < ((i_1 / 2) + 1); n++){
        x[2*n + 1] = x[2*n + 1] - G * (x[2*n] + x[2*n + 2]);
    }

    // Step 5
    for(int n=(i_0 / 2); n < ((i_1 / 2) + 1); n++){
        x[2*n] = x[2*n] - B * (x[2*n - 1] + x[2*n + 1]);
    }

    // Step 6
    for(int n=(i_0 / 2); n < (i_1 / 2); n++){
        x[2*n + 1] = x[2*n + 1] - A * (x[2*n] + x[2*n + 2]);
    }

    return x;
}

vector<double> IDWT::trim_vector(vector<double> &y, int ext_left, int ext_right){
    deque<double> y_deq;
    copy(y.begin(), y.end(), back_inserter(y_deq));
    for(int i=0; i < ext_left; i++){
        y_deq.pop_front();
    }
    for(int i=0; i < ext_right; i++){
        y_deq.pop_back();
    }
    vector<double> out_y;
    copy(y_deq.begin(), y_deq.end(), back_inserter(out_y));

    return out_y;
}

void IDWT::print_2D_vector(vector<vector<double>> &a){
    cout << fixed << setprecision(2);
    for(const auto&row : a){
        for(double val : row){
            cout << val << "\t";
        }
        cout << endl;
    }
}

void IDWT::print_2D_vector(vector<vector<int>> &a){
    for(const auto&row : a){
        for(int val : row){
            cout << val << "\t";
        }
        cout << endl;
    }
}

void IDWT::print_vector(vector<double> &a){
    for(const auto&val : a){
        cout << val << ", ";
    }
    cout << endl;
}

void IDWT::print_vector(vector<int> &a){
    for(const auto&val : a){
        cout << val << ", ";
    }
    cout << endl;
}
