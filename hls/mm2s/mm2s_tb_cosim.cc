#include "../../p2scout-aie-generator/aie-generator.h"
#include "src/mm2s.h"

#include <iostream>
#include <exception>
#include <iostream>

int main() {
    std::cout << "[TB] Starting mm2s TB" << std::endl;

    try {
        aie::AIEGenerator gen("/home/gizago/pl-design/p2scout-aie-generator/raws/puppi_WTo3Pion_PU200_orbit1.raw");
        const auto& view = gen.get_orbit_view();
        const auto& map = gen.get_association_map();
    
        hls::stream<qdma_axis<32,0,0,0>> s0, s1, s2, s3, s4, s5;

        std::cout << "[TB] Calling mm2s, mem size = " << view.size() << std::endl;

        // use static memory because dynamic memory is not allowed
        static uint64_t mem[200000] = { 0u };
        std::memcpy(mem, view.data(), view.size() * sizeof(uint64_t));
    
        mm2s(mem, s0, s1, s2, s3, s4, s5);
        
        std::cout << "[TB] mm2s returned, exiting TB with 0" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[TB] EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[TB] UNKNOWN EXCEPTION: " << std::endl;
        return 1;
    }
}