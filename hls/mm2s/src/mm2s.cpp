#include "mm2s.h"

void mm2s(
    const uint64_t* mem, 
    hls::stream<qdma_t>& s0,
    hls::stream<qdma_t>& s1,
    hls::stream<qdma_t>& s2,
    hls::stream<qdma_t>& s3,
    hls::stream<qdma_t>& s4,
    hls::stream<qdma_t>& s5
) {
    #pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem 
    #pragma HLS INTERFACE axis port=s0
    #pragma HLS INTERFACE axis port=s1
    #pragma HLS INTERFACE axis port=s2
    #pragma HLS INTERFACE axis port=s3
    #pragma HLS INTERFACE axis port=s4
    #pragma HLS INTERFACE axis port=s5

    #pragma HLS INTERFACE s_axilite port=mem bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    ap_uint<64> headers[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++) // burst memory access
    {
        headers[ii] = ap_uint<64>(mem[ii]);
    }

    ap_uint<12> bx       [ORBIT_SIZE];
    ap_uint<32> orbit    [ORBIT_SIZE]; // probably not necessary to unpack the orbit for each header
    ap_uint<12> num_cands[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        #pragma HLS UNROLL

        bx[ii]        = headers[ii].range(23, 12);
        orbit[ii]     = headers[ii].range(55, 24);
        num_cands[ii] = headers[ii].range(11, 0);
    }

    unsigned int event_base = 0u;
    unsigned int stream_port_index = 0u;

    ap_uint<64> words[MAX_CANDS];
    ap_uint<16> pt   [MAX_CANDS];
    ap_int<16>  eta  [MAX_CANDS];
    ap_int<16>  phi  [MAX_CANDS];
    ap_uint<16> pid  [MAX_CANDS];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        for (unsigned int jj=0; jj<num_cands[ii]; jj++) // burst memory access
        {
            words[jj] = ap_uint<64>(mem[ORBIT_SIZE + event_base + jj]);
        }

        for (unsigned int jj=0; jj<num_cands[ii]; jj++)
        {
            #pragma HLS PIPELINE II=1

            pt[jj]  = words[jj].range(13, 0); // pt is 14-bit wide

            eta[jj] = words[jj].range(25, 14); // eta is 12-bit wide
            eta[jj] = eta[jj] | ((eta[jj][11]) ? 0xF000 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            phi[jj] = words[jj].range(36, 26); // phi is 11-bit wide
            phi[jj] = phi[jj] << 5; // rescale phi to 16 bits, no need to pad any bit
            // phi[jj] = phi[jj] | ((phi[jj][10]) ? 0xF800 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            pid[jj] = words[jj].range(39, 37); // pid is 2-bit wide
        }

        qdma_t bx_cands_header, orbit_header;
        bx_cands_header.data.range(15, 0)  = bx[ii] & 0x0FFF; // first write bx number
        bx_cands_header.data.range(31, 16) = num_cands[ii] & 0x0FFF; // then write number of candidates
        orbit_header.data.range(31, 0)    = orbit[ii]; // on the other stream, write only the orbit, using the full 32b

        switch (stream_port_index) 
        {
            case 0u: s0.write(bx_cands_header); s1.write(orbit_header); break;
            case 1u: s2.write(bx_cands_header); s3.write(orbit_header); break;
            default: s4.write(bx_cands_header); s5.write(orbit_header); break;
        }
        
        qdma_t x_pt, x_eta, x_phi, x_pid;
        
        for (unsigned int jj=0; jj<num_cands[ii]; jj+=2)
        {
            #pragma HLS PIPELINE II=1

            const bool pair = (jj + 1 < num_cands[ii]);
            const bool last = (jj + 2 >= num_cands[ii]);
            const ap_uint<4> keep = pair ? ap_uint<4>(0xF) : ap_uint<4>(0x3); // each bit says whether a byte has to be kept or not
            
            x_pt.data.range(15, 0) = pt[jj];
            x_pt.data.range(31,16) = pair ? pt[jj + 1] : ap_uint<16>(0);

            x_pid.data.range(15, 0) = pid[jj];
            x_pid.data.range(31,16) = pair ? pid[jj + 1] : ap_uint<16>(0);

            x_pt.set_last(last);
            x_pid.set_last(last);
            x_pt.set_keep(keep);
            x_pid.set_keep(keep);

            switch (stream_port_index) 
            {
                case 0u: s0.write(x_pt); s1.write(x_pid); break;
                case 1u: s2.write(x_pt); s3.write(x_pid); break;
                default: s4.write(x_pt); s5.write(x_pid); break;
            }
        }

        for (unsigned int jj=0; jj<num_cands[ii]; jj+=2)
        {
            #pragma HLS PIPELINE II=1

            bool pair = (jj + 1 < num_cands[ii]);
            const bool last = (jj + 2 >= num_cands[ii]);
            const ap_uint<4> keep = pair ? ap_uint<4>(0xF) : ap_uint<4>(0x3);

            x_eta.data.range(15, 0) = eta[jj];
            x_eta.data.range(31,16) = pair ? eta[jj + 1] : ap_int<16>(0);

            x_phi.data.range(15, 0) = phi[jj];
            x_phi.data.range(31,16) = pair ? phi[jj + 1] : ap_int<16>(0);

            x_eta.set_last(last);
            x_eta.set_last(keep);
            x_phi.set_keep(last);
            x_phi.set_keep(keep);

            switch (stream_port_index) 
            {
                case 0u: s0.write(x_eta); s1.write(x_phi); break;
                case 1u: s2.write(x_eta); s3.write(x_phi); break;
                default: s4.write(x_eta); s5.write(x_phi); break;
            }
        }

        event_base += num_cands[ii];
        stream_port_index = (stream_port_index + 1) % 3;
    }
}