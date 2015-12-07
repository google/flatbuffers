// automatically generated, do not modify

namespace NamespaceA.NamespaceB
{

using FlatBuffers;

public sealed class TableInNestedNS : Table {
  public static TableInNestedNS GetRootAsTableInNestedNS(ByteBuffer _bb) { return GetRootAsTableInNestedNS(_bb, new TableInNestedNS()); }
  public static TableInNestedNS GetRootAsTableInNestedNS(ByteBuffer _bb, TableInNestedNS obj) { return (obj.__init(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public TableInNestedNS __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int Foo { get { int o = __offset(4); return o != 0 ? bb.GetInt(o + bb_pos) : (int)0; } }
  public bool MutateFoo(int foo) { int o = __offset(4); if (o != 0) { bb.PutInt(o + bb_pos, foo); return true; } else { return false; } }

  public static Offset<TableInNestedNS> CreateTableInNestedNS(FlatBufferBuilder builder,
      int foo = 0) {
    builder.StartObject(1);
    TableInNestedNS.AddFoo(builder, foo);
    return TableInNestedNS.EndTableInNestedNS(builder);
  }

  public static void StartTableInNestedNS(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddFoo(FlatBufferBuilder builder, int foo) { builder.AddInt(0, foo, 0); }
  public static Offset<TableInNestedNS> EndTableInNestedNS(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<TableInNestedNS>(o);
  }
};


}
