#include "../include/mq_coder.h"

using namespace std;

MQEncoder::MQEncoder(){
    reset_encoder();
    context_table = init_context_table();
}
    


void MQEncoder::reset_encoder(){
    A = 0x8000;
    C = 0;
    t_count = 12;
    temp_byte = 0;
    L = -1;
    codeword.clear();
}

vector<context_table_entry> MQEncoder::init_context_table(){
    // Assume 18 contexts, all started at zero
    vector<context_table_entry> contexts;
    for(int i=0; i<19; i++){
        context_table_entry cte = {0, false};
        contexts.push_back(cte);
    }
    contexts[0].lut_entry = 4;
    for(int i=1; i<9; i++){
        contexts[i].lut_entry = 0;
    }
    contexts[9].lut_entry = 3;
    for(int i=10; i<18; i++){
        contexts[i].lut_entry = 0;
    }
    contexts[18].lut_entry = 46;

    for(int i=0; i<19; i++){
        contexts[i].s_k = false;
    }
    return contexts;
}

void MQEncoder::encode(bool x, uint8_t k){      
    A -= mq_lut[context_table[k].lut_entry].p;

    if(x == context_table[k].s_k){
        code_mps(k);
    }
    else{
        code_lps(k);
    }
}

void MQEncoder::code_mps(uint8_t k){
    uint16_t Qe = mq_lut[context_table[k].lut_entry].p;
    if(A >= 0x8000){
        C += Qe;
    }
    else{
        if(A < Qe){
            A = Qe;
        }
        else{
            C += Qe;
        }
        context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].mps;
        do{
            A = A << 1;
            C = C << 1;
            t_count--;
            if(t_count == 0) transfer_byte();
        } while(A < 0x8000);
    }
}

void MQEncoder::code_lps(uint8_t k){
    uint16_t Qe = mq_lut[context_table[k].lut_entry].p;
    
    if(A < Qe){
        C += Qe;
    }
    else{
        A = Qe;
    }
    context_table[k].s_k ^= mq_lut[context_table[k].lut_entry].x_s;
    context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].lps;

    do{
        A = A << 1;
        C = C << 1;
        t_count--;
        if(t_count == 0) transfer_byte();
    }while(A < 0x8000);
}

void MQEncoder::transfer_byte(){
    //cout << "Transferring: " << hex << (int)temp_byte << dec << endl;
    if(temp_byte == 0xFF){
        put_byte();
        temp_byte = (C >> 20) & 0xFF;
        C &= 0xFFFFF;
        t_count = 7;
    }
    else{
        temp_byte += (C >> 27) & 0x1;
        C &= 0x7FFFFFF;
        put_byte();
        if(temp_byte == 0xFF){
            temp_byte = (C >> 20) & 0xFF;
            C &= 0xFFFFF;
            t_count = 7;
        }
        else{
            temp_byte = (C >> 19) & 0xFF;
            C &= 0x7FFFF;
            t_count = 8;
        }
    }
}

void MQEncoder::put_byte(){
    if(L >= 0){
        codeword.push_back(temp_byte);
    }
    L++;
}

void MQEncoder::easy_term(){
    int n = 27 - 15 - t_count;
    C = C << t_count;
    // cout << "L: " << L << endl;
    while(n > 0){
        // cout << "byte" << endl;
        transfer_byte();
        n -= t_count;
        C = C << t_count;
    }
    // cout << "byte" << endl;
    transfer_byte();
    temp_byte = 0;
}

vector<uint8_t> MQEncoder::get_codeword(){
    return codeword;
}

void MQEncoder::print_state(){
    cout << "A: " << hex << A;
    cout << ", C: " << hex << C;
    cout << ", Temp Byte: " << hex << (int)temp_byte;
    cout << ", t_count: " << dec << (int)t_count;
    cout << ", L: " << dec << L << "\n";
}


MQDecoder::MQDecoder(vector<uint8_t> cw){
        load_codeword(cw);
        reset_decoder();        
        context_table = init_context_table();
    }

MQDecoder::MQDecoder(){

}

void MQDecoder::reset_decoder(){
    temp_byte = 0;
    L = 0;
    C = 0;
    fill_lsbs();
    C = C << t_count;
    fill_lsbs();
    C = C << 7;
    t_count -= 7;
    A = 0x8000;
}

void MQDecoder::load_codeword(vector<uint8_t> cw){
    codeword = cw;
    L_max = codeword.size();
}

vector<context_table_entry> MQDecoder::init_context_table(){
    // Assume 18 contexts, all started at zero
    vector<context_table_entry> contexts;
    for(int i=0; i<19; i++){
        context_table_entry cte = {0, false};
        contexts.push_back(cte);
    }
    contexts[0].lut_entry = 4;
    for(int i=1; i<9; i++){
        contexts[i].lut_entry = 0;
    }
    contexts[9].lut_entry = 3;
    for(int i=10; i<18; i++){
        contexts[i].lut_entry = 0;
    }
    contexts[18].lut_entry = 46;

    for(int i=0; i<19; i++){
        contexts[i].s_k = false;
    }
    return contexts;
}

void MQDecoder::fill_lsbs(){
    t_count = 8;
    BL = codeword[L];
    if((L == L_max) || ((temp_byte == 0xFF) && (BL > 0x8F))){
        C += 0xFF;
    }
    else{
        if(temp_byte == 0xFF){
            t_count = 7;
        }
        temp_byte = BL;
        L++;
        C = C + (temp_byte << (8-t_count));
    }
}

bool MQDecoder::decode(uint8_t k){
    uint16_t Qe = mq_lut[context_table[k].lut_entry].p;
    bool x = context_table[k].s_k;
    A -= Qe;
    if((C >> 8) >= Qe){
        C = C - (Qe << 8);
        if(A < 0x8000){
            if(A < Qe){
                x = !context_table[k].s_k;
                context_table[k].s_k ^= mq_lut[context_table[k].lut_entry].x_s;
                context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].lps;
            }
            else{
                context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].mps;
            }
            do{
                renorm_once();
            }while(A < 0x8000);
        }
    }
    else{
        if(A < Qe){
            context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].mps;
        }
        else{
            x = !context_table[k].s_k;
            context_table[k].s_k ^= mq_lut[context_table[k].lut_entry].x_s;
            context_table[k].lut_entry = mq_lut[context_table[k].lut_entry].lps;
        }
        A = Qe;
        do{
            renorm_once();
        }while(A < 0x8000);
    }
    return x;
}

void MQDecoder::renorm_once(){
    if(t_count == 0){
        fill_lsbs();
    }
    A = A << 1;
    C = C << 1;
    t_count--;
}

void MQDecoder::print_state(){
    cout << "A: " << hex << A;
    cout << ", C: " << hex << C;
    cout << ", Temp Byte: " << hex << (int)temp_byte;
    cout << ", t_count: " << dec << (int)t_count;
    cout << ", L: " << dec << L << "\n";
}