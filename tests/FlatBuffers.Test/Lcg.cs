namespace FlatBuffers.Test
{
    /// <summary>
    /// Lcg Pseudo RNG
    /// </summary>
    internal sealed class Lcg
    {
        private const uint InitialValue = 10000;
        private uint _state;

        public Lcg()
        {
            _state = InitialValue;
        }

        public uint Next()
        {
            return (_state = 69069 * _state + 362437);
        }

        public void Reset()
        {
            _state = InitialValue;
        }
    }
}