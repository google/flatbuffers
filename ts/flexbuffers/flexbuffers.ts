/* eslint-disable @typescript-eslint/no-namespace */
flexbuffers.toObject = (buffer) => {
    return flexbuffers.toReference(buffer).toObject();
  };
  

flexbuffers.encode = (object, size = 2048, deduplicateStrings = true, deduplicateKeys = true, deduplicateKeyVectors = true) => {
    const builder = flexbuffers.builder(size > 0 ? size : 2048, deduplicateStrings, deduplicateKeys, deduplicateKeyVectors);
    builder.add(object);
    return builder.finish();
};

function fromUTF8Array(data) {
    const decoder = new TextDecoder();
    return decoder.decode(data);
}

function toUTF8Array(str) {
    const encoder = new TextEncoder();
    return encoder.encode(str);
}

export namespace flexbuffers {

}

export default flexbuffers;
