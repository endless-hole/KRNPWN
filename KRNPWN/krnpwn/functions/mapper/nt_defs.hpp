#pragma once
namespace nt
{
	typedef enum class _POOL_TYPE
	{
		NonPagedPool,
		NonPagedPoolExecute,
		PagedPool,
		NonPagedPoolMustSucceed,
		DontUseThisType,
		NonPagedPoolCacheAligned,
		PagedPoolCacheAligned,
		NonPagedPoolCacheAlignedMustS,
		MaxPoolType,
		NonPagedPoolBase,
		NonPagedPoolBaseMustSucceed,
		NonPagedPoolBaseCacheAligned,
		NonPagedPoolBaseCacheAlignedMustS,
		NonPagedPoolSession,
		PagedPoolSession,
		NonPagedPoolMustSucceedSession,
		DontUseThisTypeSession,
		NonPagedPoolCacheAlignedSession,
		PagedPoolCacheAlignedSession,
		NonPagedPoolCacheAlignedMustSSession,
		NonPagedPoolNx,
		NonPagedPoolNxCacheAligned,
		NonPagedPoolSessionNx
	} POOL_TYPE;

	typedef enum _MEMORY_CACHING_TYPE_ORIG
	{
		MmFrameBufferCached = 2
	} MEMORY_CACHING_TYPE_ORIG;

	typedef enum _MEMORY_CACHING_TYPE
	{
		MmNonCached = FALSE,
		MmCached = TRUE,
		MmWriteCombined = MmFrameBufferCached,
		MmHardwareCoherentCached,
		MmNonCachedUnordered,       // IA64
		MmUSWCCached,
		MmMaximumCacheType,
		MmNotMapped = -1
	} MEMORY_CACHING_TYPE;

	typedef enum _MM_PAGE_PRIORITY
	{
		LowPagePriority,
		NormalPagePriority = 16,
		HighPagePriority = 32
	} MM_PAGE_PRIORITY;
}
