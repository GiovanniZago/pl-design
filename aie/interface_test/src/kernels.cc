#include "kernels.h"

void interface_test(input_stream<int16> * __restrict in0, input_stream<int16> * __restrict in1, output_stream<int32> * __restrict out) {
    // create buffers to store values from stream
    const unsigned int MAX_CANDS = 208;
    alignas(aie::vector_decl_align) int16 buf0[MAX_CANDS] = { 0 };
    alignas(aie::vector_decl_align) int16 buf1[MAX_CANDS] = { 0 };

    // create instance of tlast struct, so we can use it to read values from the stream (3rd >> overloading on the documentation)
    aie::tlast<int16, bool> s0_handler{0, false};
    aie::tlast<int16, bool> s1_handler{0, false};
    
    unsigned int ii = 0;

    // read pt and eta
    while ((!s0_handler.t_last) & (!s1_handler.t_last))
    {   
        // read pt
        s0 >> s0_handler;
        buf0[ii] = s0_handler.value;

        // read eta
        s1 >> s1_handler;
        buf1[ii] = s1_handler.value;

        ++ii;
    }

    div_t result_a = div(ii, (unsigned int) 32);
    
    if (result_a.quot == 0) {
        const int NUM_VECT_A = 1;
    } else {
        const int NUM_VECT_A = (result_a.rem == 0) ? result_a.quot : result_a.quot + 1;
    }

    aie::vector<int16, 32> pt[NUM_VECT_A] = { aie::broadcast<int16, 32>(0) };
    aie::vector<int16, 32> eta[NUM_VECT_A] = { aie::broadcast<int16, 32>(0) };

    aie::vector<int16, 32> *vec_ptr0 = (aie::vector<int16, 32> *) buf0; 
    aie::vector<int16, 32> *vec_ptr1 = (aie::vector<int16, 32> *) buf1; 

    for (unsigned int vidx = 0; vidx < NUM_VECT_A; vidx++) {
        pt[vidx].load(vec_ptr0);
        eta[vidx].load(vec_ptr1);
        ++vec_ptr0;
        ++vec_ptr1;
    }

    // restore variables back to initial position
    memset(buf0, (int16) 0, MAX_CANDS * sizeof(buf0[0]))
    memset(buf1, (int16) 0, MAX_CANDS * sizeof(buf1[0]))
    s0_handler{0, false};
    s1_handler{0, false};
    ii = 0;
    vec_ptr0 -= NUM_VECT_A;
    vec_ptr1 -= NUM_VECT_A;

    // read phi and pid
    while ((!s0_handler.t_last) & (!s1_handler.t_last))
    {   
        // read pt
        s0 >> s0_handler;
        buf0[ii] = s0_handler.value;

        // read eta
        s1 >> s1_handler;
        buf1[ii] = s1_handler.value;

        ++ii;
    }

    div_t result_b = div(ii, (unsigned int) 32);
    
    if (result_b.quot == 0) {
        const int NUM_VECT_B = 1;
    } else {
        const int NUM_VECT_B = (result_b.rem == 0) ? result_b.quot : result_b.quot + 1;
    }

    // check consistency between the two streams
    if (NUM_VECT_A != NUM_VECT_B) {
        throw std::runtime_error("The number of int16 values read for pt, eta and phi, pid should be the same, but it is not.\n");
    }

    aie::vector<int16, 32> phi[NUM_VECT_B] = { aie::broadcast<int16, 32>(0) };
    aie::vector<int16, 32> pid[NUM_VECT_B] = { aie::broadcast<int16, 32>(0) };

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        phi[vidx].load(vec_ptr0);
        pid[vidx].load(vec_ptr1);
        ++vec_ptr0;
        ++vec_ptr1;
    }

    // data loopback
    for (unsigned int vidx = 0; vidx < NUM_VECT_A; vidx++) {
        writeincr(out, pt[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_A; vidx++) {
        writeincr(out, eta[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        writeincr(out, phi[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        writeincr(out, pid[vidx]);
    }
}