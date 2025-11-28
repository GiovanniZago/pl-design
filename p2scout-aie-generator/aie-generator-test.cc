#include "aie-generator.h"

#include <iostream>

int main() {
    aie::AIEGenerator gen("/home/gizago/pl-design/p2scout-aie-generator/raws/puppi_WTo3Pion_PU200_orbit1.raw");
    const auto& view = gen.get_orbit_view();
    const auto* begin_view = view.data();
    const auto* end_view = view.data() + view.size();

    auto ptr = begin_view;

    for (unsigned int i = 0; i < 3564; ++i) {
        uint16_t run, bx; 
        uint32_t orbit, npuppi;
        bool good;

        l1puppiUnpack::parseHeader(*ptr, run, bx, orbit, good, npuppi);

        std::cout << "Run " << run
            << " Orbit " << orbit 
            << " Bx " << bx 
            << " Npuppi " << npuppi 
            << " Good " << good 
            << std::endl;

        ++ptr;
    }

    while (ptr < end_view) {
        uint16_t pt;
        int16_t eta, phi;

        l1puppiUnpack::readshared(*ptr, pt, eta, phi);

        std::cout << "Pt " << pt
            << " Eta " << eta 
            << " Phi " << phi 
            << std::endl;
    
        ++ptr;
    }
}