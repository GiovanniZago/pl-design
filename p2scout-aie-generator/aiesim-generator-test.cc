#include "aiesim-generator.h"

#include <iostream>

int main() {
    aiesim::AIESimGenerator gen("raws/puppi_WTo3Pion_PU200_orbit1.raw");
    gen.dump_sim_files("aiesim/data0.txt", "aiesim/data1.txt");
}