#include "../include/dwt.h"

using namespace std;

// Start of Forward Discrete Wavelet Transform Class Member Functions
FDWT::FDWT(vector<double> &img, point o, int lvls, int height, int width){
    levels = lvls;
    rows = height;
    cols = width;
    image = img;
    origin = o;
    extent.x = cols + origin.x;
    extent.y = rows + origin.y;
    transformed = multi_level_dwt(image, levels);
}

vector<int> FDWT::get_transformed_int(){
    vector<int> transformed_int;
    transformed_int = double_to_int(transformed);
    return transformed_int;
}

vector<double> FDWT::get_transformed(){ return transformed;}

vector<double> FDWT::multi_level_dwt(vector<double> &img, int lvls){
    int m = rows;
    int n = cols;
    vector<double> ll;
    vector<double> a(rows*cols, 0);

    ll = img;
    for(int i=0; i < lvls; i++){
        vector<double> a_b;
        // cout << "LL: \n";
        // print_2D_vector(ll, m, n);

        a_b = sd2d(ll, m, n);
        // cout << "a_b: \n";
        // print_2D_vector(a_b, m, n);

        // Copy a_b into a
        for(int i=0; i<m; i++){
            for(int j=0; j<n; j++){
                a[i*cols + j] = a_b[i*n + j];
            }
        }
        // cout << "A:\n";
        // print_2D_vector(a, rows, cols);

        // Extract ll from a_b
        int new_m = (m+1)/2;
        int new_n = (n+1)/2;
        ll.clear();
        ll.resize(0);
        for(int i = 0; i < new_m; i++){
            for(int j=0; j < new_n; j++){
                ll.push_back(a_b[i*n + j]);
            }
        }
        m = new_m;
        n = new_n;
    }
    return a;
}

vector<double> FDWT::int_to_double(vector<int> &int_vec){
    vector<double> double_vec;
    for(int value : int_vec){
        double_vec.push_back(static_cast<double>(value));
    }
    return double_vec;
}

vector<int> FDWT::double_to_int(vector<double> &double_vec){
    vector<int> int_vec(double_vec.size(), 0);
    for(const double &val : double_vec){
        int rounded = round(val);
        int_vec.push_back(rounded);
    }
    return int_vec;
}

vector<double> FDWT::sd2d(vector<double> &a, int height, int width){
    a = ver_sd(a, height, width);
    a = hor_sd(a, height, width);
    a = deint2(a, height, width);
    return a;
}

vector<double> FDWT::ver_sd(vector<double> &a, int height, int width){
    size_t u = 0;
    int i_0 = origin.y;
    while(u < width){
        vector<double> x = hor_to_vert(a, u, height, width);
        vector<double> y = sd1d(x, i_0);
        replace_column(a, y, u, height, width);
        u++;
    }
    return a;
}

vector<double> FDWT::hor_to_vert(vector<double> &a, int u, int height, int width){
    vector<double> out_vec;
    if(u >= (width)){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(int i=0; i<height; i++){
        out_vec.push_back(a[i*width + u]);
    }
    return out_vec;
}

vector<double> FDWT::replace_column(vector<double> &a, vector<double> y, int u, int height, int width){
    if(u >= width){
        cout << "Column: " << u << " out of bounds for vector length: " << a.size() << endl;
        return {{}};
    }
    for(size_t i=0; i<height; i++){
        // cout << "y[" << i << "]:" << y[i] << ", a[" << i << "][" << u << "]: " << a[i][u] << endl;
        a[i*width + u] = y[i];
    }
    // cout << a.size() << endl;
    return a;
}

vector<double> FDWT::hor_sd(vector<double> &a, int height, int width){
    int v = 0;
    int i_0 = origin.x;
    while(v < height){
        vector<double> x;
        for(int u=0; u<width; u++){
            x.push_back(a[v*width + u]);
        }
        vector<double> y = sd1d(x, i_0);
        for(int u=0; u<width; u++){
            a[v*width + u] = y[u];
        }
        v++;
    }
    return a;
}

vector<double> FDWT::deint2(vector<double> &a, int height, int width){
    vector<double> combined(a.size(), 0);

    // Determine the dimensions of LL subband
    // Add 1 so that the larger portion is LL
    int ll_height, ll_width;
    ll_height = (height + 1) / 2;
    ll_width = (width + 1) / 2;

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            int c_x, c_y; // destination indices
            if(i%2 == 0){
                // LL, HL row
                if(j%2 == 0){
                    // LL column
                    c_x = j/2;
                    c_y = i/2;
                    combined[c_y * width + c_x] = a[i*width + j];
                }
                else{
                    // HL column
                    c_x = j/2 + ll_width;
                    c_y = i/2;
                    combined[c_y * width + c_x] = a[i*width + j];
                }
            }
            else{
                // LH, HH row
                if(j%2 == 0){
                    // LH column
                    c_x = j/2;
                    c_y = i/2 + ll_height;
                    combined[c_y * width + c_x] = a[i*width + j];
                }
                else{
                    // HH column
                    c_x = j/2 + ll_width;
                    c_y = i/2 + ll_height;
                    combined[c_y * width + c_x] = a[i*width + j];
                }
            }
        }
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

void FDWT::print_2D_vector(vector<double> &a, int height, int width){
    cout << fixed << setprecision(2);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            cout << (int)a[i*width + j] << "\t";
        }
        cout << endl;
    }
}

