import Foundation

public struct ByteBuffer {
    
    /// Storage is a container that would hold the memory pointer to solve the issue of
    /// deallocating the memory that was held by (memory: UnsafeMutableRawPointer)
    @usableFromInline final class Storage {
        
        /// pointer to the start of the buffer object in memory
        var memory: UnsafeMutableRawPointer
        /// Capacity of UInt8 the buffer can hold
        var capacity: Int
        
        init(count: Int, alignment: Int) {
            memory = UnsafeMutableRawPointer.allocate(byteCount: count, alignment: alignment)
            capacity = count
        }
        
        deinit {
            memory.deallocate()
        }
        
        func copy(from ptr: UnsafeRawPointer, count: Int) {
            memory.copyMemory(from: ptr, byteCount: count)
        }
        
        func initalize(for size: Int) {
            memory.initializeMemory(as: UInt8.self, repeating: 0, count: size)
        }
    }
    
    @usableFromInline var _storage: Storage
    
    /// The size of the elements written to the buffer + their paddings
    private var _writerSize: Int = 0
    /// Aliginment of the current  memory being written to the buffer
    internal var alignment = 1
    /// Current Index which is being used to write to the buffer, it is written from the end to the start of the buffer
    internal var writerIndex: Int { return _storage.capacity - _writerSize }

    /// Reader is the position of the current Writer Index (capacity - size)
    public var reader: Int { return writerIndex }
    /// Current size of the buffer
    public var size: UOffset { return UOffset(_writerSize) }
    /// Public Pointer to the buffer object in memory. This should NOT be modified for any reason
    public var memory: UnsafeMutableRawPointer { return _storage.memory }
    /// Current capacity for the buffer
    public var capacity: Int { return _storage.capacity }

    /// Constructor that creates a Flatbuffer object from a UInt8
    /// - Parameter bytes: Array of UInt8
    public init(bytes: [UInt8]) {
        var b = bytes
        _storage = Storage(count: bytes.count, alignment: alignment)
        _writerSize = _storage.capacity
        b.withUnsafeMutableBytes { bufferPointer in
            self._storage.copy(from: bufferPointer.baseAddress!, count: bytes.count)
        }
    }

    /// Constructor that creates a Flatbuffer from the Swift Data type object
    /// - Parameter data: Swift data Object
    public init(data: Data) {
        var b = data
        _storage = Storage(count: data.count, alignment: alignment)
        _writerSize = _storage.capacity
        b.withUnsafeMutableBytes { bufferPointer in
            self._storage.copy(from: bufferPointer.baseAddress!, count: data.count)
        }
    }

    /// Constructor that creates a Flatbuffer instance with a size
    /// - Parameter size: Length of the buffer
    init(initialSize size: Int) {
        let size = size.convertToPowerofTwo
        _storage = Storage(count: size, alignment: alignment)
        _storage.initalize(for: size)
    }

#if swift(>=5.0)
    /// Constructor that creates a Flatbuffer object from a ContiguousBytes
    /// - Parameters:
    ///   - contiguousBytes: Binary stripe to use as the buffer
    ///   - count: amount of readable bytes
    public init<Bytes: ContiguousBytes>(
        contiguousBytes: Bytes,
        count: Int
    ) {
        _storage = Storage(count: count, alignment: alignment)
        _writerSize = _storage.capacity
        contiguousBytes.withUnsafeBytes { buf in
            _storage.copy(from: buf.baseAddress!, count: buf.count)
        }
    }
#endif

    /// Creates a copy of the buffer that's being built by calling sizedBuffer
    /// - Parameters:
    ///   - memory: Current memory of the buffer
    ///   - count: count of bytes
    internal init(memory: UnsafeMutableRawPointer, count: Int) {
        _storage = Storage(count: count, alignment: alignment)
        _storage.copy(from: memory, count: count)
        _writerSize = _storage.capacity
    }

