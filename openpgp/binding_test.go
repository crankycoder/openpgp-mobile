package openpgp

// #cgo CFLAGS: -I../output/binding
// #cgo LDFLAGS: -L../output/binding -lopenpgp_bridge
// #include "libopenpgp_bridge.h"
// #include <stdlib.h>
import "C"
import (
	"testing"
	"unsafe"
)

// TestBindingLoads verifies that the C binding can be loaded
func TestBindingLoads(t *testing.T) {
	// This test will fail to compile if the binding isn't built
	// Simply importing C with the header is enough to verify loading
	t.Log("C binding loaded successfully")
}

// TestBytesReturnAllocation tests BytesReturn memory management
func TestBytesReturnAllocation(t *testing.T) {
	// Test creating and freeing a BytesReturn with error
	errorMsg := C.CString("test error")
	defer C.free(unsafe.Pointer(errorMsg))
	
	br := (*C.BytesReturn)(C.malloc(C.size_t(unsafe.Sizeof(C.BytesReturn{}))))
	br.message = nil
	br.size = 0
	br.error = errorMsg
	
	// Verify the structure is set correctly
	if br.error == nil {
		t.Fatal("Expected error to be set")
	}
	if br.message != nil {
		t.Fatal("Expected message to be nil")
	}
	
	// Note: We don't call OpenPGPFreeBytesReturn here because it would
	// free errorMsg which we're managing separately for this test
	C.free(unsafe.Pointer(br))
}

// TestOpenPGPEncodeText tests the text encoding function
func TestOpenPGPEncodeText(t *testing.T) {
	input := C.CString("Hello, World!")
	encoding := C.CString("utf-8")
	defer C.free(unsafe.Pointer(input))
	defer C.free(unsafe.Pointer(encoding))
	
	result := C.OpenPGPEncodeText(input, encoding)
	if result == nil {
		t.Fatal("Expected non-nil result")
	}
	defer C.OpenPGPFreeBytesReturn(result)
	
	if result.error != nil {
		errorStr := C.GoString(result.error)
		t.Fatalf("Unexpected error: %s", errorStr)
	}
	
	if result.size == 0 {
		t.Fatal("Expected non-zero size")
	}
	
	// Convert result back to Go string to verify
	encoded := C.GoBytes(result.message, result.size)
	if string(encoded) != "Hello, World!" {
		t.Fatalf("Expected 'Hello, World!', got '%s'", string(encoded))
	}
}

// TestOpenPGPDecodeText tests the text decoding function
func TestOpenPGPDecodeText(t *testing.T) {
	// First encode some text
	input := []byte("Hello, Decode!")
	encoding := C.CString("utf-8")
	defer C.free(unsafe.Pointer(encoding))
	
	// Decode it back
	decoded := C.OpenPGPDecodeText(
		unsafe.Pointer(&input[0]),
		C.int(len(input)),
		encoding,
		0, // not fatal
		0, // don't ignore BOM
		0, // not streaming
	)
	if decoded == nil {
		t.Fatal("Expected non-nil result")
	}
	defer C.OpenPGPFreeString(decoded)
	
	decodedStr := C.GoString(decoded)
	if decodedStr != "Hello, Decode!" {
		t.Fatalf("Expected 'Hello, Decode!', got '%s'", decodedStr)
	}
}

// TestMemoryManagement verifies our memory management functions work
func TestMemoryManagement(t *testing.T) {
	// Test OpenPGPFreeString with nil
	C.OpenPGPFreeString(nil) // Should not crash
	
	// Test OpenPGPFreeBytesReturn with nil
	C.OpenPGPFreeBytesReturn(nil) // Should not crash
	
	// Test with actual allocations
	str := C.CString("test string")
	C.OpenPGPFreeString(str) // Should free without issues
	
	t.Log("Memory management functions work correctly")
}