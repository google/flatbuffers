namespace FlatBuffers.Test
{
    /// <summary>
    /// A test Table object that gives easy access to the slot data
    /// </summary>
    internal class TestTable : Table
    {
        public TestTable(ByteBuffer bb, int pos)
        {
            base.bb = bb;
            base.bb_pos = pos;
        }

        public bool GetSlot(int slot, bool def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetSbyte(bb_pos + off) != 0;
        }

        public sbyte GetSlot(int slot, sbyte def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetSbyte(bb_pos + off);
        }

        public byte GetSlot(int slot, byte def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.Get(bb_pos + off);
        }

        public short GetSlot(int slot, short def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetShort(bb_pos + off);
        }

        public ushort GetSlot(int slot, ushort def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetUshort(bb_pos + off);
        }

        public int GetSlot(int slot, int def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetInt(bb_pos + off);
        }

        public uint GetSlot(int slot, uint def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetUint(bb_pos + off);
        }

        public long GetSlot(int slot, long def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetLong(bb_pos + off);
        }

        public ulong GetSlot(int slot, ulong def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetUlong(bb_pos + off);
        }

        public float GetSlot(int slot, float def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetFloat(bb_pos + off);
        }

        public double GetSlot(int slot, double def)
        {
            var off = base.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return bb.GetDouble(bb_pos + off);
        }
    }
}