    /// Creates a copy of the existing flatbuffer, by copying it to a different memory.
    /// - Parameters:
    ///   - memory: Current memory of the buffer
    ///   - count: count of bytes
    ///   - removeBytes: Removes a number of bytes from the current size
    internal init(memory: UnsafeMutableRawPointer, count: Int, removing removeBytes: Int) {
        _storage = Storage(count: count, alignment: alignment)
        _storage.copy(from: memory, count: count)
        _writerSize = removeBytes
    }

    /// Fills the buffer with padding by adding to the writersize
    /// - Parameter padding: Amount of padding between two to be serialized objects
    @usableFromInline mutating func fill(padding: UInt32) {
        ensureSpace(size: padding)
        _writerSize += (MemoryLayout<UInt8>.size * Int(padding))
    }

    ///Adds an array of type Scalar to the buffer memory
    /// - Parameter elements: An array of Scalars
    @usableFromInline mutating func push<T: Scalar>(elements: [T]) {
        let size = elements.count * MemoryLayout<T>.size
        ensureSpace(size: UInt32(size))
        elements.lazy.reversed().forEach { (s) in
            push(value: s, len: MemoryLayout.size(ofValue: s))
        }
    }
    
    ///Adds an array of type Bool to the buffer memory
    /// - Parameter elements: An array of Bool
    @usableFromInline mutating func push(elements: [Bool]) {
        let size = elements.count * MemoryLayout<Bool>.size
        ensureSpace(size: UInt32(size))
        elements.lazy.reversed().forEach { (s) in
            push(value: s ? 1 : 0, len: MemoryLayout.size(ofValue: s))
        }
    }

    /// A custom type of structs that are padded according to the flatbuffer padding,
    /// - Parameters:
    ///   - value: Pointer to the object in memory
    ///   - size: Size of Value being written to the buffer
    @usableFromInline mutating func push(struct value: UnsafeMutableRawPointer, size: Int) {
        ensureSpace(size: UInt32(size))
        memcpy(_storage.memory.advanced(by: writerIndex - size), value, size)
        defer { value.deallocate() }
        _writerSize += size
    }

    /// Adds an object of type Scalar into the buffer
    /// - Parameters:
    ///   - value: Object  that will be written to the buffer
    ///   - len: Offset to subtract from the WriterIndex
    @usableFromInline mutating func push<T: Scalar>(value: T, len: Int) {
        ensureSpace(size: UInt32(len))
        var v = value.convertedEndian
        memcpy(_storage.memory.advanced(by: writerIndex - len), &v, len)
        _writerSize += len
    }

    /// Adds a string to the buffer using swift.utf8 object
    /// - Parameter str: String that will be added to the buffer
    /// - Parameter len: length of the string
    @usableFromInline mutating func push(string str: String, len: Int) {
        ensureSpace(size: UInt32(len))
        if str.utf8.withContiguousStorageIfAvailable({ self.push(bytes: $0, len: len) }) != nil {
        } else {
            let utf8View = str.utf8
            for c in utf8View.lazy.reversed() {
                push(value: c, len: 1)
            }
        }
    }

    /// Writes a string to Bytebuffer using UTF8View
    /// - Parameters:
    ///   - bytes: Pointer to the view
    ///   - len: Size of string
    @usableFromInline mutating internal func push(bytes: UnsafeBufferPointer<String.UTF8View.Element>, len: Int) -> Bool {
        memcpy(_storage.memory.advanced(by: writerIndex - len), UnsafeRawPointer(bytes.baseAddress!), len)
        _writerSize += len
        return true
    }

    /// Write stores an object into the buffer directly or indirectly.
    ///
    /// Direct: ignores the capacity of buffer which would mean we are referring to the direct point in memory
    /// indirect: takes into respect the current capacity of the buffer (capacity - index), writing to the buffer from the end
    /// - Parameters:
    ///   - value: Value that needs to be written to the buffer
    ///   - index: index to write to
    ///   - direct: Should take into consideration the capacity of the buffer
    func write<T>(value: T, index: Int, direct: Bool = false) {
        var index = index
        if !direct {
            index = _storage.capacity - index
        }
        _storage.memory.storeBytes(of: value, toByteOffset: index, as: T.self)
    }

