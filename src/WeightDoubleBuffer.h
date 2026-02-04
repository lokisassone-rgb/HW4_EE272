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
        Params params = paramsIn.read();

        for (int oy1 = 0; oy1 < params.OY1; oy1++){
            for (int ox1 = 0; ox1 < params.OX1; ox1++){
                for (int oc1 = 0; oc1 < params.OC1; oc1++){
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> tile;
                    PackedInt<WEIGHT_PRECISION, 4> input;
                
                    //define size of a weight tile (IC1*IC0*FX*FY)
                    int numberofWeightsPerTile = int(params.IC1) * IC0 * int(params.FX) * int(params.FY);
                    for (int i = 0; i < numberofWeightsPerTile; i++) {
                        for (int weight_index = 0; weight_index < OC0/4; weight_index++){
                            input = din.read();
                            for (int j = 0; j < 4; j++) {
                                tile.data[i].value[weight_index*4+j] = input.value[j];
                                    }
                                }
                            }
                        }
                    }
                    dout.write(tile);
                }
            }
        };
        // Your code ends here
        // -------------------------------

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
        Params params = paramsIn.read();

        for (int oy1 = 0; oy1 < params.OY1; oy1++){
            for (int ox1 = 0; ox1 < params.OX1; ox1++){
                for (int oc1 = 0; oc1 < params.OC1; oc1++){
                    
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> tmp = din.read();
                    int numberofWeightsPerTile = int(params.IC1) * IC0 * int(params.FX) * int(params.FY);
                    for (int i = 0; i < numberofWeightsPerTile; i++){
                        dout.write(tmp.data[i]);
                            }
                        }
                    }
                }
            }
        };
        // Your code ends here
        // -------------------------------

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
    #include <cstdio>
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif
