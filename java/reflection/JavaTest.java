package reflection;/*
 * Copyright 2014 Google Inc. All rights reserved.
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

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;

class JavaTest {
    public static void main(String[] args) {
        TestReflection();

        System.out.println("FlatBuffers test: completed successfully");
    }

    static void TestReflection() {
        byte[] data = null;
        File file = new File("monster_test.bfbs");
        RandomAccessFile f = null;
        try {
            f = new RandomAccessFile(file, "r");
            data = new byte[(int)f.length()];
            f.readFully(data);
            f.close();
        } catch(IOException e) {
            System.err.println("FlatBuffers test: couldn't read binary schema file");
            return;
        } finally {
            if (f != null) {
                try {
                    f.close();
                } catch (IOException e) {
                    //ignored
                }
            }
        }

        // Now test it:

        ByteBuffer bb = ByteBuffer.wrap(data);
        Schema schema = Schema.getRootAsSchema(bb);
        Object rootTable = schema.rootTable();
        TestEq(rootTable.name(), "Monster");
        TestEq(rootTable.fieldsLength(), 29);
        Field field = rootTable.field("hp");
        TestEq(field.name(), "hp");
        TestEq(field.id(), 2);

    }

    static <T> void TestEq(T a, T b) {
        if (!a.equals(b)) {
            System.out.println("" + a.getClass().getName() + " " + b.getClass().getName());
            System.out.println("FlatBuffers test FAILED: \'" + a + "\' != \'" + b + "\'");
            assert false;
            System.exit(1);
        }
    }
}