    /// Makes sure that buffer has enouch space for each of the objects that will be written into it
    /// - Parameter size: size of object
    @discardableResult
    @usableFromInline mutating func ensureSpace(size: UInt32) -> UInt32 {
        if Int(size) + _writerSize > _storage.capacity { reallocate(size) }
        assert(size < FlatBufferMaxSize, "Buffer can't grow beyond 2 Gigabytes")
        return size
    }

    /// Reallocates the buffer incase the object to be written doesnt fit in the current buffer
    /// - Parameter size: Size of the current object
    @usableFromInline mutating internal func reallocate(_ size: UInt32) {
        let currentWritingIndex = writerIndex
        while _storage.capacity <= _writerSize + Int(size) {
            _storage.capacity = _storage.capacity << 1
        }

        /// solution take from Apple-NIO
        _storage.capacity = _storage.capacity.convertToPowerofTwo

        let newData = UnsafeMutableRawPointer.allocate(byteCount: _storage.capacity, alignment: alignment)
        newData.initializeMemory(as: UInt8.self, repeating: 0, count: _storage.capacity)
        newData
            .advanced(by: writerIndex)
            .copyMemory(from: _storage.memory.advanced(by: currentWritingIndex), byteCount: _writerSize)
        _storage.memory.deallocate()
        _storage.memory = newData
    }

    /// Clears the current size of the buffer
    mutating public func clearSize() {
        _writerSize = 0
    }

    /// Clears the current instance of the buffer, replacing it with new memory
    mutating public func clear() {
        _writerSize = 0
        alignment = 1
        _storage.memory.deallocate()
        _storage.memory = UnsafeMutableRawPointer.allocate(byteCount: _storage.capacity, alignment: alignment)
    }
    
    /// Resizes the buffer size
    /// - Parameter size: new size for the buffer
    @usableFromInline mutating internal func resize(_ size: Int) {
        _writerSize = size
    }

    /// Reads an object from the buffer
    /// - Parameters:
    ///   - def: Type of the object
    ///   - position: the index of the object in the buffer
    public func read<T>(def: T.Type, position: Int) -> T {
        return _storage.memory.advanced(by: position).load(as: T.self)
    }

    /// Reads a slice from the memory assuming a type of T
    /// - Parameters:
    ///   - index: index of the object to be read from the buffer
    ///   - count: count of bytes in memory
    public func readSlice<T>(index: Int32,
                             count: Int32) -> [T] {
        let start = _storage.memory.advanced(by: Int(index)).assumingMemoryBound(to: T.self)
        let array = UnsafeBufferPointer(start: start, count: Int(count))
        return Array(array)
    }

    /// Reads a string from the buffer and encodes it to a swift string
    /// - Parameters:
    ///   - index: index of the string in the buffer
    ///   - count: length of the string
    ///   - type: Encoding of the string
    public func readString(at index: Int32,
                           count: Int32,
                           type: String.Encoding = .utf8) -> String? {
        let start = _storage.memory.advanced(by: Int(index)).assumingMemoryBound(to: UInt8.self)
        let bufprt = UnsafeBufferPointer(start: start, count: Int(count))
        return String(bytes: Array(bufprt), encoding: type)
    }

    /// Creates a new Flatbuffer object that's duplicated from the current one
    /// - Parameter removeBytes: the amount of bytes to remove from the current Size
    public func duplicate(removing removeBytes: Int = 0) -> ByteBuffer {
        return ByteBuffer(memory: _storage.memory, count: _storage.capacity, removing: _writerSize - removeBytes)
    }
}

extension ByteBuffer: CustomDebugStringConvertible {

    public var debugDescription: String {
        """
        buffer located at: \(_storage.memory), with capacity of \(_storage.capacity)
        { writerSize: \(_writerSize), readerSize: \(reader), writerIndex: \(writerIndex) }
        """
    }
}
