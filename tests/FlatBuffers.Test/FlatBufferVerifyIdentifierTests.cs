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
using MyGame.Example;

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

        // A Monster whose testnestedflatbuffer field holds a finished Monster
        // buffer: the layout for which generated code calls VerifyNestedBuffer.
        private static byte[] BuildMonsterWithNestedBuffer(bool corruptInner)
        {
            var innerBuilder = new FlatBufferBuilder(64);
            var innerName = innerBuilder.CreateString("inner");
            Monster.StartMonster(innerBuilder);
            Monster.AddName(innerBuilder, innerName);
            Monster.AddHp(innerBuilder, 100);
            Monster.FinishMonsterBuffer(innerBuilder, Monster.EndMonster(innerBuilder));
            var innerBytes = innerBuilder.SizedByteArray();
            if (corruptInner)
            {
                // Overwrite the length of the inner name string.
                innerBytes[innerBytes.Length - innerName.Value] = 0xFF;
            }

            var outerBuilder = new FlatBufferBuilder(128);
            var outerName = outerBuilder.CreateString("outer");
            var nested = Monster.CreateTestnestedflatbufferVector(outerBuilder, innerBytes);
            Monster.StartMonster(outerBuilder);
            Monster.AddName(outerBuilder, outerName);
            Monster.AddTestnestedflatbuffer(outerBuilder, nested);
            Monster.FinishMonsterBuffer(outerBuilder, Monster.EndMonster(outerBuilder));
            return outerBuilder.SizedByteArray();
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferAcceptsNestedFlatBuffer()
        {
            // VerifyNestedBuffer has no identifier for the nested buffer and
            // passes "", so the identifier check must be skipped for it.
            Assert.IsTrue(Monster.VerifyMonster(new ByteBuffer(BuildMonsterWithNestedBuffer(false))));
        }

        [FlatBuffersTestMethod]
        public void VerifyBufferRejectsCorruptNestedFlatBuffer()
        {
            Assert.IsFalse(Monster.VerifyMonster(new ByteBuffer(BuildMonsterWithNestedBuffer(true))));
        }
    }
}
