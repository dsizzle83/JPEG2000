#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "../include/my_struct.h"
#include "../include/dwt.h"
#include "../include/quantizer.h"
#include "../include/tiling.h"
#include "../include/code_block.h"

using namespace std;

// Utility function to trim whitespace (optional)
std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Function to parse a point from a string in the format "(x, y)"
point parsePoint(const std::string &line) {
    point p;
    size_t start = line.find('(');
    size_t comma = line.find(',', start);
    size_t end = line.find(')', comma);
    if (start != std::string::npos && comma != std::string::npos && end != std::string::npos) {
        p.x = std::stoi(line.substr(start + 1, comma - start - 1));
        p.y = std::stoi(line.substr(comma + 1, end - comma - 1));
    }
    return p;
}

// Function that reads one codeblock from the file stream.
bool parseCodeBlock(std::ifstream &in, code_block &cb) {
    std::string line;
    
    // First, skip lines until you reach a line starting with "CodeBlock".
    while (std::getline(in, line)) {
        if (line.find("CodeBlock") != std::string::npos)
            break;
    }
    if (in.eof()) return false;
    
    // Read Anchor
    if (!std::getline(in, line)) return false;
    if (line.find("Anchor:") != std::string::npos) {
        cb.anchor = parsePoint(line);
    }
    
    // Read Subband Type
    if (!std::getline(in, line)) return false;
    if (line.find("Subband Type:") != std::string::npos) {
        size_t pos = line.find(":");
        cb.s_b.type = std::stoi(line.substr(pos + 1));
    }
    
    // Read Subband Top Left
    if (!std::getline(in, line)) return false;
    if (line.find("Subband Top Left:") != std::string::npos) {
        cb.s_b.top_l = parsePoint(line);
    }
    
    // Read Subband Bottom Right
    if (!std::getline(in, line)) return false;
    if (line.find("Subband Bottom Right:") != std::string::npos) {
        cb.s_b.bot_r = parsePoint(line);
    }
    
    // Read Step Size
    if (!std::getline(in, line)) return false;
    if (line.find("Step Size:") != std::string::npos) {
        size_t pos = line.find(":");
        cb.s_b.step_size = std::stod(line.substr(pos + 1));
    }
    
    // Read Number of bits
    if (!std::getline(in, line)) return false;
    if (line.find("Number of bits:") != std::string::npos) {
        size_t pos = line.find(":");
        cb.num_bits = std::stoi(line.substr(pos + 1));
    }
    
    // Read "Sign Data:" header
    if (!std::getline(in, line)) return false;
    // Optionally check that the line contains "Sign Data:"
    
    // Now read the sign data matrix.
    std::vector<std::vector<bool>> signData;
    while (std::getline(in, line)) {
        line = trim(line);
        // Break if the line is empty or if it's a delimiter line.
        if (line.empty() || line.find("----------------") != std::string::npos)
            break;
        std::vector<bool> row;
        std::istringstream iss(line);
        std::string token;
        // Each token is expected to be "1" or "0"
        while (iss >> token) {
            row.push_back(token == "1");
        }
        signData.push_back(row);
    }
    cb.sign_data = signData;

    // Now read the bit plane matrices
    for(int i=0; i<cb.num_bits; i++){
        bit_plane bp;
        vector<vector<bool>> bp_data;
        while (std::getline(in, line)) {
            if (line.find("Bitplane:") != std::string::npos) {
                size_t pos = line.find(":");
                bp.bit_level = std::stoi(line.substr(pos + 1));
            }
            line = trim(line);
            // Break if the line is empty or if it's a delimiter line.
            if (line.empty() || line.find("----------------") != std::string::npos)
                break;
            std::vector<bool> row;
            std::istringstream iss(line);
            std::string token;
            // Each token is expected to be "1" or "0"
            while (iss >> token) {
                row.push_back(token == "1");
            }
            bp_data.push_back(row);
        }
        bp.plane_data = bp_data;
        cb.bit_planes.push_back(bp);
    }

    return true;
}


int main() {
    std::ifstream infile("Codeblocks.txt");
    if (!infile) {
        std::cerr << "Error opening file!" << std::endl;
        return EXIT_FAILURE;
    }

    // Optionally, you could first read tile information (if needed)
    std::string tileLine;
    std::getline(infile, tileLine);  // e.g., "Tile Anchor: (0, 0)"
    std::getline(infile, tileLine);  // e.g., "Tile Dimensions: 512 rows x 512 columns"

    std::vector<code_block> codeblocks;
    code_block cb;
    while (parseCodeBlock(infile, cb)) {
        codeblocks.push_back(cb);
    }

    std::cout << "Read " << codeblocks.size() << " code blocks." << std::endl;

    // Debug: print out the first code block's anchor and number of bits if available
    if (!codeblocks.empty()) {
        std::cout << "First CodeBlock Anchor: (" << codeblocks[0].anchor.x << ", " 
                  << codeblocks[0].anchor.y << ")" << std::endl;
        std::cout << "Number of bits: " << codeblocks[0].num_bits << std::endl;
    }

    return 0;
}