#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <Windows.h>

//!!error codes from DLL project, Don't forget to change in both files if you change it
#define ERR_BUFFER_NULL				-1L
#define ERR_INVALID_SIZE 			-2L
#define ERR_STRIDE_MISSMATCH		-3L
#define ERR_BOUNDSCHECK_FAILED		-4L
#define ERR_INTEGRITY_CHECK_FAILED	-5L

#define REQUIRED_SSE_BYTE_ALLIGNEMENT			16
#define ALLOCATOR_HEADER_SIGNITURE				0xCDECCDEC
#define ALLOCATOR_BOUNDS_CHECKER_CODE			0x0BADBEEF

// we need byte precision when we define this struct
#pragma pack(push,1)

//size of this struct need to be multiple of 16
struct MemoryHeader
{
	CRITICAL_SECTION	TransactionLock;			// in case you need this buffer to be threadsafe then you can lock deallocator until jobs finish using the buffer
	unsigned int		HeaderSigniture;			// In case we are not using transaction lock we can use as a very very low security measure
	unsigned int		SizeOfAllocedBuffer;		// Size of the actual requested buffer. Could be used by external tool for bounds checking
	unsigned char		Alligment;					// We can create specialized functions when memory is alligned that can be up to 2 times faster then non alligned. Not implemented for now
	unsigned int		SizeOfBoundsChecker;		// add a row / col to the buffer so we can check if we overwrote something
	unsigned char		HeaderStuffing[11];			// stuff header to be multiple of 16. This is barbaric hardcoding
};

//get rid of breaking compiler alligment. Let compiler generate alligned reading
#pragma pack(pop)

int		_AllocateMemory( unsigned char **OutputBuffer, int BufferSize, int Stride );
int		_AllocateRGBMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride );
int		_AllocateYUVMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride );
int		_FreeMemory( void * );
int		_FreeMemoryWaitForTransactions( void * );
int		_MemorySpinLockEnterTransaction( void * );
int		_MemorySpinLockExitTransaction( void * );

#endif