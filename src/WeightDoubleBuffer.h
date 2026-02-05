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
        int numberofTiles = params.OX1 * params.OY1 * params.OC1;
        int sizeofDoubleBuffer = int(params.FX) * int(params.FY) * int(params.IC1) * IC0;
        
        chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> temp;
        PackedInt<WEIGHT_PRECISION, 4> tempdinread;

        for (int i = 0; i < numberofTiles; i++){
            for (int j = 0; j < sizeofDoubleBuffer; j++){
                for (int k = 0; k < OC0/4; k++){
                    tempdinread = din.read();
                    temp.data[j].value[k*4] = tempdinread.value[0];
                    temp.data[j].value[k*4+1] = tempdinread.value[1];
                    temp.data[j].value[k*4+2] = tempdinread.value[2];
                    temp.data[j].value[k*4+3] = tempdinread.value[3];
                }
            }
            dout.write(temp);
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
        int numberofTiles = params.OX1 * params.OY1 * params.OC1;
        int sizeofDoubleBuffer = int(params.FX) * int(params.FY) * int(params.IC1) * IC0;

        chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> temp;
        PackedInt<WEIGHT_PRECISION, OC0> tempdoutwrite;


        for (int i = 0; i < numberofTiles; i++){
            temp = din.read();
            for (int j = 0; j < sizeofDoubleBuffer; j++){
                tempdoutwrite = temp.data[j];
                dout.write (tempdoutwrite);
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
