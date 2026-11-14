// Copyright 2026 Ameen Alqershi
// Licensed under the Apache License, Version 2.0 (the "License");
using Google.FlatBuffers;

namespace Google.FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class FlatBuffersVerifierSecurityTests
    {
        // A structurally valid table whose nested byte vector starts at byte 20.
        private static byte[] BuildNestedBufferTable()
        {
            var bytes = new byte[28];
            bytes[4] = 6;  // vtable length
            bytes[6] = 8;  // table size
            bytes[8] = 4;  // nested field is four bytes into table
            bytes[12] = 8; // table's vtable lives eight bytes behind table
            bytes[16] = 4; // uoffset to vector at byte 20
            bytes[20] = 4; // four payload bytes
            return bytes;
        }

        [FlatBuffersTestMethod]
        public void ValidNestedBufferFieldIsAccepted()
        {
            var verifier = new Verifier(new ByteBuffer(BuildNestedBufferTable()));
            Assert.IsTrue(verifier.VerifyTableStart(12));
            Assert.IsTrue(verifier.VerifyNestedBuffer(12, 4, null, true));
        }

        [FlatBuffersTestMethod]
        public void OutOfBoundsNestedBufferOffsetIsRejectedWithoutThrowing()
        {
            var bytes = BuildNestedBufferTable();
            bytes[8] = 20; // field now points beyond the end of the buffer
            var verifier = new Verifier(new ByteBuffer(bytes));
            Assert.IsTrue(verifier.VerifyTableStart(12));
            Assert.IsFalse(verifier.VerifyNestedBuffer(12, 4, null, true));
        }
    }
}
