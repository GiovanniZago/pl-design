#include "aie-generator.h"

#include <iostream>

int main() {
    aie::AIEGenerator gen("raws/puppi_WTo3Pion_PU200_orbit1.raw"); // the file has been generated using p2scout-generator with Native64 option
    gen.dump_aiesim_files("aiesim/data0.txt", "aiesim/data1.txt", 24, 28);
}