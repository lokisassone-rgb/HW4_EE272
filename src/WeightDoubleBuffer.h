#ifndef WEIGHT_DOUBLE_BUFFER_H
#define WEIGHT_DOUBLE_BUFFER_H


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

        for (uint_16 oy1_idx = 0; oy1_idx < params.OY1; oy1_idx++) {
            for (uint_16 ox1_idx = 0; ox1_idx < params.OX1; ox1_idx++) {
                for (uint_16 oc1_idx = 0; oc1_idx < params.OC1; oc1_idx++) {

                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> tile;
                    PackedInt<WEIGHT_PRECISION, 4> inputItem;

                    uint_16 numTileItems = params.IC1 * params.FY * params.FX * IC0;
                    for (int tileItemIdx = 0; tileItemIdx < numTileItems; tileItemIdx++) {
                        for (int inputItemIdx = 0; inputItemIdx < OC0/4; inputItemIdx++) { // assume OC0 is a power of two >= 4
                            inputItem = din.read(); // read next 4-packed input item
                            for (int i = 0; i < 4; i++) {
                                tile.data[tileItemIdx].value[4*inputItemIdx + i] = inputItem.value[i];
                            }
                        }
                    }

                    dout.write(tile); // write tile (infers double buffer)
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

        for (uint_16 oy1_idx = 0; oy1_idx < params.OY1; oy1_idx++) {
            for (uint_16 ox1_idx = 0; ox1_idx < params.OX1; ox1_idx++) {
                for (uint_16 oc1_idx = 0; oc1_idx < params.OC1; oc1_idx++) {

                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> tile = din.read(); // read tile (infers double buffer)

                    uint_16 numTileItems = params.IC1 * params.FY * params.FX * IC0;
                    for (int adr = 0; adr < numTileItems; adr++) {
                        dout.write(tile.data[adr]); // write weights in sequential order for the systolic array to digest
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
