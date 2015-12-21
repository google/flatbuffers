using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers 
{
    public interface IStruct : IFieldGroup
    {
        StructPos StructPos { get; }
    }

    public static class Struct 
    {
        public static StructPos GetStructPos<TStruct>(TStruct structure) where TStruct : IStruct 
        {
            return structure.StructPos;
        }
    }
}
