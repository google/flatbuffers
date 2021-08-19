// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

[Newtonsoft.Json.JsonConverter(typeof(Newtonsoft.Json.Converters.StringEnumConverter))]
public enum KeywordsInUnion : byte
{
  NONE = 0,
  static_ = 1,
  internal_ = 2,
};

public class KeywordsInUnionUnion {
  public KeywordsInUnion Type { get; set; }
  public object Value { get; set; }

  public KeywordsInUnionUnion() {
    this.Type = KeywordsInUnion.NONE;
    this.Value = null;
  }

  public T As<T>() where T : class { return this.Value as T; }
  public global::KeywordsInTableT Asstatic() { return this.As<global::KeywordsInTableT>(); }
  public static KeywordsInUnionUnion Fromstatic(global::KeywordsInTableT _static) { return new KeywordsInUnionUnion{ Type = KeywordsInUnion.static_, Value = _static }; }
  public global::KeywordsInTableT Asinternal() { return this.As<global::KeywordsInTableT>(); }
  public static KeywordsInUnionUnion Frominternal(global::KeywordsInTableT _internal) { return new KeywordsInUnionUnion{ Type = KeywordsInUnion.internal_, Value = _internal }; }

  public static int Pack(FlatBuffers.FlatBufferBuilder builder, KeywordsInUnionUnion _o) {
    switch (_o.Type) {
      default: return 0;
      case KeywordsInUnion.static_: return global::KeywordsInTable.Pack(builder, _o.Asstatic()).Value;
      case KeywordsInUnion.internal_: return global::KeywordsInTable.Pack(builder, _o.Asinternal()).Value;
    }
  }
}

public class KeywordsInUnionUnion_JsonConverter : Newtonsoft.Json.JsonConverter {
  public override bool CanConvert(System.Type objectType) {
    return objectType == typeof(KeywordsInUnionUnion) || objectType == typeof(System.Collections.Generic.List<KeywordsInUnionUnion>);
  }
  public override void WriteJson(Newtonsoft.Json.JsonWriter writer, object value, Newtonsoft.Json.JsonSerializer serializer) {
    var _olist = value as System.Collections.Generic.List<KeywordsInUnionUnion>;
    if (_olist != null) {
      writer.WriteStartArray();
      foreach (var _o in _olist) { this.WriteJson(writer, _o, serializer); }
      writer.WriteEndArray();
    } else {
      this.WriteJson(writer, value as KeywordsInUnionUnion, serializer);
    }
  }
  public void WriteJson(Newtonsoft.Json.JsonWriter writer, KeywordsInUnionUnion _o, Newtonsoft.Json.JsonSerializer serializer) {
    if (_o == null) return;
    serializer.Serialize(writer, _o.Value);
  }
  public override object ReadJson(Newtonsoft.Json.JsonReader reader, System.Type objectType, object existingValue, Newtonsoft.Json.JsonSerializer serializer) {
    var _olist = existingValue as System.Collections.Generic.List<KeywordsInUnionUnion>;
    if (_olist != null) {
      for (var _j = 0; _j < _olist.Count; ++_j) {
        reader.Read();
        _olist[_j] = this.ReadJson(reader, _olist[_j], serializer);
      }
      reader.Read();
      return _olist;
    } else {
      return this.ReadJson(reader, existingValue as KeywordsInUnionUnion, serializer);
    }
  }
  public KeywordsInUnionUnion ReadJson(Newtonsoft.Json.JsonReader reader, KeywordsInUnionUnion _o, Newtonsoft.Json.JsonSerializer serializer) {
    if (_o == null) return null;
    switch (_o.Type) {
      default: break;
      case KeywordsInUnion.static_: _o.Value = serializer.Deserialize<global::KeywordsInTableT>(reader); break;
      case KeywordsInUnion.internal_: _o.Value = serializer.Deserialize<global::KeywordsInTableT>(reader); break;
    }
    return _o;
  }
}

