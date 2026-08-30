/*
 * Copyright 2026 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;

namespace Google.FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class FlatBufferVerifyIdentifierTests
    {
        // The smallest real table verify action: exactly what generated code
        // emits for a table without fields.
        private static bool VerifyEmptyTable(Verifier verifier, uint tablePos)
        {
            return verifier.VerifyTableStart(tablePos) && verifier.VerifyTableEnd(tablePos);
        }

        private static byte[] BuildBuffer(string identifier)
        {
            var fbb = new FlatBufferBuilder(64);
            fbb.StartTable(0);
            int root = fbb.EndTable();
            if (identifier == null)
            {
                fbb.Finish(root);
            }
            else
            {
                fbb.Finish(root, identifier);
            }
            return fbb.SizedByteArray();
        }

        private static bool Verify(byte[] buffer, string identifier)
        {
            var verifier = new Verifier(new ByteBuffer(buffer));
            return verifier.VerifyBuffer(identifier, false, VerifyEmptyTable);
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferAcceptsMatchingIdentifier()
        {
            Assert.IsTrue(Verify(BuildBuffer("NULL"), "NULL"));
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferRejectsMismatchedIdentifier()
        {
            Assert.IsFalse(Verify(BuildBuffer("NULL"), "ZZZZ"));
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferRejectsMissingIdentifier()
        {
            Assert.IsFalse(Verify(BuildBuffer(null), "NULL"));
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferSkipsCheckForEmptyIdentifier()
        {
            // Documented contract in FlatBufferVerify.cs: "When empty identifier
            // is provided the identifier validation is skipped." flatc emits
            // VerifyBuffer("", ...) for schemas that declare no file_identifier.
            Assert.IsTrue(Verify(BuildBuffer(null), ""));
            Assert.IsTrue(Verify(BuildBuffer("NULL"), ""));
        }
    }
}
