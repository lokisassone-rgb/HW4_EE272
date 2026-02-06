#ifndef SYSTOLIC_ARRAY_H
#define SYSTOLIC_ARRAY_H

#include "ProcessingElement.h"
#include "conv.h"
#include "Fifo.h"
#include "SystolicArrayCore.h"

// Include mc_scverify.h for CCS_* macros
#include <mc_scverify.h>

class SystolicArrayLooper
{
public:
    SystolicArrayLooper() {}

#pragma hls_design interface
void run(ac_channel<Params> &paramsIn,
         ac_channel<Params> &paramsOut,
         ac_channel<LoopIndices> &loopIndicesOut)
    {
        // -------------------------------
        // Generate the loop indices here for the systolic array.
        // Write the loop indices as well as the params out to channels.
        // Your code starts here
        Params params = paramsIn.read();
        for (int oy1=0; oy1 < params.OY1; oy1++){//loop through conv gold tb same way conv loop
            for (int ox1=0; ox1< params.OX1; ox1++){
                for (int oc1 = 0; oc1 < params.OC1; oc1++) {
                    for (int ic1 = 0; ic1 < params.IC1; ic1++) {
                        for (int fy = 0; fy < params.FY; fy++) {
                            for (int fx = 0; fx < params.FX; fx++) {
                                paramsOut.write(params);//write out params
                                LoopIndices loopindex; //instansiate LoopIndixes to write out
                                loopindex.ic1_idx = ic1;
                                loopindex.fx_idx = fx;
                                loopindex.fy_idx = fy;
                                loopIndicesOut.write(loopindex);
                            }
                        }
                    }
                }
            }
        }
        // Your code ends here
        // -------------------------------
    }   
};

template <typename IDTYPE, typename WDTYPE, typename ODTYPE, int OC0, int IC0>
class SystolicArrayWrapper
{
public:
    SystolicArrayWrapper(){}
    
#pragma hls_design interface
    void run(ac_channel<PackedInt<INPUT_PRECISION, IC0> > &input, 
             ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weight, 
             ac_channel<PackedInt<OUTPUT_PRECISION, OC0> > &output,
             ac_channel<Params> &paramsIn)
    {
        systolicArrayLooper.run(paramsIn, paramsChannel, loopIndicesChannel);
        systolicArrayCore.run(input, weight, output, paramsChannel, loopIndicesChannel);
    }
private:
    SystolicArrayCore<IDTYPE, WDTYPE, ODTYPE, OC0, IC0> systolicArrayCore;
    SystolicArrayLooper systolicArrayLooper;
    ac_channel<Params> paramsChannel;
    ac_channel<LoopIndices> loopIndicesChannel;
};

#endif