void FDWT::print_2D_vector(vector<int> &a, int height, int width){
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            cout << a[i*width + j] << "\t";
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
IDWT::IDWT(vector<double> &t, point o, int lvl, int height, int width){
    level = lvl;
    origin = o;
    transformed = t;
    cout << "Transformed in the constructuor\n";
    print_2D_vector(t, height, width);
    rows = height;
    cols = width;
    extent.x = cols + origin.x;
    extent.y = rows + origin.y;
    image = multilevel_idwt(transformed, level);
}

vector<double> IDWT::get_image(){
    return image;
}

vector<double> IDWT::multilevel_idwt(vector<double> &t, int lvl){
    
    cout << "Rows: " << rows << ", Cols: " << cols << endl;
    cout << "T: \n";
    print_2D_vector(t, rows, cols);

    for(int i=lvl; i > 0; i--){
        point subband_boundary;
        subband_boundary.x = min(cols, (cols + 1) / (1 << (i-1)));
        subband_boundary.y = min(rows, (rows + 1) / (1 << (i-1)));

        cout << "\nSubband Boundary: (" << subband_boundary.x << ", " << subband_boundary.y << ")\n";

        vector<double> subband(subband_boundary.y * subband_boundary.x, 0);
        for(int i=0; i<subband_boundary.y; i++){
            for(int j=0; j<subband_boundary.x; j++){
                subband[i*subband_boundary.x + j] = t[i*cols + j];
            }
        }
        cout << "Subband " << lvl << ":\n";
        print_2D_vector(subband, subband_boundary.y, subband_boundary.x);

        subband = sr2d(subband, subband_boundary.y, subband_boundary.x);
        for(int i=0; i<subband_boundary.y; i++){
            for(int j=0; j<subband_boundary.x; j++){
                t[i*cols + j] = subband[i*subband_boundary.x + j];
            }
        }
    }
    return t;
}

vector<double> IDWT::sr2d(vector<double> &a, int height, int width){
    a = int2d(a, height, width);
    a = hor_sr(a, height, width);
    a = ver_sr(a, height, width);
    return a;
}

vector<double> IDWT::int_to_double(vector<int> &int_vec){
    vector<double> double_vec;
    
    for(int &value : int_vec){
        double_vec.push_back(static_cast<double>(value));
    }

    return double_vec;
}

vector<int> IDWT::double_to_int(vector<double> &double_vec){
    vector<int> int_vec;
    
    for(double &value : double_vec){
        int rounded = round(value);
        int_vec.push_back(rounded);
    }
    
    return int_vec;
}

vector<double> IDWT::int2d(vector<double> &a, int height, int width){
    int m = height;
    int n = width;
    vector<double> out_vector(m*n, 0);

    point boundary;  // Relative boundary
    boundary.x = (n + 1) / 2;
    boundary.y = (m + 1) / 2;
    
    // LL
    // cout << "LL: " << endl;
    for(int i=0; i < boundary.y; i++){
        for(int j=0; j < boundary.x; j++){
            out_vector[2*i*width + 2*j] = a[i*width + j];
        }
    }
    //HL
    for (int i=0; i<boundary.y; i++){
        for(int j=boundary.x; j<n; j++){
            out_vector[2*i*width + 2*(j-boundary.x) + 1] = a[i*width + j];
        }
    }
    //LH
    for(int i=boundary.y; i<m; i++){
        for(int j=0; j<boundary.x; j++){
            out_vector[(2*(i-boundary.y) + 1) * width + 2*j] = a[i*width + j];
        }
    }
    //HH
    for(int i=boundary.y; i<m; i++){
        for(int j=boundary.x; j<n; j++){
            out_vector[(2*(i-boundary.y) + 1)*width + 2*(j-boundary.x) + 1] = a[i*width + j];
        }
    }
    return out_vector;
}
vector<double> IDWT::hor_sr(vector<double> &a, int height, int width){
    size_t v = 0;
    int i_0 = origin.x;
    while(v < height){
        vector<double> y;
        for(int i=0; i<width; i++){
            y.push_back(a[v*width + i]);
        }
        vector<double> x = sr1d(y, i_0);
        for(int i=0; i<width; i++){
            a[v*width + i] = x[i];
        }
        v++;
    }
    return a;
}
vector<double> IDWT::ver_sr(vector<double> &a, int height, int width){
    size_t u = 0;
    int i_0 = origin.y;
    while(u < width){
        vector<double> x = hor_to_vert(a, u, height, width);
        vector<double> y = sr1d(x, i_0);
        a = replace_column(a, y, u, height, width);
        u++;
    }
    return a;
}

vector<double> IDWT::hor_to_vert(vector<double> &a, int u, int height, int width){
    vector<double> out_vec;
    if(u >= width){
        cout << "Column: " << u << " out of bounds for vector length: " << width << endl;
        return {{}};
    }
    for(int i=0; i<height; i++){
        out_vec.push_back(a[i*width + u]);
    }
    return out_vec;
}

vector<double> IDWT::replace_column(vector<double> &a, vector<double> &y, int u, int height, int width){
    if(u >= width){
        cout << "Column: " << u << " out of bounds for vector length: " << width << endl;
        return {{}};
    }
    for(size_t i=0; i<y.size(); i++){
        // cout << "y[" << i << "]:" << y[i] << ", a[" << i << "][" << u << "]: " << a[i][u] << endl;
        a[i*width + u] = y[i];
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

void IDWT::print_2D_vector(vector<double> &a, int height, int width){
    cout << fixed << setprecision(2);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            int val = static_cast<int>(round(a[i*width + j]));
            cout << val << "\t";
        }
        cout << endl;
    }
}

void IDWT::print_2D_vector(vector<int> &a, int height, int width){
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            cout << a[i*width + j] << "\t";
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
