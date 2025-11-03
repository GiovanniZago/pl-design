#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <iomanip>

#include "aie-generator.h"
#include "puppi-unpackers.h"

namespace aiesim {

class AIESimGenerator : public aie::AIEGenerator {
    public:
        AIESimGenerator(const std::string &fname) : AIEGenerator(fname) {};

        void dump_sim_files(const std::string &fout0, const std::string &fout1, const bool tlast = false) {
            std::ofstream out0(fout0, std::ios::out);
            std::ofstream out1(fout1, std::ios::out);

            if ((!out0) | (!out1)) throw std::runtime_error("Error in opening the out files");

            const auto& view = orbit_view(); // execute method from base class
            auto ptr = view.data(); // get pointer to begin of header block of data
            uint32_t temp = 0;

            if (tlast) {
                std::vector<uint32_t> association_map(3565, 0);
                
                // advance the pointer by 3564 elements while looping through the vector
                for (auto iter = association_map.begin() + 1; iter < association_map.end(); ++iter, ++ptr) {
                    uint16_t run, bx;
                    uint32_t orbit;
                    bool good;
                    uint32_t npuppi;

                    l1puppiUnpack::parseHeader(*ptr, run, bx, orbit, good, npuppi);

                    if (!good) throw std::runtime_error("Encountered a non-valid event header");

                    *iter = npuppi + temp;
                    temp += npuppi;
                }
            } else {
                ptr += 3564; // skip all headers

                for (; ptr < view.data() + view.size(); ptr++) {
                    uint16_t pt;
                    int16_t eta, phi;
                    uint16_t pid;
    
                    l1puppiUnpack::readall(*ptr, pt, eta, phi, pid);
    
                    out0 << std::right << std::setw(10) << pt << std::setw(10) << eta << "\n";
                    out1 << std::right << std::setw(10) << phi << std::setw(10) << pid << "\n";
    
                    ++ptr;
                }       
            }
            
            out0.close();
            out1.close();
        }
};


} // namespace aiesim