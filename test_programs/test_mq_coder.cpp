#include "../include/mq_coder.h"
using namespace std;

int main(){
    // Define a vector of input symbols.
    // For clarity, we use std::vector<bool> for a sequence of bits.
    bool input_symbols[84] = {
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true, 
        false, true, false, true, false, true
    };

    // Instantiate the encoder.
    MQEncoder encoder;
    encoder.print_state();
    
    // Encode each input symbol using a specific context index (here, we use context 0).
    for (int i=0; i<42; i++) {
        encoder.encode(input_symbols[i], 5);
        encoder.print_state();
    }
    encoder.easy_term();
    
    // Retrieve the encoded codeword.
    std::vector<uint8_t> encoded_codeword = encoder.get_codeword();
    
    // (Optional) Print out the encoded codeword in hexadecimal.
    std::cout << "Encoded codeword: ";
    for (auto byte : encoded_codeword) {
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::dec << std::endl; // Reset to decimal output

    // Instantiate the decoder with the encoded codeword.
    MQDecoder decoder(encoded_codeword);
    
    // Decode each symbol and collect the decoded bits.
    std::vector<bool> decoded_symbols;
    for (size_t i = 0; i < 42; i++) {
        bool bit = decoder.decode(0);
        decoder.print_state();
        //cout << bit;
        decoded_symbols.push_back(bit);
    }
    cout << endl;    

    // Compare the decoded symbols with the original input.
    bool success = true;
    for (size_t i = 0; i < 42; i++) {
        cout << "Encoded Symbol[" << i << "]: " << (int)input_symbols[i] << ", Decoded Symbol[" << i << "]: " << (int)decoded_symbols[i] << "\n";
        if (input_symbols[i] != decoded_symbols[i]) {
            success = false;
            //break;
        }
    }

    std::cout << "Decoding " << (success ? "successful" : "failed") << std::endl;

    encoder.reset_encoder();
    // Encode each input symbol using a specific context index (here, we use context 0).
    for (int i=0; i<42; i++) {
        encoder.encode(input_symbols[i], 0);
        encoder.print_state();
    }
    encoder.easy_term();

    encoded_codeword = encoder.get_codeword();

    
    decoder.load_codeword(encoded_codeword);
    decoder.reset_decoder();

    decoded_symbols.clear();
    for (size_t i = 0; i < 42; i++) {
        bool bit = decoder.decode(0);
        decoder.print_state();
        //cout << bit;
        decoded_symbols.push_back(bit);
    }
    cout << endl;

    for (size_t i = 0; i < 42; i++) {
        cout << "Encoded Symbol[" << i << "]: " << (int)input_symbols[i] << ", Decoded Symbol[" << i << "]: " << (int)decoded_symbols[i] << "\n";
        if (input_symbols[i] != decoded_symbols[i]) {
            success = false;
            //break;
        }
    }
    
    
    std::cout << "Decoding " << (success ? "successful" : "failed") << std::endl;
    return 0;
}