#include "../include/bp_coder_rd.h"

using namespace std;

BitPlaneEncoderRD::BitPlaneEncoderRD(){

}

BitPlaneEncoderRD::BitPlaneEncoderRD(code_block &c_b) 
    : B_i(c_b)
{
    rows = c_b.height;
    cols = c_b.width;
    init_state_tables();
}

void BitPlaneEncoderRD::load_code_block(code_block &c_b){
    B_i = c_b;
    rows = c_b.height;
    cols = c_b.width;
    sort(B_i.bit_planes.begin(), B_i.bit_planes.end(), 
        [](const bit_plane &a, const bit_plane &b) { 
            return a.bit_level > b.bit_level; 
        });
}

void BitPlaneEncoderRD::reset_bp_encoder(){
    vector<uint8_t>().swap(sig_state);
    vector<uint8_t>().swap(del_sig);
    vector<uint8_t>().swap(member);
    init_state_tables();
}

void BitPlaneEncoderRD::init_state_tables(){
    sig_state = vector<uint8_t>(rows*cols, 0);
    del_sig = vector<uint8_t>(rows*cols, 0);
    member = vector<uint8_t>(rows*cols, 0);
}

void BitPlaneEncoderRD::reconstruct_wavelet_coeffs(){
    vector<double> wc(rows*cols, 0.0);
    for(int i=0; i<static_cast<int>(B_i.bit_planes.size()); i++){
        double bit_level = static_cast<double>(B_i.bit_planes[i].bit_level);
        double increment = pow(2.0, bit_level) * B_i.s_b.step_size;
        for(int y=0; y<rows; y++){
            for(int x=0; x<cols; x++){
                wc[y*cols + x] += (B_i.bit_planes[i].plane_data[y*cols + x] == 1)? increment : 0.0;
            }
        }
    }
    for(int y=0; y<rows; y++){
        for(int x=0; x<cols; x++){
            wc[y*cols + x] += B_i.s_b.step_size / 2.0; // Center the coeffecients within the LSB interval
            wc[y*cols + x] *= static_cast<double>(B_i.sign_data[y*cols + x]); // convert to signed data
        }
    }
    wavelet_coeffs = wc;
    vector<double>().swap(wc);
}

void BitPlaneEncoderRD::init_implied_coeffs(){
    vector<double> iwc(rows*cols, 0);
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            iwc[i*cols + j] = 0.0;
        }
    }
    implied_coeffs = iwc;
    vector<double>().swap(iwc);
}

void BitPlaneEncoderRD::update_implied_coeffs(point loc, int p, bool symbol){
    double increment = pow(2.0, static_cast<double>(p) - 1.0);
    bool sign_pos = (B_i.sign_data[loc.y * cols + loc.x] == 1);
    increment *= (sign_pos ^ symbol)? -1 : 1;
    implied_coeffs[loc.y*cols + loc.x] += increment;
}  

double BitPlaneEncoderRD::calc_total_distortion(){
    double dist = 0.0;
    double diff;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            diff = implied_coeffs[i*cols + j] - wavelet_coeffs[i*cols + j];
            dist += diff * diff;
        }
    }
    dist = dist * B_i.s_b.energy;
    return dist;
}

