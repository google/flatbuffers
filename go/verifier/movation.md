## 0. movation

i trying implementing a instance message service via  tcp client / server.    The Message serialized by flatbuffers, so, read streaming flatbuffers Message is must.

learning from 

https://github.com/google/flatbuffers/issues/3898

https://github.com/google/flatbuffers/issues/5775

https://github.com/google/flatbuffers/issues/4604

https://github.com/google/flatbuffers/pull/3905

i will implementing verify in Go,  here is some knowdge:

## 1. struct of serialized flatbuffers

  assume serialized flatbufers ( Message ) as two part:  Header ( metadata ) / Payload ( data ....) 

### 1.1 Message Header 

1. 4 byte of root header ( , get offset of payload start

2. zero or more then one byte of prefilled ZERO

3. vtable struct , as below reference from builder.go,  the Payload size ( data size ) include in vtable

```

// A vtable has the following format:

//   <VOffsetT: size of the vtable in bytes, including this value>

//   <VOffsetT: size of the object in bytes, including the vtable offset>

//   <VOffsetT: offset for a field> * N, where N is the number of fields in

//	        the schema for this type. Includes deprecated fields.

// Thus, a vtable is made of 2 + N elements, each SizeVOffsetT bytes wide.

//

// An object has the following format:

//   <SOffsetT: offset to this object's vtable (may be negative)>

//   <byte: data>+

```

### 1.2 Message payload part struct 

TBD....


## 2. read streaming serialized flatbuffers

1. read  Message (flatbuf serialized data)  from network, 4 byte by 4 byte 

2. read and parse header.  check is a valid flatbufers or not  

3. get the data size of message from header,   and get the rest data

4. un-serialize the Message when know the IDL

5. ( option ) Experimental reflaction to guess ths struct of IDL

 ## 3. a bug in builder.go , in func (b *Builder) WriteVtable() 

As my assume,  serialized flatbufers ( Message ) as two part:  Header ( metadata ) / Payload ( data ....) .

<VOffsetT: size of the vtable in bytes, including this value>   -------------- call it as vTableSize 
 <VOffsetT: size of the object in bytes, including the vtable offset> -------- this " size of the object" should be size of  Message Payload ( call it as payloadSize ) 

so,  

**whole size of serialized flatbuffers  =  HeaderSize ( include vTableSize )  + payloadSize**

in builder.go
```
		// The two metadata fields are written last.

		// First, store the object bytesize:
		objectSize := objectOffset // - b.objectEnd   ---------------------- bug
		b.PrependVOffsetT(VOffsetT(objectSize))

		// Second, store the vtable bytesize:
		vBytes := (len(b.vtable) + VtableMetadataFields) * SizeVOffsetT
		b.PrependVOffsetT(VOffsetT(vBytes))

		// Next, write the offset to the new vtable in the
		// already-allocated SOffsetT at the beginning of this object:
		objectStart := SOffsetT(len(b.Bytes)) - SOffsetT(objectOffset)
		WriteSOffsetT(b.Bytes[objectStart:],
			SOffsetT(b.Offset())-SOffsetT(objectOffset))

		// Finally, store this vtable in memory for future
		// deduplication:
		b.vtables = append(b.vtables, b.Offset())
```

------------------

in builder.go , line 163:
```
      		objectSize := objectOffset - b.objectEnd  
```


testing IDL like this:
```
table String {
    Field:string;
}
```

filled the  Field with "1234567",    serialized data should be

[1100 0 0 0 0 0 110 0 10100 0 100 0 110 0 0 0 100 0 0 0 111 0 0 0 110001 110010 110011 110100 110101 110110 110111 0]

--> Object size in 9th byte shuld be 20  --- [ 101000 ] -----> 4 byte SOffset / 4 byte vector construction / 4 byte is real size of string ( "1234567" is 7 )  / 7 byte "1234567" +  1 byte ended string with "zero" byte  ,  no any pre-filled "zero" byte in end of string 

total serialized data is 32 byte:
* 4 byte is UOffset.
* 2 byte pre-filled "zero" byte
* 6 byte vTable,  include  2 byte VOffset for vTable size, 2 byte VOffset for data object size, 2 byte VOffset  for one slot ( in this example , only one field in table ) 
* 20 byte for data , include 4 byte SOffset, 16 byte for string vector construction 


but , we got: 
 
[1100 0 0 0 0 0 110 0 1000 0 100 0 110 0 0 0 100 0 0 0 111 0 0 0 110001 110010 110011 110100 110101 110110 110111 0]
 
--> Object size in 9th byte is 8   --- [ 1000]
 
in builder.go , line 163, change to :
```
      		objectSize := objectOffset  
```

will got correct


-----------------
correct object size in vTable , will help to read streaming flatbuffers .
-----------------

i will PR today.





any suggestion  r welcome!