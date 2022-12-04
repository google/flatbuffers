// automatically generated by the FlatBuffers compiler, do not modify

import com.google.flatbuffers.BaseVector;
import com.google.flatbuffers.BooleanVector;
import com.google.flatbuffers.ByteVector;
import com.google.flatbuffers.Constants;
import com.google.flatbuffers.DoubleVector;
import com.google.flatbuffers.FlatBufferBuilder;
import com.google.flatbuffers.FloatVector;
import com.google.flatbuffers.LongVector;
import com.google.flatbuffers.StringVector;
import com.google.flatbuffers.Struct;
import com.google.flatbuffers.Table;
import com.google.flatbuffers.UnionVector;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@SuppressWarnings("unused")
public final class BookReader extends Struct {
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public BookReader __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int booksRead() { return bb.getInt(bb_pos + 0); }
  public void mutateBooksRead(int books_read) { bb.putInt(bb_pos + 0, books_read); }

  public static int createBookReader(FlatBufferBuilder builder, int booksRead) {
    builder.prep(4, 4);
    builder.putInt(booksRead);
    return builder.offset();
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public BookReader get(int j) { return get(new BookReader(), j); }
    public BookReader get(BookReader obj, int j) {  return obj.__assign(__element(j), bb); }
  }
  public BookReaderT unpack() {
    BookReaderT _o = new BookReaderT();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(BookReaderT _o) {
    int _oBooksRead = booksRead();
    _o.setBooksRead(_oBooksRead);
  }
  public static int pack(FlatBufferBuilder builder, BookReaderT _o) {
    if (_o == null) return 0;
    return createBookReader(
      builder,
      _o.getBooksRead());
  }
}

