// automatically generated by the FlatBuffers compiler, do not modify
// ignore_for_file: unused_import, unused_field, unused_element, unused_local_variable

library namespace_a.namespace_b;

import 'dart:typed_data' show Uint8List;
import 'package:flat_buffers/flat_buffers.dart' as fb;


class ColorTestTable {
  ColorTestTable._(this._bc, this._bcOffset);
  factory ColorTestTable(List<int> bytes) {
    fb.BufferContext rootRef = new fb.BufferContext.fromBytes(bytes);
    return reader.read(rootRef, 0);
  }

  static const fb.Reader<ColorTestTable> reader = const _ColorTestTableReader();

  final fb.BufferContext _bc;
  final int _bcOffset;

  Color get color => Color.fromValue(const fb.Int8Reader().vTableGet(_bc, _bcOffset, 4, 2));

  @override
  String toString() {
    return 'ColorTestTable{color: $color}';
  }

  ColorTestTableT unpack() => ColorTestTableT(
      color: color);

  static int pack(fb.Builder fbBuilder, ColorTestTableT? object) {
    if (object == null) return 0;
    return object.pack(fbBuilder);
  }
}

class ColorTestTableT {
  Color color;

  ColorTestTableT({
      this.color = Color.Blue});

  int pack(fb.Builder fbBuilder) {
    fbBuilder.startTable(1);
    fbBuilder.addInt8(0, color.value);
    return fbBuilder.endTable();
  }

  @override
  String toString() {
    return 'ColorTestTableT{color: $color}';
  }
}

class _ColorTestTableReader extends fb.TableReader<ColorTestTable> {
  const _ColorTestTableReader();

  @override
  ColorTestTable createObject(fb.BufferContext bc, int offset) => 
    new ColorTestTable._(bc, offset);
}

class ColorTestTableBuilder {
  ColorTestTableBuilder(this.fbBuilder) {}

  final fb.Builder fbBuilder;

  void begin() {
    fbBuilder.startTable(1);
  }

  int addColor(Color? color) {
    fbBuilder.addInt8(0, color?.value);
    return fbBuilder.offset;
  }

  int finish() {
    return fbBuilder.endTable();
  }
}

class ColorTestTableObjectBuilder extends fb.ObjectBuilder {
  final Color? _color;

  ColorTestTableObjectBuilder({
    Color? color,
  })
      : _color = color;

  /// Finish building, and store into the [fbBuilder].
  @override
  int finish(fb.Builder fbBuilder) {
    fbBuilder.startTable(1);
    fbBuilder.addInt8(0, _color?.value);
    return fbBuilder.endTable();
  }

  /// Convenience method to serialize to byte list.
  @override
  Uint8List toBytes([String? fileIdentifier]) {
    fb.Builder fbBuilder = new fb.Builder(deduplicateTables: false);
    int offset = finish(fbBuilder);
    fbBuilder.finish(offset, fileIdentifier);
    return fbBuilder.buffer;
  }
}
