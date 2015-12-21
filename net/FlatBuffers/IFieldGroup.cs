using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers 
{
    public interface IFieldGroup 
    {
        ByteBuffer ByteBuffer { get; }
    }

    public static class FieldGroup 
    {
        public static ByteBuffer GetByteBuffer<TFldGrp>(TFldGrp fieldGroup) where TFldGrp : IFieldGroup 
        {
            return fieldGroup.ByteBuffer;
        }
    }
}
