/*
 * Copyright 2019 Google Inc. All rights reserved.
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

import static com.google.flatbuffers.Constants.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;

/**
 * Helper type for accessing vector of signed or unsigned 32-bit values.
 */
public final class IntVector extends BaseVector {
  /**
   * Assigns vector access object to vector data.
   *
   * @param _vector Start data of a vector.
   * @param _bb Table's ByteBuffer.
   * @return Returns current vector access object assigned to vector data whose offset is stored at
   *         `vector`.
   */
  public IntVector __assign(int _vector, ByteBuffer _bb) {
    if(_bb == null){
      throw new IllegalArgumentException("ByteBuffer cannot be null");
    }
    __reset(_vector, Constants.SIZEOF_INT, _bb); return this;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append("[");
    for(int i = 0; i < length(); i++){
      if(i > 0){
        sb.append(", ");
      }
      sb.append(get(i));
    }
    sb.append("]");
    return sb.toString();
  }

  /**
   * Reads the integer at the given index.
   *
   * @param j The index from which the integer will be read.
   * @return the 32-bit value at the given index.
   */
  public int get(int j) {
    if(j < 0 || j >= length()){
      throw new IndexOutOfBoundsException("Index " + j + " is out of bounds for length " + length());
    }
    return bb.getInt(__element(j));
  }

  /**
   * Reads the integer at the given index, zero-extends it to type long, and returns the result,
   * which is therefore in the range 0 through 4294967295.
   *
   * Java does not natively support unsigned integers, so this method returns a 'long' to represent the unsigned 32-bit value.
   * 
   * @param j The index from which the integer will be read.
   * @return the unsigned 32-bit at the given index.
   */

  public long getAsUnsigned(int j) {
    return (long) get(j) & 0xFFFFFFFFL;
  }
}
