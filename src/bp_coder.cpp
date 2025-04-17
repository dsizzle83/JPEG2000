#include "../include/bp_coder.h"

using namespace std;


BitPlaneEncoder::BitPlaneEncoder(code_block &c_b) 
    : B_i(c_b)
{
    rows = static_cast<int>(B_i.sign_data.size());
    cols = static_cast<int>(B_i.sign_data[0].size());
    init_state_tables();
}

void BitPlaneEncoder::init_state_tables(){
    sig_state = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
    del_sig = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
    member = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
}

uint8_t BitPlaneEncoder::sig_context(point loc){
    int k_h = 0, k_v = 0, k_d = 0;
    uint8_t context;
    if(loc.x != 0){
        k_h += sig_state[loc.y][loc.x - 1];
        if(loc.y != 0){
            k_v += sig_state[loc.y - 1][loc.x];
            k_d += sig_state[loc.y - 1][loc.x - 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[loc.y + 1][loc.x];
            k_d += sig_state[loc.y + 1][loc.x - 1];
        }
    }
    if(loc.x != cols - 1){
        k_h += sig_state[loc.y][loc.x + 1];
        if(loc.y != 0){
            k_v += sig_state[loc.y - 1][loc.x];
            k_d += sig_state[loc.y - 1][loc.x + 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[loc.y + 1][loc.x];
            k_d += sig_state[loc.y + 1][loc.x + 1];
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

pair<uint8_t, int8_t> BitPlaneEncoder::sign_context(point loc){
    int8_t x_h = 0, x_v = 0;
    
    if(loc.x != 0){
        x_h += sig_state[loc.y][loc.x - 1] * B_i.sign_data[loc.y][loc.x - 1];
    }
    if(loc.x != cols -1){
        x_h += sig_state[loc.y][loc.x + 1] * B_i.sign_data[loc.y][loc.x + 1];
    }
    if(loc.y != 0){
        x_v += sig_state[loc.y - 1][loc.x] * B_i.sign_data[loc.y - 1][loc.x];
    }
    if(loc.y != rows -1){
        x_v += sig_state[loc.y + 1][loc.x] * B_i.sign_data[loc.y + 1][loc.x];
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

uint8_t BitPlaneEncoder::mag_context(point loc){
    uint8_t context;
    if(del_sig[loc.y][loc.x] == 1) context = 17;
    else{
        uint8_t k_sig = sig_context(loc);
        if(k_sig == 0) context = 15;
        else context = 16;
    }
    return context;
}

void BitPlaneEncoder::sig_prop_pass(vector<vector<uint8_t>> &stripes, point anchor){
    uint8_t k_sig;
    point loc;
    for(size_t j=0; j<stripes[0].size(); j++){
        loc.x = anchor.x + j;
        for(size_t i=0; i<stripes.size(); i++){
            loc.y = anchor.y + i;
            k_sig = sig_context(loc);
            if((sig_state[loc.y][loc.x] == 0) && (k_sig > 0)){
                mq_encoder.encode((stripes[i][j] == 1), k_sig);
                if(stripes[i][j] == 1){
                    sig_state[loc.y][loc.x] = 1;
                    encode_sign(loc);
                }
                member[loc.y][loc.x] = 1;
            }
            else member[loc.y][loc.x] = 0;
        }
    }
}

void BitPlaneEncoder::encode_sign(point loc){
    pair<uint8_t, int8_t> sign_pair = sign_context(loc);
    int8_t sign = B_i.sign_data[loc.y][loc.x];
    bool symbol = ((sign * sign_pair.second) == 1)? false : true;

    mq_encoder.encode(symbol, sign_pair.first);
}

void BitPlaneEncoder::mag_ref_pass(vector<vector<uint8_t>> &stripes, point anchor){
    uint8_t k_mag;
    point loc;
    for(size_t j=0; j<stripes[0].size(); j++){
        loc.x = anchor.x + j;
        for(size_t i=0; i<stripes.size(); i++){
            loc.y = anchor.y + i;
            if(sig_state[loc.y][loc.x] && !member[loc.y][loc.x]){
                k_mag = mag_context(loc);
                mq_encoder.encode((stripes[i][j] == 1), k_mag);
                del_sig[loc.y][loc.x] = sig_state[loc.y][loc.x];
            }
        }
    }
}

void BitPlaneEncoder::clean_up_pass(vector<vector<uint8_t>> &stripes, point anchor){
    uint8_t k_sig;
    point loc;
    int r;
    for(size_t j = 0; j < stripes[0].size(); j++){
        loc.x = anchor.x + j;
        for(size_t i=0; i< stripes.size(); i++){
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
                    while((r < 4) && (stripes[i + r][j] == 0)) r++;
                    if(r == 4){
                        mq_encoder.encode(false, 9);
                    }
                    else{
                        // run interruption
                        mq_encoder.encode(true, 9);
                        bool symbol = (r/2 > 0);
                        mq_encoder.encode(symbol, 18);
                        symbol = (r % 2 == 1);
                        mq_encoder.encode(symbol, 18);
                    }
                }
            }
            if((sig_state[loc.y][loc.x] == 0) && (member[loc.y][loc.x] == 0)){
                if(r >= 0) r--;
                else{
                    k_sig = sig_context(loc);
                    mq_encoder.encode((stripes[i][j] == 1), k_sig);
                }
                if(stripes[i][j] == 1){
                    sig_state[loc.y][loc.x] = 1;
                    encode_sign(loc);
                }
            }
        }
    }
}

bit_plane BitPlaneEncoder::get_bit_plane(int p){
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

vector<vector<uint8_t>> BitPlaneEncoder::get_stripes(int start_row, int p){
    // First locate the bit plane in the bitplanes vector
    vector<vector<uint8_t>> stripes;
    bit_plane bp = get_bit_plane(p);
    if(start_row > rows - 1){
        cout << "Accessed row beyond bit_plane!";
        return stripes;
    }
    else{
        int i=0;
        for(i=0; i<4; i++){
            if(i + start_row < rows){
                stripes.push_back(bp.plane_data[start_row + i]);  
            }
        }
        return stripes;
    }
}

void BitPlaneEncoder::encode_code_block(){
    int pass_count = 0;
    for(int p=B_i.num_bits - 1; p>=0; p--){
        if(p < B_i.num_bits - 1){
            sig_prop_pass_full(p);
            mag_ref_pass_full(p);
        }
        cleanup_pass_full(p);
    }
}

void BitPlaneEncoder::make_coded_block(){
    out_block.bits = B_i.num_bits;
    out_block.height = rows;
    out_block.width = cols;
    out_block.anchor = B_i.anchor;
    out_block.sb_info = B_i.s_b;
    out_block.lengths = L_z;
    out_block.code_words = code_stream;
}

void BitPlaneEncoder::term_and_append_length(){
    mq_encoder.easy_term();
    vector<uint8_t> cw = mq_encoder.get_codeword();
    code_stream.push_back(cw);
    L_z.push_back(cw.size());
    mq_encoder.reset_encoder();
}

void BitPlaneEncoder::sig_prop_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    while(start_row < rows){
        vector<vector<uint8_t>> stripes = get_stripes(start_row, p);
        anchor.y = start_row;
        sig_prop_pass(stripes, anchor);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoder::mag_ref_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    while(start_row < rows){
        vector<vector<uint8_t>> stripes = get_stripes(start_row, p);
        anchor.y = start_row;
        mag_ref_pass(stripes, anchor);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoder::cleanup_pass_full(int p){
    int start_row = 0;
    point anchor = {0, 0};
    while(start_row < rows){
        vector<vector<uint8_t>> stripes = get_stripes(start_row, p);
        anchor.y = start_row;
        clean_up_pass(stripes, anchor);
        start_row += 4;
    }
    term_and_append_length();
}

void BitPlaneEncoder::snapshot(int pass_count){
    const string &base_filename = "encoder_snapshot";
    ofstream out(base_filename + "_" + to_string(pass_count) + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sig_state:\n";
    for(const auto &row : sig_state){
        for(uint8_t val: row){
            out << (val == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\ndel_sig:\n";
    for(const auto &row : del_sig){
        for(uint8_t val: row){
            out << (val == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\nmember:\n";
    for(const auto &row : member){
        for(uint8_t val: row){
            out << (val == 1 ? "1 " : "0 ");
        }
        out << "\n";
    }
    out.close();
}

void BitPlaneEncoder::print_sign_state(){
    const string &base_filename = "encoder_sign_state";
    ofstream out(base_filename + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sign state:\n";
    for(const auto &row : B_i.sign_data){
        for(int8_t val: row){
            if(val >= 0) out << " ";
            out << static_cast<int>(val) << " ";
        }
        out << "\n";
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

BitPlaneDecoder::BitPlaneDecoder(deque<vector<uint8_t>> code_words, int height, int width, int bits, subband_info sb_info, point origin, deque<int> lengths){
    L_z = lengths;
    code_stream = code_words;
    rows = height;
    cols = width;
    num_bits = bits;
    B_i.s_b = sb_info;
    anchor = origin;
    Z_hat = 3 * num_bits - 2; // Maximum number of coding passes
    init_mq_decoder();
    init_code_block();
    init_state_tables();
}

BitPlaneDecoder::BitPlaneDecoder(coded_block in_block){
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

void BitPlaneDecoder::init_mq_decoder(){
    mq_decoder.load_codeword(code_stream.front());
    mq_decoder.reset_decoder();
    vector<context_table_entry> cx_table;
    cx_table = mq_decoder.init_context_table();
    mq_decoder.set_context_table(cx_table);
}

void BitPlaneDecoder::init_code_block(){
    B_i.num_bits = num_bits;
    B_i.anchor = anchor;
    try {
        B_i.sign_data = vector<vector<int8_t>>(rows, vector<int8_t>(cols, 0));
    } catch (const std::bad_alloc& e) {
        cerr << "Failed to allocate sign_data: " << e.what() << "\n";
        throw;
    }
    for(int i=num_bits - 1; i>=0; i--){
        vector<vector<uint8_t>> plane_data(rows, vector<uint8_t>(cols, 0));
        bit_plane bp;
        bp.plane_data = plane_data;
        bp.bit_level = i;
        B_i.bit_planes.push_back(bp);
    }
}

void BitPlaneDecoder::init_state_tables(){
    sig_state = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
    del_sig = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
    member = vector<vector<uint8_t>>(rows, vector<uint8_t>(cols, 0));
}

void BitPlaneDecoder::snapshot(int pass_count){
    const string &base_filename = "decoder_snapshot";
    ofstream out(base_filename + "_" + to_string(pass_count) + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sig_state:\n";
    for(const auto &row : sig_state){
        for(uint8_t val: row){
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\ndel_sig:\n";
    for(const auto &row : del_sig){
        for(uint8_t val: row){
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }

    out << "\nmember:\n";
    for(const auto &row : member){
        for(uint8_t val: row){
            out << (val == 1? "1 " : "0 ");
        }
        out << "\n";
    }
    out.close();
}

void BitPlaneDecoder::recover_codeblock(){
    int z;
    int pass_count = 0;
    for(int p = num_bits-1; p>=0; p--){
        // cout << "Bit: " << p << " pass\n";
        z = 3 * (num_bits - 1 - p) - 1;
        if(p < num_bits - 1){
            if(z <= Z_hat){
                sig_prop_pass(p);
                resync_decoder();
            }
            if((z + 1) <= Z_hat){
                mag_ref_pass(p);
                resync_decoder();
            }
        }
        if(z + 2 <= Z_hat){
            cleanup_pass(p);
            resync_decoder();
        }
    }
    // cout << "Finished Recover Codeblock\n";
}

uint8_t BitPlaneDecoder::sig_context(point loc){
    int k_h = 0, k_v = 0, k_d = 0;
    uint8_t context;
    if(loc.x != 0){
        k_h += sig_state[loc.y][loc.x - 1];
        if(loc.y != 0){
            k_v += sig_state[loc.y - 1][loc.x];
            k_d += sig_state[loc.y - 1][loc.x - 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[loc.y + 1][loc.x];
            k_d += sig_state[loc.y + 1][loc.x - 1];
        }
    }
    if(loc.x != cols - 1){
        k_h += sig_state[loc.y][loc.x + 1];
        if(loc.y != 0){
            k_v += sig_state[loc.y - 1][loc.x];
            k_d += sig_state[loc.y - 1][loc.x + 1];
        }
        if(loc.y != rows - 1){
            k_v += sig_state[loc.y + 1][loc.x];
            k_d += sig_state[loc.y + 1][loc.x + 1];
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

pair<uint8_t, int8_t> BitPlaneDecoder::sign_context(point loc){
    int8_t x_h = 0, x_v = 0;
    
    if(loc.x != 0){
        x_h += sig_state[loc.y][loc.x - 1] * B_i.sign_data[loc.y][loc.x - 1];
    }
    if(loc.x != cols -1){
        x_h += sig_state[loc.y][loc.x + 1] * B_i.sign_data[loc.y][loc.x + 1];
    }
    if(loc.y != 0){
        x_v += sig_state[loc.y - 1][loc.x] * B_i.sign_data[loc.y - 1][loc.x];
    }
    if(loc.y != rows -1){
        x_v += sig_state[loc.y + 1][loc.x] * B_i.sign_data[loc.y + 1][loc.x];
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

uint8_t BitPlaneDecoder::mag_context(point loc){
    uint8_t context;
    if(del_sig[loc.y][loc.x] == 1) context = 17;
    else{
        uint8_t k_sig = sig_context(loc);
        if(k_sig == 0) context = 15;
        else context = 16;
    }
    return context;
}

void BitPlaneDecoder::sig_prop_pass(int p){
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
                if((sig_state[loc.y][loc.x] == 0) && (k_sig > 0)){
                    bool symbol = mq_decoder.decode(k_sig);
                    B_i.bit_planes[bp_index].plane_data[loc.y][loc.x] = symbol? 1: 0;
                    if(symbol){
                        sig_state[loc.y][loc.x] = 1;
                        decode_sign(loc);
                    }
                    member[loc.y][loc.x] = 1;
                }
                else member[loc.y][loc.x] = 0;
            }
        }
        start_row += 4;
    }
}

int BitPlaneDecoder::get_plane_index(int p){
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

void BitPlaneDecoder::decode_sign(point loc){
    pair<uint8_t, int8_t> sign_pair = sign_context(loc);
    bool symbol = mq_decoder.decode(sign_pair.first);
    int8_t sign;
    if(!symbol){
        sign = sign_pair.second;
    }
    else{
        sign = -1 * sign_pair.second;
    }
    B_i.sign_data[loc.y][loc.x] = sign;
}

void BitPlaneDecoder::mag_ref_pass(int p){
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

                if((sig_state[loc.y][loc.x] == 1) && (member[loc.y][loc.x] == 0)){
                    k_mag = mag_context(loc);
                    bool symbol = mq_decoder.decode(k_mag);
                    B_i.bit_planes[bp_index].plane_data[loc.y][loc.x] = (symbol ? 1 : 0);
                    del_sig[loc.y][loc.x] = sig_state[loc.y][loc.x];
                }
            }
        }
        start_row += 4;
    }        
}

void BitPlaneDecoder::cleanup_pass(int p){
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
                        if(!symbol){
                            r = 4;
                        }
                        else{
                            // Run Interruption
                            symbol = mq_decoder.decode(18);
                            r = symbol? 2 : 0;
                            symbol = mq_decoder.decode(18);
                            r += symbol? 1 : 0;
                            B_i.bit_planes[bp_index].plane_data[loc.y + r][loc.x] = 1;
                        }
                    }
                }
                if((sig_state[loc.y][loc.x] == 0) && (member[loc.y][loc.x] == 0)){
                    if(r >= 0) r--;
                    else{
                        k_sig = sig_context(loc);
                        symbol = mq_decoder.decode(k_sig);
                        B_i.bit_planes[bp_index].plane_data[loc.y][loc.x] = (symbol? 1 : 0);
                    }
                    if(B_i.bit_planes[bp_index].plane_data[loc.y][loc.x] == 1){
                        sig_state[loc.y][loc.x] = 1;
                        decode_sign(loc);
                    }
                }
            }
        }
        start_row += 4;
    }            
}

void BitPlaneDecoder::resync_decoder(){
    L_z.pop_front();
    code_stream.pop_front();
    mq_decoder.load_codeword(code_stream.front());
    mq_decoder.reset_decoder();
}

void BitPlaneDecoder::print_sign_state(){
    const string &base_filename = "decoder_sign_state";
    ofstream out(base_filename + ".txt");
    if(!out){
        cout << "Failed to open snapshot file.\n";
    }

    out << "sign state:\n";
    for(const auto &row : B_i.sign_data){
        for(int8_t val: row){
            if(val >= 0) out << " ";
            out << static_cast<int>(val) << " ";
        }
        out << "\n";
    }
}