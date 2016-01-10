namespace FlatBuffers.Test
{
    /// <summary>
    /// A test Table object that gives easy access to the slot data
    /// </summary>
    internal class TestTable
    {
        private TablePos pos;

        public TestTable(ByteBuffer bb, int pos)
        {
            this.pos = new TablePos(pos, bb);
        }

        public bool GetSlot(int slot, bool def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetSbyte(this.pos.bb_pos + off) != 0;
        }

        public sbyte GetSlot(int slot, sbyte def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetSbyte(this.pos.bb_pos + off);
        }

        public byte GetSlot(int slot, byte def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.Get(this.pos.bb_pos + off);
        }

        public short GetSlot(int slot, short def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetShort(this.pos.bb_pos + off);
        }

        public ushort GetSlot(int slot, ushort def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetUshort(this.pos.bb_pos + off);
        }

        public int GetSlot(int slot, int def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetInt(this.pos.bb_pos + off);
        }

        public uint GetSlot(int slot, uint def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetUint(this.pos.bb_pos + off);
        }

        public long GetSlot(int slot, long def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetLong(this.pos.bb_pos + off);
        }

        public ulong GetSlot(int slot, ulong def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetUlong(this.pos.bb_pos + off);
        }

        public float GetSlot(int slot, float def)
        {
          var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetFloat(this.pos.bb_pos + off);
        }

        public double GetSlot(int slot, double def)
        {
            var off = this.pos.__offset(slot);

            if (off == 0)
            {
                return def;
            }
            return this.pos.bb.GetDouble(this.pos.bb_pos + off);
        }
    }
}