uint8_t BitPlaneEncoderRD::sig_context(point loc){
    int k_h = 0, k_v = 0, k_d = 0;
    uint8_t context;
    if(loc.x != 0){
        k_h += sig_state[loc.y*cols + loc.x - 1];
        if(loc.y != 0){
            k_v += sig_state[(loc.y - 1)*cols + loc.x];
            k_d += sig_state[(loc.y - 1)*cols + loc.x - 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[(loc.y + 1)*cols + loc.x];
            k_d += sig_state[(loc.y + 1)*cols + loc.x - 1];
        }
    }
    if(loc.x != cols - 1){
        k_h += sig_state[(loc.y)*cols + loc.x + 1];
        if(loc.y != 0){
            k_v += sig_state[(loc.y - 1)*cols + loc.x];
            k_d += sig_state[(loc.y - 1)*cols + loc.x + 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[(loc.y + 1)*cols + loc.x];
            k_d += sig_state[(loc.y + 1)*cols + loc.x + 1];
        }
    }
    if(B_i.s_b.type == 0 || B_i.s_b.type == 2){
        if(k_h == 2) context = 8;
        else if(k_h == 1){
            if(k_v >= 1) context = 7;
            else if(k_d >= 1) context = 6;
            else context = 5;
        }
        else{
            if(k_v == 2) context = 4;
            else if(k_v == 1) context = 3;
            else if(k_d >= 2) context = 2;
            else if(k_d == 1) context = 1;
            else context = 0;
        }
    }
    else if(B_i.s_b.type == 1){
        if(k_v == 2) context = 8;
        else if(k_v == 1){
            if(k_h >= 1) context = 7;
            else if(k_d >= 1) context = 6;
            else context = 5;
        }
        else{
            if(k_h == 2) context = 4;
            else if(k_h == 1) context = 3;
            else{
                if(k_d >= 2) context = 2;
                else if(k_d == 1) context = 1;
                else context = 0;
            }
        }
    }
    else{
        int k_hv = k_h + k_v;
        if(k_d >= 3) context = 8;
        else if(k_d == 2){
            if(k_hv >= 1) context = 7;
            else context = 6;
        }
        else if(k_d == 1){
            if(k_hv >= 2) context = 5;
            else if(k_hv == 1) context = 4;
            else context = 3;
        }
        else{
            if(k_hv >= 2) context = 2;
            else if(k_hv == 1) context = 1;
            else context = 0;
        }
    }
    return context;
}

pair<uint8_t, int8_t> BitPlaneEncoderRD::sign_context(point loc){
    int8_t x_h = 0, x_v = 0;
    
    if(loc.x != 0){
        x_h += sig_state[loc.y*cols + loc.x - 1] * B_i.sign_data[loc.y*cols + loc.x - 1];
    }
    if(loc.x != cols -1){
        x_h += sig_state[loc.y*cols + loc.x + 1] * B_i.sign_data[loc.y*cols + loc.x + 1];
    }
    if(loc.y != 0){
        x_v += sig_state[(loc.y - 1)*cols + loc.x] * B_i.sign_data[(loc.y - 1)*cols + loc.x];
    }
    if(loc.y != rows -1){
        x_v += sig_state[(loc.y + 1)*cols + loc.x] * B_i.sign_data[(loc.y + 1)*cols + loc.x];
    }


    int8_t flip;
    uint8_t context;
    if(x_h > 0){
        if(x_v > 0) context = 14;
        else if(x_v == 0) context = 13;
        else context = 12;
    }
    else if(x_h == 0){
        if(x_v != 0) context = 11;
        else context = 10;
    }
    else{
        if(x_v > 0) context = 12;
        else if(x_v == 0) context = 13;
        else context = 14;
    }
    
    if(x_h > 0){
        flip = 1;
    }
    else if(x_h == 0){
        if(x_v >= 0) flip = 1;
        else flip = -1;
    }
    else{
        flip = -1;
    }

    pair<uint8_t, int8_t> out_pair;
    out_pair = make_pair(context, flip);

    return out_pair;
}

uint8_t BitPlaneEncoderRD::mag_context(point loc){
    uint8_t context;
    if(del_sig[loc.y*cols + loc.x] == 1) context = 17;
    else{
        uint8_t k_sig = sig_context(loc);
        if(k_sig == 0) context = 15;
        else context = 16;
    }
    return context;
}

void BitPlaneEncoderRD::sig_prop_pass(vector<uint8_t> &stripes, point anchor, int height, int width, int pass){
    uint8_t k_sig;
    point loc;
    for(size_t j=0; j<width; j++){
        loc.x = anchor.x + j;
        for(size_t i=0; i<height; i++){
            loc.y = anchor.y + i;
            k_sig = sig_context(loc);
            if((sig_state[loc.y*cols + loc.x] == 0) && (k_sig > 0)){
                bool symbol = stripes[i*width + j] == 1;
                update_implied_coeffs(loc, pass, symbol);
                mq_encoder.encode(symbol, k_sig);
                // debug_encode(stripes[i*width + j] == 1, k_sig, loc, pass, 's');
                if(stripes[i*width + j] == 1){
                    sig_state[loc.y*cols + loc.x] = 1;
                    encode_sign(loc, pass, 's');
                }
                member[loc.y*cols + loc.x] = 1;
            }
            else member[loc.y*cols + loc.x] = 0;
        }
    }
}

void BitPlaneEncoderRD::encode_sign(point loc, int pass, char pass_type){
    pair<uint8_t, int8_t> sign_pair = sign_context(loc);
    int8_t sign = B_i.sign_data[loc.y*cols + loc.x];
    bool symbol = ((sign * sign_pair.second) == 1)? false : true;

    mq_encoder.encode(symbol, sign_pair.first);
    // debug_encode(symbol, sign_pair.first, loc, pass, pass_type);
}

void BitPlaneEncoderRD::mag_ref_pass(vector<uint8_t> &stripes, point anchor, int height, int width, int pass){
    uint8_t k_mag;
    point loc;
    for(size_t j=0; j<width; j++){
        loc.x = anchor.x + j;
        for(size_t i=0; i<height; i++){
            loc.y = anchor.y + i;
            if(sig_state[loc.y*cols + loc.x] && !member[loc.y*cols + loc.x]){
                k_mag = mag_context(loc);
                bool symbol = stripes[i*width + j] == 1;
                update_implied_coeffs(loc, pass, symbol);
                mq_encoder.encode(symbol, k_mag);
                // debug_encode((stripes[i*width + j] == 1), k_mag, loc, pass, 'm');
                del_sig[loc.y*cols + loc.x] = sig_state[loc.y*cols + loc.x];
            }
        }
    }
}

void BitPlaneEncoderRD::clean_up_pass(vector<uint8_t> &stripes, point anchor, int height, int width, int pass){
    uint8_t k_sig;
    point loc;
    int r;
    for(int j = 0; j < width; j++){
        loc.x = anchor.x + j;
        for(int i=0; i< height; i++){
            loc.y = anchor.y + i;
            if((loc.y % 4 == 0) && (loc.y <= rows - 4)){
                r = -1;
                bool all_zeros = true;
                for(int k=0; k<4; k++){
                    point run_loc = loc;
                    run_loc.y += k;
                    uint8_t k_sig_k = sig_context(run_loc);
                    if(k_sig_k > 0){
                        all_zeros = false;
                        break;
                    }
                }
                if(all_zeros){
                    r = 0;
                    while((r < 4) && (stripes[(i + r)*width + j] == 0)) r++;
                    
                    if(r == 4){
                        // debug_encode(false, 9, loc, pass, 'c');
                        mq_encoder.encode(false, 9);
                    }
                    else{
                        // run interruption
                        // debug_encode(true, 9, loc, pass, 'c');
                        mq_encoder.encode(true, 9);
                        bool symbol = (r/2 > 0);
                        // debug_encode(symbol, 18, loc, pass, 'c');
                        mq_encoder.encode(symbol, 18);
                        symbol = (r % 2 == 1);
                        // debug_encode(symbol, 18, loc, pass, 'c');
                        mq_encoder.encode(symbol, 18);
                    }
                    //debug_run_mode(r, loc, pass);
                }
            }
            if((sig_state[loc.y*cols + loc.x] == 0) && (member[loc.y*cols + loc.x] == 0)){
                if(r >= 0) r--;
                else{
                    k_sig = sig_context(loc);
                    // debug_encode(stripes[i*width + j] == 1, k_sig, loc, pass, 'c');
                    bool symbol = stripes[i*width + j] == 1;
                    update_implied_coeffs(loc, pass, symbol);
                    mq_encoder.encode(symbol, k_sig);
                }
                if(stripes[i*width + j] == 1){
                    sig_state[loc.y*cols + loc.x] = 1;
                    encode_sign(loc, pass, 'c');
                }
            }
        }
    }
}

bit_plane BitPlaneEncoderRD::get_bit_plane(int p){
    bit_plane temp;
    bool found_plane = false;
    for(auto &bp : B_i.bit_planes){
        if(bp.bit_level == p){
            temp = bp;
            found_plane = true;
            break;
        }
    }
    if(!found_plane) cout << "No Plane Found at bit level p: " << p << "\n";
    return temp;
}

vector<uint8_t> BitPlaneEncoderRD::get_stripes(int start_row, int p, int &height, int width){
    // First locate the bit plane in the bitplanes vector
    // Because of the variable height of the stripes depending on the codeblock dimensions
    // divisibility by 4, height is passed by reference so it can be used outside of this function
    vector<uint8_t> stripes;
    height = 0;
    bit_plane bp = get_bit_plane(p);
    if(start_row > rows - 1){
        cout << "Accessed row beyond bit_plane!";
        return stripes;
    }
    else{
        int i=0;
        for(i=0; i<4; i++){
            if(start_row + i <= rows - 1){
                height++;
                for(int j=0; j<width; j++){
                    stripes.push_back(bp.plane_data[(i+start_row)*cols + j]);
                }
            }
            else break;
        }
        return stripes;
    }
}

void BitPlaneEncoderRD::encode_code_block(){
    int pass_count = 0;
    for(int p=B_i.num_bits - 1; p>=0; p--){
        if(p < B_i.num_bits - 1){
            sig_prop_pass_full(p);
            // snapshot(++pass_count);
            mag_ref_pass_full(p);
            // snapshot(++pass_count);
        }
        cleanup_pass_full(p);
        // snapshot(++pass_count);
    }
}

void BitPlaneEncoderRD::make_coded_block(){
    out_block.bits = B_i.num_bits;
    out_block.height = rows;
    out_block.width = cols;
    out_block.anchor = B_i.anchor;
    out_block.sb_info = B_i.s_b;
    out_block.lengths = L_z;
    out_block.code_words = code_stream;
}

void BitPlaneEncoderRD::term_and_append_length(){
    mq_encoder.easy_term();
    vector<uint8_t> cw = mq_encoder.get_codeword();
    code_stream.push_back(cw);
    L_z.push_back(static_cast<int>(cw.size()));
    double dist = calc_total_distortion();
    pair<int, double> rd_pair = make_pair(static_cast<int>(cw.size()), dist);
    out_block.rate_dis.push_back(rd_pair);
    mq_encoder.reset_encoder();
}

void BitPlaneEncoderRD::sig_prop_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    while(start_row < rows){
        int height;
        vector<uint8_t> stripes = get_stripes(start_row, p, height, cols);
        anchor.y = start_row;
        sig_prop_pass(stripes, anchor, height, cols, p);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoderRD::mag_ref_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    while(start_row < rows){
        int height;
        vector<uint8_t> stripes = get_stripes(start_row, p, height, cols);
        anchor.y = start_row;
        mag_ref_pass(stripes, anchor, height, cols, p);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoderRD::cleanup_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    double thresh = B_i.s_b.step_size * pow(2.0, static_cast<double>(p));
    double dist = 0;
    while(start_row < rows){
        int height;
        vector<uint8_t> stripes = get_stripes(start_row, p, height, cols);
        anchor.y = start_row;
        clean_up_pass(stripes, anchor, height, cols, p);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoderRD::snapshot(int pass_count){
    const string &base_filename = "encoder_snapshot";
    ofstream out(base_filename + "_" + to_string(pass_count) + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sig_state:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            out << (sig_state[i*cols + j] == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\ndel_sig:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            out << (del_sig[i*cols + j] == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\nmember:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            out << (member[i*cols + j] == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }
    out.close();
}

void BitPlaneEncoderRD::print_sign_state(){
    const string &base_filename = "encoder_sign_state";
    ofstream out(base_filename + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sign state:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            int8_t val = B_i.sign_data[i*cols + j];
            if(val >= 0) out << " ";
            out << static_cast<int>(val) << " ";
        }
        out << "\n";
    }
}

void BitPlaneEncoderRD::debug_encode(bool bit, uint8_t ctx, point loc, int pass, char pass_type){
    FILE * pFile;
    pFile = fopen("encode_log_file.txt", "a");
    std::fprintf(pFile,
        "ENC %c p=%d loc=(%d,%d) ctx=%2d bit=%d\n",
        pass_type, pass, loc.x, loc.y, ctx, bit);
    fclose(pFile);
    mq_encoder.encode(bit, ctx);
}

void BitPlaneEncoderRD::debug_run_mode(int r, point loc, int pass){
    FILE * pFile;
    pFile = fopen("encode_log_file.txt", "a");
    std::fprintf(pFile,
        "ENC p=%d loc=(%d,%d) r= %d\n",
        pass, loc.x, loc.y, r);
    fclose(pFile);
}

double BitPlaneEncoderRD::get_sig_dist(uint8_t ctx, bool symbol, double thresh){
    double prob = mq_encoder.get_prob(ctx, symbol);
    double dist = prob * 2.25 * thresh * thresh;
    return dist;
}

double BitPlaneEncoderRD::get_mag_dist(double thresh){
    double dist = 0.25 * thresh * thresh;
    return dist; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

BitPlaneDecoderRD::BitPlaneDecoderRD(){

}

BitPlaneDecoderRD::BitPlaneDecoderRD(coded_block &in_block){
    L_z = in_block.lengths;
    code_stream = in_block.code_words;
    rows = in_block.height;
    cols = in_block.width;
    num_bits = in_block.bits;
    B_i.s_b = in_block.sb_info;
    anchor = in_block.anchor;
    Z_hat = 3 * num_bits - 2; // Maximum number of coding passes
    if (rows <= 0 || cols <= 0 || num_bits <= 0 || num_bits > 64) {
        throw std::runtime_error("Invalid dimensions or bit depth");
    }
    if (code_stream.empty()) {
        throw std::runtime_error("Empty code stream");
    }
    init_mq_decoder();
    init_code_block();
    init_state_tables();
}

void BitPlaneDecoderRD::load_encoded_block(coded_block &in_block){
    L_z = in_block.lengths;
    code_stream = in_block.code_words;
    rows = in_block.height;
    cols = in_block.width;
    num_bits = in_block.bits;
    B_i.s_b = in_block.sb_info;
    anchor = in_block.anchor;
    Z_hat = 3 * num_bits - 2; // Maximum number of coding passes
    if (rows <= 0 || cols <= 0 || num_bits <= 0 || num_bits > 64) {
        throw std::runtime_error("Invalid dimensions or bit depth");
    }
    if (code_stream.empty()) {
        throw std::runtime_error("Empty code stream");
    }
}

void BitPlaneDecoderRD::reset_bp_decoder(){
    init_mq_decoder();
    init_code_block();
    init_state_tables();
}

void BitPlaneDecoderRD::init_mq_decoder(){
    mq_decoder.load_codeword(code_stream.front());
    mq_decoder.reset_decoder();
    vector<context_table_entry> cx_table;
    cx_table = mq_decoder.init_context_table();
    mq_decoder.set_context_table(cx_table);
}

void BitPlaneDecoderRD::init_code_block(){
    vector<bit_plane>().swap(B_i.bit_planes);
    B_i.num_bits = num_bits;
    B_i.anchor = anchor;
    try {
        B_i.sign_data = vector<int8_t>(rows*cols, 0);
    } catch (const std::bad_alloc& e) {
        cerr << "Failed to allocate sign_data: " << e.what() << "\n";
        throw;
    }
    for(int i=num_bits - 1; i>=0; i--){
        vector<uint8_t> plane_data(rows*cols, 0);
        bit_plane bp;
        bp.plane_data = plane_data;
        bp.bit_level = i;
        B_i.bit_planes.push_back(bp);
    }
    sort(B_i.bit_planes.begin(), B_i.bit_planes.end(), 
        [](const bit_plane &a, const bit_plane &b) { 
            return a.bit_level > b.bit_level; 
        });
    B_i.height = rows;
    B_i.width = cols;
}

void BitPlaneDecoderRD::init_state_tables(){
    sig_state = vector<uint8_t>(rows*cols, 0);
    del_sig = vector<uint8_t>(rows*cols, 0);
    member = vector<uint8_t>(rows*cols, 0);
}

void BitPlaneDecoderRD::snapshot(int pass_count){
    const string &base_filename = "decoder_snapshot";
    ofstream out(base_filename + "_" + to_string(pass_count) + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sig_state:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            uint8_t val = sig_state[i*cols + j];
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\ndel_sig:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            uint8_t val = del_sig[i*cols + j];
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\nmember:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            uint8_t val = member[i*cols + j];
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }

    out.close();
}

void BitPlaneDecoderRD::recover_codeblock(){
    int z;
    int pass_count = 0;
    for(int p = num_bits-1; p>=0; p--){
        // cout << "Bit: " << p << " pass\n";
        z = 3 * (num_bits - 1 - p) - 1;
        if(p < num_bits - 1){
            if(z <= Z_hat){
                sig_prop_pass(p);
                // snapshot(++pass_count);
                resync_decoder();
            }
            if((z + 1) <= Z_hat){
                mag_ref_pass(p);
                // snapshot(++pass_count);
                resync_decoder();
            }
        }
        if(z + 2 <= Z_hat){
            cleanup_pass(p);
            // snapshot(++pass_count);
            resync_decoder();
        }
    }
}

uint8_t BitPlaneDecoderRD::sig_context(point loc){
    int k_h = 0, k_v = 0, k_d = 0;
    uint8_t context;
    if(loc.x != 0){
        k_h += sig_state[loc.y*cols + loc.x - 1];
        if(loc.y != 0){
            k_v += sig_state[(loc.y - 1)*cols + loc.x];
            k_d += sig_state[(loc.y - 1)*cols + loc.x - 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[(loc.y + 1)*cols + loc.x];
            k_d += sig_state[(loc.y + 1)*cols + loc.x - 1];
        }
    }
    if(loc.x != cols - 1){
        k_h += sig_state[(loc.y)*cols + loc.x + 1];
        if(loc.y != 0){
            k_v += sig_state[(loc.y - 1)*cols + loc.x];
            k_d += sig_state[(loc.y - 1)*cols + loc.x + 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[(loc.y + 1)*cols + loc.x];
            k_d += sig_state[(loc.y + 1)*cols + loc.x + 1];
        }
    }
    if(B_i.s_b.type == 0 || B_i.s_b.type == 2){
        if(k_h == 2) context = 8;
        else if(k_h == 1){
            if(k_v >= 1) context = 7;
            else if(k_d >= 1) context = 6;
            else context = 5;
        }
        else{
            if(k_v == 2) context = 4;
            else if(k_v == 1) context = 3;
            else if(k_d >= 2) context = 2;
            else if(k_d == 1) context = 1;
            else context = 0;
        }
    }
    else if(B_i.s_b.type == 1){
        if(k_v == 2) context = 8;
        else if(k_v == 1){
            if(k_h >= 1) context = 7;
            else if(k_d >= 1) context = 6;
            else context = 5;
        }
        else{
            if(k_h == 2) context = 4;
            else if(k_h == 1) context = 3;
            else{
                if(k_d >= 2) context = 2;
                else if(k_d == 1) context = 1;
                else context = 0;
            }
        }
    }
    else{
        int k_hv = k_h + k_v;
        if(k_d >= 3) context = 8;
        else if(k_d == 2){
            if(k_hv >= 1) context = 7;
            else context = 6;
        }
        else if(k_d == 1){
            if(k_hv >= 2) context = 5;
            else if(k_hv == 1) context = 4;
            else context = 3;
        }
        else{
            if(k_hv >= 2) context = 2;
            else if(k_hv == 1) context = 1;
            else context = 0;
        }
    }
    return context;
}

pair<uint8_t, int8_t> BitPlaneDecoderRD::sign_context(point loc){
    int8_t x_h = 0, x_v = 0;
    
    if(loc.x != 0){
        x_h += sig_state[loc.y*cols + loc.x - 1] * B_i.sign_data[loc.y*cols + loc.x - 1];
    }
    if(loc.x != cols -1){
        x_h += sig_state[loc.y*cols + loc.x + 1] * B_i.sign_data[loc.y*cols + loc.x + 1];
    }
    if(loc.y != 0){
        x_v += sig_state[(loc.y - 1)*cols + loc.x] * B_i.sign_data[(loc.y - 1)*cols + loc.x];
    }
    if(loc.y != rows -1){
        x_v += sig_state[(loc.y + 1)*cols + loc.x] * B_i.sign_data[(loc.y + 1)*cols + loc.x];
    }


    int8_t flip;
    uint8_t context;
    if(x_h > 0){
        if(x_v > 0) context = 14;
        else if(x_v == 0) context = 13;
        else context = 12;
    }
    else if(x_h == 0){
        if(x_v != 0) context = 11;
        else context = 10;
    }
    else{
        if(x_v > 0) context = 12;
        else if(x_v == 0) context = 13;
        else context = 14;
    }
    
    if(x_h > 0){
        flip = 1;
    }
    else if(x_h == 0){
        if(x_v >= 0) flip = 1;
        else flip = -1;
    }
    else{
        flip = -1;
    }

    pair<uint8_t, int8_t> out_pair;
    out_pair = make_pair(context, flip);

    return out_pair;
}

uint8_t BitPlaneDecoderRD::mag_context(point loc){
    uint8_t context;
    if(del_sig[loc.y*cols + loc.x] == 1) context = 17;
    else{
        uint8_t k_sig = sig_context(loc);
        if(k_sig == 0) context = 15;
        else context = 16;
    }
    return context;
}

void BitPlaneDecoderRD::sig_prop_pass(int p){
    uint8_t k_sig;
    point loc;
    int bp_index = get_plane_index(p);
    int start_row = 0;
    while(start_row < rows){
        for(int j=0; j<cols; j++){
            loc.x = j;
            for(int i=0; i<4; i++){
                loc.y = start_row + i;
                if(loc.y == rows) break;
                k_sig = sig_context(loc);
                if((sig_state[loc.y*cols + loc.x] == 0) && (k_sig > 0)){
                    bool symbol = mq_decoder.decode(k_sig);
                    //bool symbol = debug_decode(k_sig, loc, p, 's');
                    B_i.bit_planes[bp_index].plane_data[loc.y*cols + loc.x] = symbol? 1: 0;
                    if(symbol){
                        sig_state[loc.y*cols + loc.x] = 1;
                        decode_sign(loc, p, 's');
                    }
                    member[loc.y*cols + loc.x] = 1;
                }
                else member[loc.y*cols + loc.x] = 0;
            }
        }
        start_row += 4;
    }
}

int BitPlaneDecoderRD::get_plane_index(int p){
    auto it = find_if(B_i.bit_planes.begin(), B_i.bit_planes.end(), [p](const bit_plane &bp) 
    {
        return bp.bit_level == p;
    });
    if(it != B_i.bit_planes.end()){
        int index = distance(B_i.bit_planes.begin(), it);
        return index;
    }
    else{
        cout << "No Bit Plane at bit level p: " << p << "\n";
        return -1;
    }
}

void BitPlaneDecoderRD::decode_sign(point loc, int pass, char pass_type){
    pair<uint8_t, int8_t> sign_pair = sign_context(loc);
    bool symbol = mq_decoder.decode(sign_pair.first);
    //bool symbol = debug_decode(sign_pair.first, loc, pass, pass_type);
    int8_t sign;
    if(!symbol){
        sign = sign_pair.second;
    }
    else{
        sign = -1 * sign_pair.second;
    }
    B_i.sign_data[loc.y*cols + loc.x] = sign;
}

void BitPlaneDecoderRD::mag_ref_pass(int p){
    uint8_t k_mag;
    point loc;
    int bp_index = get_plane_index(p);
    int start_row = 0;
    while(start_row < rows){
        for(int j=0; j<cols; j++){
            loc.x = j;
            for(int i=0; i<4; i++){
                loc.y = start_row + i;
                if(loc.y == rows) break;

                if((sig_state[loc.y*cols + loc.x] == 1) && (member[loc.y*cols + loc.x] == 0)){
                    k_mag = mag_context(loc);
                    bool symbol = mq_decoder.decode(k_mag);
                    // bool symbol = debug_decode(k_mag, loc, p, 'm');
                    B_i.bit_planes[bp_index].plane_data[loc.y*cols + loc.x] = (symbol ? 1 : 0);
                    del_sig[loc.y*cols + loc.x] = sig_state[loc.y*cols + loc.x];
                }
            }
        }
        start_row += 4;
    }        
}

void BitPlaneDecoderRD::cleanup_pass(int p){
    uint8_t k_sig;
    point loc;
    int bp_index = get_plane_index(p);
    int start_row = 0;
    int r;
    bool symbol;
    while(start_row < rows){
        for(int j=0; j<cols; j++){
            loc.x = j;
            for(int i=0; i<4; i++){
                loc.y = start_row + i;
                if(loc.y == rows) break;
                if((loc.y % 4 == 0) && loc.y <= rows - 4){
                    //Entering a full stripe column
                    r = -1;
                    //Check for all zero significance contexts in column
                    bool all_zeros = true;
                    for(int k = 0; k<4; k++){
                        k_sig = sig_context({loc.x, loc.y + k});
                        if(k_sig != 0){
                            all_zeros = false;
                            break;
                        }
                    }
                    if(all_zeros){
                        symbol = mq_decoder.decode(9);
                        // symbol = debug_decode(9, loc, p, 'c');
                        if(!symbol){
                            r = 4;
                        }
                        else{
                            // Run Interruption
                            symbol = mq_decoder.decode(18);
                            // symbol = debug_decode(18, loc, p, 'c');                            
                            r = symbol? 2 : 0;
                            // symbol = debug_decode(18, loc, p, 'c'); 
                            symbol = mq_decoder.decode(18);
                            r += symbol? 1 : 0;
                            B_i.bit_planes[bp_index].plane_data[(loc.y + r)*cols + loc.x] = 1;
                        }
                        //debug_run_mode(r, loc, p);
                    }
                }
                if((sig_state[loc.y*cols + loc.x] == 0) && (member[loc.y*cols + loc.x] == 0)){
                    if(r >= 0) r--;
                    else{
                        k_sig = sig_context(loc);
                        // symbol = debug_decode(k_sig, loc, p, 'c'); 
                        symbol = mq_decoder.decode(k_sig);
                        B_i.bit_planes[bp_index].plane_data[loc.y*cols + loc.x] = (symbol? 1 : 0);
                    }
                    if(B_i.bit_planes[bp_index].plane_data[loc.y*cols + loc.x] == 1){
                        sig_state[loc.y*cols + loc.x] = 1;
                        decode_sign(loc, p, 'c');
                    }
                }
            }
        }
        start_row += 4;
    }            
}

void BitPlaneDecoderRD::resync_decoder(){
    if(L_z.size() != 0) L_z.pop_front();
    if(code_stream.size() != 0) code_stream.pop_front();
    if(code_stream.size() != 0) mq_decoder.load_codeword(code_stream.front());
    else mq_decoder.load_codeword({0xFF});
    mq_decoder.reset_decoder();
}

void BitPlaneDecoderRD::print_sign_state(){
    const string &base_filename = "decoder_sign_state";
    ofstream out(base_filename + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sign state:\n";
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            int8_t val = B_i.sign_data[i*cols + j];
            if(val >= 0) out << " ";
            out << static_cast<int>(val) << " ";
        }
        out << "\n";
    }
}

bool BitPlaneDecoderRD::debug_decode(uint8_t ctx, point loc, int pass, char pass_type){
    FILE * pFile;
    pFile = fopen("decode_log_file.txt", "a");
    bool symbol = mq_decoder.decode(ctx);
    std::fprintf(pFile,
        "ENC %c p=%d loc=(%d,%d) ctx=%2d bit=%d\n",
        pass_type, pass, loc.x, loc.y, ctx, symbol);
    fclose(pFile);
    return symbol;
}
void BitPlaneDecoderRD::debug_run_mode(int r, point loc, int pass){
    FILE * pFile;
    pFile = fopen("decode_log_file.txt", "a");
    std::fprintf(pFile,
        "ENC p=%d loc=(%d,%d) r= %d\n",
        pass, loc.x, loc.y, r);
    fclose(pFile);
}