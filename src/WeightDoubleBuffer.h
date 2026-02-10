#ifndef WEIGHT_DOUBLE_BUFFER_H
#define WEIGHT_DOUBLE_BUFFER_H

#include "conv.h"


template <int size, int IC0, int OC0>
class WeightDoubleBufferWriter{
public:
    WeightDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> > &dout)
    {
        // -------------------------------
        // Your code starts here

        Params params = paramsIn.read(); // read params
        #pragma hls_pipeline_init_interval 1
        for (int oy1 = 0; oy1 < OY1_MAX; oy1++) {//oy and ox number of tiles
            if (oy1 >= params.OY1) { break; }
            for (int ox1 = 0; ox1 < OX1_MAX; ox1++) {
                if (ox1 >= params.OX1) { break; }
                for (int oc1 = 0; oc1 < OC1_MAX; oc1++) {
                    if (oc1 >= params.OC1) { break; }

                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> temp; //create a new tile
                    PackedInt<WEIGHT_PRECISION, 4> inputItem;
                    PackedInt<WEIGHT_PRECISION, OC0> tempdinwrite;

                    int numTileItems = int(params.IC1) * int(params.FY) * int(params.FX) * IC0;
                    for (int tileItemIdx = 0; tileItemIdx < size; tileItemIdx++) {
                        if (tileItemIdx >= numTileItems) { break; }
                        for (int inputItemIdx = 0; inputItemIdx < OC0/4; inputItemIdx++) {
                            inputItem = din.read();
                            tempdinwrite.value[4*inputItemIdx] = inputItem.value[0];
                            tempdinwrite.value[4*inputItemIdx + 1] = inputItem.value[1];
                            tempdinwrite.value[4*inputItemIdx + 2] = inputItem.value[2];
                            tempdinwrite.value[4*inputItemIdx + 3] = inputItem.value[3];
                        }
                        temp.data[tileItemIdx] = tempdinwrite;
                    }

                    dout.write(temp); 
                }
            }
        }

        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class WeightDoubleBufferReader{
public:
    WeightDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > &din, 
                        ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &dout)
    {
        // -------------------------------
        // Your code starts here

        Params params = paramsIn.read(); // read params
        #pragma hls_pipeline_init_interval 1
        for (int oy1 = 0; oy1 < OY1_MAX; oy1++) {
            if (oy1 >= params.OY1) { break; }
            for (int ox1 = 0; ox1 < OX1_MAX; ox1++) {
                if (ox1 >= params.OX1) { break; }
                for (int oc1 = 0; oc1 < OC1_MAX; oc1++) {
                    if (oc1 >= params.OC1) { break; }

                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> temp = din.read();

                    int numTileItems = int(params.IC1) * int(params.FY) * int(params.FX) * IC0; //number of calculations per pixel
                  //  #pragma hls_pipeline_init_interval 1
                    for (int adr = 0; adr < size; adr++) {
                        if (adr >= numTileItems) { break; }
                        dout.write(temp.data[adr]); // write tile item by item into address
                    }
                }
            }
        }

        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class WeightDoubleBuffer{
public:
  WeightDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &weights_in, 
                      ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weights_out,
                      ac_channel<Params> &paramsIn)
    {
        Params params = paramsIn.read();

        // #ifndef __SYNTHESIS__
        // ac_int<ac::log2_ceil<size>::val, false> block_size = IC0*params.IC1*params.FX*params.FY;
        // assert(block_size <= size);
        // #endif

        weightDoubleBufferReaderParams.write(params);
        weightDoubleBufferWriterParams.write(params);

        weightDoubleBufferWriter.run(weightDoubleBufferWriterParams, weights_in, mem);
        weightDoubleBufferReader.run(weightDoubleBufferReaderParams, mem, weights_out);
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif