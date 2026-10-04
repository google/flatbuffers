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
package com.google.flatbuffers;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;

import java.nio.charset.StandardCharsets;
import org.junit.Test;

public class FlexBuffersJsonTest {
  private static FlexBuffers.Reference stringRoot(String value) {
    FlexBuffersBuilder builder = new FlexBuffersBuilder();
    builder.putString(value);
    builder.finish();
    return FlexBuffers.getRoot(builder.getBuffer());
  }

  @Test
  public void stringsEscapeJsonSyntaxAndControlCharacters() {
    String[] values = {"\"", "\\", "\b", "\f", "\n", "\r", "\t", "\u0000", "\u001f"};
    String[] escaped = {"\\\"", "\\\\", "\\b", "\\f", "\\n", "\\r", "\\t", "\\u0000", "\\u001f"};
    for (int i = 0; i < values.length; i++) {
      FlexBuffers.Reference root = stringRoot(values[i]);
      assertEquals(values[i], root.asString());
      assertEquals('"' + escaped[i] + '"', root.toString());
    }
  }

  @Test
  public void allControlCharactersAreEscapedInsideOneString() {
    StringBuilder value = new StringBuilder();
    for (int i = 0; i < 0x20; i++) {
      value.append((char) i);
    }
    String expected =
        "\"\\u0000\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007"
            + "\\b\\t\\n\\u000b\\f\\r\\u000e\\u000f"
            + "\\u0010\\u0011\\u0012\\u0013\\u0014\\u0015\\u0016\\u0017"
            + "\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f\"";
    assertEquals(expected, stringRoot(value.toString()).toString());
  }

  @Test
  public void unicodeAndOrdinaryCharactersArePreserved() {
    String value = "ASCII / é 中文 \u007f \u2028 \u2029 😨";
    FlexBuffers.Reference root = stringRoot(value);
    assertEquals('"' + value + '"', root.toString());
    assertEquals(value, root.asString());
    assertEquals("\"\"", stringRoot("").toString());
    assertEquals("\"\\\\n\"", stringRoot("\\n").toString());
  }

  @Test
  public void nestedMapsAndVectorsEscapeKeysAndValues() {
    FlexBuffersBuilder builder = new FlexBuffersBuilder();
    int map = builder.startMap();
    int vector = builder.startVector();
    builder.putString("first\nline");
    int nestedMap = builder.startMap();
    builder.putString("tab\tkey", "quote\"value");
    builder.endMap(null, nestedMap);
    builder.endVector("slash\\key", vector, false, false);
    builder.endMap(null, map);
    builder.finish();

    FlexBuffers.Reference root = FlexBuffers.getRoot(builder.getBuffer());
    String expected =
        "{ \"slash\\\\key\" : [ \"first\\nline\", { \"tab\\tkey\" : \"quote\\\"value\" } ] }";
    assertEquals(expected, root.toString());
    FlexBuffers.Map result = root.asMap();
    assertEquals("slash\\key", result.keys().get(0).toString());
    FlexBuffers.Vector values = result.get("slash\\key").asVector();
    assertEquals("first\nline", values.get(0).asString());
    assertEquals("quote\"value", values.get(1).asMap().get("tab\tkey").asString());
    StringBuilder appended = new StringBuilder("prefix:");
    assertSame(appended, result.toString(appended));
    assertEquals("prefix:" + expected, appended.toString());
  }

  @Test
  public void keyReferencesAreQuotedWithoutChangingKeyAccessors() {
    byte[] key = "quote\"slash\\newline\n".getBytes(StandardCharsets.UTF_8);
    byte[] data = new byte[key.length + 4];
    System.arraycopy(key, 0, data, 0, key.length);
    data[key.length + 1] = (byte) (key.length + 1);
    data[key.length + 2] = (byte) (FlexBuffers.FBT_KEY << 2);
    data[key.length + 3] = 1;
    FlexBuffers.Reference root = FlexBuffers.getRoot(new ArrayReadWriteBuf(data, data.length));
    assertEquals("\"quote\\\"slash\\\\newline\\n\"", root.toString());
    assertEquals(new String(key, StandardCharsets.UTF_8), root.asKey().toString());
    StringBuilder appended = new StringBuilder("prefix:");
    root.asKey().toString(appended);
    assertEquals("prefix:" + new String(key, StandardCharsets.UTF_8), appended.toString());
  }

  @Test
  public void typedKeyVectorsEscapeEachElement() {
    byte[] data = {
      'a', '"', 0, 'b', '\n', 0, 2, 7, 5, 2, (byte) (FlexBuffers.FBT_VECTOR_KEY << 2), 1
    };
    FlexBuffers.Reference root = FlexBuffers.getRoot(new ArrayReadWriteBuf(data, data.length));
    String expected = "[ \"a\\\"\", \"b\\n\" ]";
    assertEquals(expected, root.toString());
    assertEquals(expected, root.asVector().toString());
    assertEquals("a\"", root.asVector().get(0).asKey().toString());
    assertEquals("b\n", root.asVector().get(1).asKey().toString());
  }

  @Test
  public void blobJsonEscapesTextWithoutChangingBytes() {
    String value = "blob\u0000\n\"\\é";
    byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
    FlexBuffersBuilder builder = new FlexBuffersBuilder();
    builder.putBlob(bytes);
    builder.finish();
    FlexBuffers.Reference root = FlexBuffers.getRoot(builder.getBuffer());
    String expected = "\"blob\\u0000\\n\\\"\\\\é\"";
    assertEquals(expected, root.toString());
    assertArrayEquals(bytes, root.asBlob().getBytes());
    assertEquals(value, root.asBlob().toString());
    StringBuilder appended = new StringBuilder("prefix:");
    assertSame(appended, root.asBlob().toString(appended));
    assertEquals("prefix:" + expected, appended.toString());
  }

  @Test
  public void ordinaryMapFormattingIsPreserved() {
    FlexBuffersBuilder builder = new FlexBuffersBuilder();
    int map = builder.startMap();
    builder.putString("name", "flatbuffers");
    builder.putBoolean("ready", true);
    builder.putInt("count", 3);
    builder.endMap(null, map);
    builder.finish();
    FlexBuffers.Reference root = FlexBuffers.getRoot(builder.getBuffer());
    assertEquals(
        "{ \"count\" : 3, \"name\" : \"flatbuffers\", \"ready\" : true }", root.toString());
  }
}
