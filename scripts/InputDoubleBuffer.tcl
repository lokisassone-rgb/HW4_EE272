set blockname [file rootname [file tail [info script] ]]

source scripts/common.tcl

directive set -DESIGN_HIERARCHY " 
    {InputDoubleBuffer<4096, ${ARRAY_DIMENSION}, ${ARRAY_DIMENSION}>} 
"

go compile

source scripts/set_libraries.tcl

go libraries
directive set -CLOCKS $clocks

go assembly

# -------------------------------
# Set the correct word widths and the stage replication
# Your code starts here
set word_width [expr ${ARRAY_DIMENSION} * 8]
set block_size ${ARRAY_DIMENSION}

directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/mem:cns -STAGE_REPLICATION 2
directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/inputDoubleBufferWriter/tmp -BLOCK_SIZE $block_size
directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/inputDoubleBufferReader/tmp -BLOCK_SIZE $block_size

directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/inputDoubleBufferReader/din -WORD_WIDTH $word_width
directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/inputDoubleBufferWriter/dout -WORD_WIDTH $word_width
directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/mem -WORD_WIDTH $word_width
directive set /InputDoubleBuffer<4096,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/.../temp.data.value -match glob -WORD_WIDTH $word_width

# Your code ends here
# -------------------------------

go architect

go allocate
go extract
