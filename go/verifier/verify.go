package verifier

import (
	"errors"

	flatbuffers "github.com/google/flatbuffers/go"
)

// Verify parse a valid serialized flatbuffers byte slice and return parse result
type Verify struct {
	payloadSize    flatbuffers.VOffsetT // payload size, include 4 byte SOffset to vTable
	vTableSize     flatbuffers.VOffsetT // vTable size
	vTableStart    flatbuffers.SOffsetT // vTable offset from start of serialized flatbuffers byte slice
	vTableOffset   flatbuffers.SOffsetT // value in first 4 byte of payload
	PositionToData flatbuffers.UOffsetT // Always < 1<<31.
	slotNumbers    int                  // how many slot ( object field ) in vTable
	slotExited     bool                 // set true when slot numbers is zero
	headVerified   bool                 // verified as avid header of serialized flatbuffers Header
	parseFinished  bool                 // flag for streaming read
}

// NewVerify  initial
func NewVerify() *Verify {
	return new(Verify)
}

// parseFirst parse the table offset
func (t *Verify) parseFirst(buf []byte, offset flatbuffers.UOffsetT) error {
	if len(buf) < flatbuffers.SizeUOffsetT {
		return errors.New("byte slice less then 4 byte")
	}
	t.PositionToData = flatbuffers.GetUOffsetT(buf[offset:])
	// if t.PositionToData < offset {
	// 	return errors.New("can't parse data position")
	// }
	return nil
}

// parseSize parse the vTable size and object size in vTable
func (t *Verify) parseSize(buf []byte) error {
	t.vTableSize = flatbuffers.GetVOffsetT(buf[t.vTableStart:])
	t.payloadSize = flatbuffers.GetVOffsetT(buf[t.vTableStart+flatbuffers.SizeVOffsetT:])
	if t.vTableSize > 0 && t.vTableSize > flatbuffers.SizeSOffsetT {
		t.slotNumbers = (int(t.vTableSize) - flatbuffers.SizeSOffsetT) / flatbuffers.SizeVOffsetT
		t.slotExited = true
	}
	return nil
}

// Parse verifier the vTable struct , return total byte size of root table / vTable / SOffset
// used in io.Reader for read flatbuffers byte slice in TCP / UDP network
//
// when parse finish with no error ,  return the numbers of Header size + 4 byte of SOffset
// *** Notice ***
// use RestPayloadSize() to get rest serialized data size
//
// please double check the comments of function PayloadSize() / HeaderSize() / RestPayloadSize()
func (t *Verify) Parse(buf []byte, offset flatbuffers.UOffsetT) (int, error) {
	// get the header offset as header size

	er1 := t.parseFirst(buf, offset)
	if er1 != nil {
		return 0, er1
	}

	//  can't parse until size of buf is equal to or more than header size
	if len(buf) < (int(t.PositionToData)+flatbuffers.SizeSOffsetT) && (int(t.PositionToData) > 0) {
		return 0, errors.New("can't parse data position")
	}

	t.vTableOffset = flatbuffers.GetSOffsetT(buf[t.PositionToData:])

	if int(t.vTableOffset) > len(buf) || t.vTableOffset < flatbuffers.SizeSOffsetT {
		return 0, errors.New("vtable not exist")
	}

	t.vTableStart = flatbuffers.SOffsetT(t.PositionToData) - t.vTableOffset

	err := t.parseSize(buf)
	if err != nil {
		return 0, err
	}
	t.headVerified = true

	return int(t.PositionToData + flatbuffers.SizeSOffsetT), nil
}

// Verify parse result
// return true when parse a verified valid serialized flatbuffers Header
func (t *Verify) Verify() bool {
	return t.headVerified
}

// ParseFinish  status of parse process
// return true when parse finish or invalid header or too mush data to parse
func (t *Verify) ParseFinish() bool {
	return t.parseFinished
}

// PayloadSize  size of flatbuffers Message payload ( data ) byte slice, include first 4  byte as SOffset
func (t *Verify) PayloadSize() int {
	return int(t.payloadSize)
}

// RestPayloadSize  size of rest flatbuffers Message payload ( data ) byte slice,
// there  first 4 byte ( as SOffset )  of payload been read to finish parse Header
func (t *Verify) RestPayloadSize() int {
	return int(t.payloadSize) - 4
}

// HeaderSize  size of flatbuffers Header byte slice
func (t *Verify) HeaderSize() int {
	return int(t.vTableStart) + int(t.vTableSize)
}

// VTableSize  size of flatbuffers vTable
func (t *Verify) VTableSize() int {
	return int(t.vTableSize)
}

// Size  whole size of flatbuffers serialized byte slice, include Header and Payload
func (t *Verify) Size() int {
	return int(t.vTableStart) + int(t.vTableSize) + int(t.payloadSize)
}

// Reset reset verifier
func (t *Verify) Reset() {
	t.headVerified = false
	t.slotExited = false
	t.PositionToData = 0
}

func (t *Verify) HeaderStart() int {
	return int(t.vTableStart)
}
