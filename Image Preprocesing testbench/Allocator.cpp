#include "Allocator.h"
#include <assert.h>

int		AllocateMemory( unsigned char **OutputBuffer, int BufferSize, int BoundsCheckerSize )
{
	assert( ( sizeof( MemoryHeader ) & ( REQUIRED_SSE_BYTE_ALLIGNEMENT - 1 ) ) == 0 );
	//we return 0 by default unless it is a success
	*OutputBuffer = NULL;
	//needs to be multiple of 4
	BoundsCheckerSize = ( ( BoundsCheckerSize / 4 + 1 ) * 4 ); 
	int AllocSize = sizeof( MemoryHeader ) + BufferSize + BoundsCheckerSize;
	void *Buff =  _aligned_malloc( AllocSize, REQUIRED_SSE_BYTE_ALLIGNEMENT );
	if( Buff == NULL )
		return ERR_BUFFER_NULL;
	MemoryHeader	*Header = (MemoryHeader	*)Buff;
	memset( Header, 0, sizeof( MemoryHeader ) );
	Header->HeaderSigniture = ALLOCATOR_HEADER_SIGNITURE;
	Header->SizeOfAllocedBuffer = BufferSize;
	Header->Alligment = REQUIRED_SSE_BYTE_ALLIGNEMENT;
	Header->SizeOfBoundsChecker = BoundsCheckerSize;
	//init Bounds Checker
	char *PBoundsChecker = (char *)Buff + sizeof( MemoryHeader );
	PBoundsChecker += Header->SizeOfAllocedBuffer;
	for( unsigned int i=0;i<Header->SizeOfBoundsChecker;i+=4)
		*(int*)&PBoundsChecker[i] = ALLOCATOR_BOUNDS_CHECKER_CODE;
	//intialize critical section
	InitializeCriticalSection( &Header->TransactionLock );
	//set output 
	*OutputBuffer = (unsigned char *)Buff + sizeof( MemoryHeader );
	return 0;
}

int		_AllocateRGBMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride )
{
	//safety checks for parameters
	if( Width <= 0 || Height <= 0 || Stride <= 0 )
		return ERR_INVALID_SIZE;
	if( Width * 3 > Stride )
		return ERR_INVALID_SIZE;
	if( BufferSize < Height * Stride )
		return ERR_INVALID_SIZE;
	return AllocateMemory( OutputBuffer, BufferSize, Stride );
}

int		_AllocateYUVMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride )
{
	//safety checks for parameters
	if( Width <= 0 || Height <= 0 || Stride <= 0 )
		return ERR_INVALID_SIZE;
	if( Width > Stride )
		return ERR_INVALID_SIZE;
	int RGB24Size = Width * Height * 3;		//we will use this. In case people will accidentally swap a YUV buffer with a RGB24 buffer we will not cause a memory corruption
	int YUVSize = Height * Stride + ( Height / 2 ) * ( Stride / 2 ) * 2; //we only need so much
	if( BufferSize < YUVSize )
		return ERR_INVALID_SIZE;
	int AllocBufferSize = max( RGB24Size, BufferSize );
	return AllocateMemory( OutputBuffer, AllocBufferSize, AllocBufferSize - YUVSize + Stride );
}

int HeaderIntegrityCheck( MemoryHeader *Header )
{
	//check for header integrity to avoid double dealloc
	if( Header->Alligment != REQUIRED_SSE_BYTE_ALLIGNEMENT )
		return ERR_INTEGRITY_CHECK_FAILED;
	if( Header->HeaderSigniture != ALLOCATOR_HEADER_SIGNITURE )
		return ERR_INTEGRITY_CHECK_FAILED;
	return 0;
}

int		_FreeMemory( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	//check bounds 
	bool BoundsCheckerResult = true;
	char *PBoundsChecker = (char *)Buff;
	PBoundsChecker += Header->SizeOfAllocedBuffer;
	for( unsigned int i=0;i<Header->SizeOfBoundsChecker;i+=4)
		if( *(int*)&PBoundsChecker[i] != ALLOCATOR_BOUNDS_CHECKER_CODE )
		{
			BoundsCheckerResult = false;
			break;
		}

	//destroy header 
	Header->Alligment = 0;
	Header->HeaderSigniture = 0;
	DeleteCriticalSection( &Header->TransactionLock );
	Header->SizeOfAllocedBuffer = 0;
	Header->SizeOfBoundsChecker = 0;

	//destroy buffer
	_aligned_free( Header );

	//signal if we corrupted memory or not
	if( BoundsCheckerResult == false )
		return ERR_BOUNDSCHECK_FAILED;

	return 0;
}

int		_FreeMemoryWaitForTransactions( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	//request lock on the buffer 
	EnterCriticalSection( &Header->TransactionLock );
	//when we get here other threads finished using the buffer
	LeaveCriticalSection( &Header->TransactionLock );

	//if other threads released the buffer then we assume it is safe to release it
	//!! note that we can't lock the buffer we are releasing, because we deallocate the lock also !
	return _FreeMemory( Buff );
}

int		_MemorySpinLockEnterTransaction( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	EnterCriticalSection( &Header->TransactionLock );
	return 0;
}

int		_MemorySpinLockExitTransaction( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	LeaveCriticalSection( &Header->TransactionLock );
	return 0;
}