using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers 
{
    public struct StructPos 
    {
        public StructPos(int bb_pos, ByteBuffer bb) 
        {
            this.bb_pos = bb_pos;
            this.bb = bb;
        }

        public readonly int bb_pos;
        public readonly ByteBuffer bb;
    }
}
