#include <adf.h>
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

static const int VEC_SIZE = 32;
static const int MAX_VECTS = 7;
static const int MAX_CANDS = VEC_SIZE * MAX_VECTS;

using namespace adf;

void stream_mux(input_stream<int32> * __restrict in0, 
                input_stream<int32> * __restrict in1, 
                output_stream<int16> * __restrict out);