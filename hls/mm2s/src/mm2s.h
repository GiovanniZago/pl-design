#ifndef mm2s_h
#define mm2s_h

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

#define ORBIT_SIZE 3564
#define MAX_CANDS 208
// #define __CSIM__

using qdma_t = qdma_axis<32,0,0,0>;

extern "C" 
{

void mm2s(
    const uint64_t* mem, 
    hls::stream<qdma_t>& s0,
    hls::stream<qdma_t>& s1,
    hls::stream<qdma_t>& s2,
    hls::stream<qdma_t>& s3,
    hls::stream<qdma_t>& s4,
    hls::stream<qdma_t>& s5
);

}

#endif