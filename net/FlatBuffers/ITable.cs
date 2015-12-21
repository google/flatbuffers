using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers 
{
    public interface ITable : IFieldGroup
    {
        TablePos TablePos { get; }
    }

    public static class Table 
    {
        public static TablePos GetTablePos<TTable>(TTable table) where TTable : ITable 
        {
            return table.TablePos;
        }
    }
}
