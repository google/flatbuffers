/* eslint-disable @typescript-eslint/no-namespace */
import { Builder } from './flexbuffers/builder';
import { toReference } from './flexbuffers/reference';
export { toReference } from './flexbuffers/reference';
export function builder() {
    return new Builder();
}
export function toObject(buffer) {
    return toReference(buffer).toObject();
}
export function encode(object, size = 2048, deduplicateStrings = true, deduplicateKeys = true, deduplicateKeyVectors = true) {
    const builder = new Builder(size > 0 ? size : 2048, deduplicateStrings, deduplicateKeys, deduplicateKeyVectors);
    builder.add(object);
    return builder.finish();
}
