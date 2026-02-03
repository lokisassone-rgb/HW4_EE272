#ifndef INPUT_DOUBLE_BUFFER_H
#define INPUT_DOUBLE_BUFFER_H


template <int size, int IC0, int OC0>
class InputDoubleBufferWriter{
public:
    InputDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<INPUT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        int numberofTiles = params.OX1*params.OY1;
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX;
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY;
        int sizeofDoubleBuffer = ix0*iy0*params.IC1;

        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp;
        PackedInt<INPUT_PRECISION, 4> tempdinread;
        PackedInt<INPUT_PRECISION, IC0> tempdinwrite;
        PackedInt<INPUT_PRECISION, IC0> tempdinwrite_prev;
        PackedInt<INPUT_PRECISION, IN_PAR> temp_prev;

        bool first = true;
        for (int i = 0; i < numberofTiles; i++) {
            PackedInt<INPUT_PRECISION, IC0> temp;
            for (int j = 0; j < sizeofDoubleBuffer; j++) {
                tempdinread = din.read();
                temp.data[j] = tempdinread;
            }
            if (!first) {
                dout.write(temp_prev);
            } else {
                first = false;
            }
            temp_prev = temp;
        }
        // Write the last tile after the loop
        dout.write(temp_prev);
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class InputDoubleBufferReader{
public:
    InputDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > &din, 
                        ac_channel<PackedInt<INPUT_PRECISION, IC0> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        int numberofTiles = params.OX1*params.OY1;
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX;
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY;
        int sizeofDoubleBuffer = ix0*iy0*params.IC1;

        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp;

        for (int i = 0; i < numberofTiles; i++){
            temp = din.read();
            for (int j = 0; j < sizeofDoubleBuffer; j++){
                dout.write( temp.data[j] );
            }
        }
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class InputDoubleBuffer{
public:
  InputDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<INPUT_PRECISION, 4> > &inputs_in, 
                      ac_channel<PackedInt<INPUT_PRECISION, IC0> > &inputs_out,
                      ac_channel<Params> &paramsIn)
    {

        Params params = paramsIn.read();

        inputDoubleBufferReaderParams.write(params);
        inputDoubleBufferWriterParams.write(params);

        inputDoubleBufferWriter.run(inputDoubleBufferWriterParams, inputs_in, mem);

        inputDoubleBufferReader.run(inputDoubleBufferReaderParams, mem, inputs_out);
    }

private:
    ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > mem;
    
    InputDoubleBufferWriter<size, IC0, OC0> inputDoubleBufferWriter;
    ac_channel<Params> inputDoubleBufferWriterParams;
    
    InputDoubleBufferReader<size, IC0, OC0> inputDoubleBufferReader;
    ac_channel<Params> inputDoubleBufferReaderParams;
};

#endif
