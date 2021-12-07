/*

Contains useful undocumented WIN NT structs

*/

#pragma once
#include <stdint.h>
#include <Windows.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

// wrapped within a namespace to avoid any conflicts

namespace nt
{
    #define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

    typedef struct _RTL_PROCESS_MODULE_INFORMATION
    {
        HANDLE Section;
        PVOID MappedBase;
        PVOID ImageBase;
        ULONG ImageSize;
        ULONG Flags;
        USHORT LoadOrderIndex;
        USHORT InitOrderIndex;
        USHORT LoadCount;
        USHORT OffsetToFileName;
        UCHAR FullPathName[ 256 ];
    } RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

    typedef struct _RTL_PROCESS_MODULES
    {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE_INFORMATION Modules[ 1 ];
    } RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

    typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

    using PEPROCESS = PVOID;

    using PsLookupProcessByProcessId = NTSTATUS( __fastcall* )(
        HANDLE     ProcessId,
        PEPROCESS* Process
        );

    typedef enum class SYSTEMINFOCLASS2
    {
        SystemBasicInformation2,
        SystemProcessorInformation2,             // obsolete...delete
        SystemPerformanceInformation2,
        SystemTimeOfDayInformation2,
        SystemPathInformation2,
        SystemProcessInformation2,
        SystemCallCountInformation2,
        SystemDeviceInformation2,
        SystemProcessorPerformanceInformation2,
        SystemFlagsInformation2,
        SystemCallTimeInformation2,
        SystemModuleInformation2,
        SystemLocksInformation2,
        SystemStackTraceInformation2,
        SystemPagedPoolInformation2,
        SystemNonPagedPoolInformation2,
        SystemHandleInformation2,
        SystemObjectInformation2,
        SystemPageFileInformation2,
        SystemVdmInstemulInformation2,
        SystemVdmBopInformation2,
        SystemFileCacheInformation2,
        SystemPoolTagInformation2,
        SystemInterruptInformation2,
        SystemDpcBehaviorInformation2,
        SystemFullMemoryInformation2,
        SystemLoadGdiDriverInformation2,
        SystemUnloadGdiDriverInformation2,
        SystemTimeAdjustmentInformation2,
        SystemSummaryMemoryInformation2,
        SystemMirrorMemoryInformation2,
        SystemPerformanceTraceInformation2,
        SystemObsolete02,
        SystemExceptionInformation2,
        SystemCrashDumpStateInformation2,
        SystemKernelDebuggerInformation2,
        SystemContextSwitchInformation2,
        SystemRegistryQuotaInformation2,
        SystemExtendServiceTableInformation2,
        SystemPrioritySeperation2,
        SystemVerifierAddDriverInformation2,
        SystemVerifierRemoveDriverInformation2,
        SystemProcessorIdleInformation2,
        SystemLegacyDriverInformation2,
        SystemCurrentTimeZoneInformation2,
        SystemLookasideInformation2,
        SystemTimeSlipNotification2,
        SystemSessionCreate2,
        SystemSessionDetach2,
        SystemSessionInformation2,
        SystemRangeStartInformation2,
        SystemVerifierInformation2,
        SystemVerifierThunkExtend2,
        SystemSessionProcessInformation2,
        SystemLoadGdiDriverInSystemSpace2,
        SystemNumaProcessorMap2,
        SystemPrefetcherInformation2,
        SystemExtendedProcessInformation2,
        SystemRecommendedSharedDataAlignment2,
        SystemComPlusPackage2,
        SystemNumaAvailableMemory2,
        SystemProcessorPowerInformation2,
        SystemEmulationBasicInformation2,
        SystemEmulationProcessorInformation2,
        SystemExtendedHandleInformation2,
        SystemLostDelayedWriteInformation2,
        SystemBigPoolInformation2,
        SystemSessionPoolTagInformation2,
        SystemSessionMappedViewInformation2,
        SystemHotpatchInformation2,
        SystemObjectSecurityMode2,
        SystemWatchdogTimerHandler2,
        SystemWatchdogTimerInformation2,
        SystemLogicalProcessorInformation2,
        SystemWow64SharedInformation2,
        SystemRegisterFirmwareTableInformationHandler2,
        SystemFirmwareTableInformation2,
        SystemModuleInformationEx2,
        SystemVerifierTriageInformation2,
        SystemSuperfetchInformation2,
        SystemMemoryListInformation2,
        SystemFileCacheInformationEx2,
        MaxSystemInfoClass2  // MaxSystemInfoClass should always be the last enum
    } SYSTEM_INFORMATION_CLASS2;

    typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_cm_partial_resource_descriptor

    #pragma pack(push,4)
    typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR
    {
        uint8_t  type;
        uint8_t  share_disposition;
        uint16_t flags;
        uint64_t address;
        uint32_t size;
        uint32_t pad;

    } CM_PARTIAL_RESOURCE_DESCRIPTOR, * PCM_PARTIAL_RESOURCE_DESCRIPTOR;
    #pragma pack(pop,4)

    typedef enum _INTERFACE_TYPE
    {
        InterfaceTypeUndefined,
        Internal,
        Isa,
        Eisa,
        MicroChannel,
        TurboChannel,
        PCIBus,
        VMEBus,
        NuBus,
        PCMCIABus,
        CBus,
        MPIBus,
        MPSABus,
        ProcessorInternal,
        InternalPowerBus,
        PNPISABus,
        PNPBus,
        Vmcs,
        ACPIBus,
        MaximumInterfaceType
    } INTERFACE_TYPE, * PINTERFACE_TYPE;


    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_cm_partial_resource_list

    typedef struct _CM_PARTIAL_RESOURCE_LIST
    {
        USHORT                         Version;
        USHORT                         Revision;
        ULONG                          Count;
        CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[ 1 ];
    } CM_PARTIAL_RESOURCE_LIST, * PCM_PARTIAL_RESOURCE_LIST;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_cm_full_resource_descriptor

    typedef struct _CM_FULL_RESOURCE_DESCRIPTOR
    {
        INTERFACE_TYPE           InterfaceType;
        ULONG                    BusNumber;
        CM_PARTIAL_RESOURCE_LIST PartialResourceList;
    } *PCM_FULL_RESOURCE_DESCRIPTOR, CM_FULL_RESOURCE_DESCRIPTOR;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_cm_resource_list

    typedef struct _CM_RESOURCE_LIST
    {
        ULONG                       Count;
        CM_FULL_RESOURCE_DESCRIPTOR List[ 1 ];
    } *PCM_RESOURCE_LIST, CM_RESOURCE_LIST;

    // https://github.com/microsoft/Windows-Driver-Frameworks/blob/main/src/publicinc/wdf/umdf/2.19/wudfwdm.h

    #define CmResourceTypeNull                0   // ResType_All or ResType_None (0x0000)
    #define CmResourceTypePort                1   // ResType_IO (0x0002)
    #define CmResourceTypeInterrupt           2   // ResType_IRQ (0x0004)
    #define CmResourceTypeMemory              3   // ResType_Mem (0x0001)
    #define CmResourceTypeDma                 4   // ResType_DMA (0x0003)
    #define CmResourceTypeDeviceSpecific      5   // ResType_ClassSpecific (0xFFFF)
    #define CmResourceTypeBusNumber           6   // ResType_BusNumber (0x0006)
    #define CmResourceTypeMemoryLarge         7   // ResType_MemLarge (0x0007)
    #define CmResourceTypeNonArbitrated     128   // Not arbitrated if 0x80 bit set
    #define CmResourceTypeConfigData        128   // ResType_Reserved (0x8000)
    #define CmResourceTypeDevicePrivate     129   // ResType_DevicePrivate (0x8001)
    #define CmResourceTypePcCardConfig      130   // ResType_PcCardConfig (0x8002)
    #define CmResourceTypeMfCardConfig      131   // ResType_MfCardConfig (0x8003)
    #define CmResourceTypeConnection        132   // ResType_Connection (0x8004)

    // CM_PARTIAL_RESOURCE_DESCRIPTOR Memory Large flags

    #define CmResourceTypeMemoryLarge40     0x200
    #define CmResourceTypeMemoryLarge48     0x400
    #define CmResourceTypeMemoryLarge64     0x800

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
