/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2014 Sylwester Wysocki <sw143@wp.pl>                         */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/*                                                                            */
/******************************************************************************/

#include <ntifs.h>
#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntddk.h>
#include <windef.h>
#include "Debug.h"
#include "Utils.h"

#define MY_POOLTAG 'kizD'

//
// Structs.
//

typedef struct _PEB_LDR_DATA
{
  BYTE Reserved1[8];
  PVOID Reserved2[3];
  LIST_ENTRY InMemoryOrderModuleList;
}
PEB_LDR_DATA, *PPEB_LDR_DATA;


typedef struct _DLLLISTENTRY
{
  LIST_ENTRY lEntry;

  DWORD dwBase;

  char szDllName[MAX_PATH];
}
DLLLISTENTRY, *PDLLLISTENTRY;


typedef struct _LDR_DATA_TABLE_ENTRY
{
  BYTE Reserved1[2];
  LIST_ENTRY InMemoryOrderLinks;
  PVOID Reserved2[2];
  PVOID DllBase;
  PVOID Reserved3[2];
  UNICODE_STRING FullDllName;
  BYTE Reserved4[8];
  PVOID Reserved5[3];

  union
  {
    ULONG CheckSum;
    PVOID Reserved6;
  };
  ULONG TimeDateStamp;
}
LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
  BYTE Reserved1[16];
  PVOID Reserved2[10];
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
}
RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB
{
  BYTE Reserved1[2];
  BYTE BeingDebugged;
  BYTE Reserved2[1];
  PVOID Reserved3[2];
  PPEB_LDR_DATA Ldr;
  PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
  BYTE Reserved4[104];
  PVOID Reserved5[52];
  /*PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;*/
  PVOID PostProcessInitRoutine;
  BYTE Reserved6[128];
  PVOID Reserved7[1];
  ULONG SessionId;
}
PEB, *PPEB;

typedef struct _SYSTEM_THREAD
{
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER CreateTime;
  ULONG WaitTime;
  PVOID StartAddress;
  CLIENT_ID ClientId;
  KPRIORITY Priority;
  LONG BasePriority;
  ULONG ContextSwitchCount;
  ULONG State;
  KWAIT_REASON WaitReason;
}
SYSTEM_THREAD, *PSYSTEM_THREAD;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
  ULONG             NextEntryDelta;
  ULONG             ThreadCount;
  ULONG             Reserved1[6];
  LARGE_INTEGER     CreateTime;
  LARGE_INTEGER     UserTime;
  LARGE_INTEGER     KernelTime;
  UNICODE_STRING    ProcessName;
  KPRIORITY         BasePriority;
  ULONG             ProcessId;
  ULONG             InheritedFromProcessId;
  ULONG             HandleCount;
  ULONG             Reserved2[2];
  VM_COUNTERS       VmCounters;
#if _WIN32_WINNT >= 0x500
  IO_COUNTERS       IoCounters;
#endif
  SYSTEM_THREAD     Threads[1];
}
SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

//
// Typedef.
//

typedef enum _SYSTEM_INFORMATION_CLASS
{
  SystemInformationClassMin = 0,
  SystemBasicInformation = 0,
  SystemProcessorInformation = 1,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemPathInformation = 4,
  SystemNotImplemented1 = 4,
  SystemProcessInformation = 5,
  SystemProcessesAndThreadsInformation = 5,
  SystemCallCountInfoInformation = 6,
  SystemCallCounts = 6,
  SystemDeviceInformation = 7,
  SystemConfigurationInformation = 7,
  SystemProcessorPerformanceInformation = 8,
  SystemProcessorTimes = 8,
  SystemFlagsInformation = 9,
  SystemGlobalFlag = 9,
  SystemCallTimeInformation = 10,
  SystemNotImplemented2 = 10,
  SystemModuleInformation = 11,
  SystemLocksInformation = 12,
  SystemLockInformation = 12,
  SystemStackTraceInformation = 13,
  SystemNotImplemented3 = 13,
  SystemPagedPoolInformation = 14,
  SystemNotImplemented4 = 14,
  SystemNonPagedPoolInformation = 15,
  SystemNotImplemented5 = 15,
  SystemHandleInformation = 16,
  SystemObjectInformation = 17,
  SystemPageFileInformation = 18,
  SystemPagefileInformation = 18,
  SystemVdmInstemulInformation = 19,
  SystemInstructionEmulationCounts = 19,
  SystemVdmBopInformation = 20,
  SystemInvalidInfoClass1 = 20,
  SystemFileCacheInformation = 21,
  SystemCacheInformation = 21,
  SystemPoolTagInformation = 22,
  SystemInterruptInformation = 23,
  SystemProcessorStatistics = 23,
  SystemDpcBehaviourInformation = 24,
  SystemDpcInformation = 24,
  SystemFullMemoryInformation = 25,
  SystemNotImplemented6 = 25,
  SystemLoadImage = 26,
  SystemUnloadImage = 27,
  SystemTimeAdjustmentInformation = 28,
  SystemTimeAdjustment = 28,
  SystemSummaryMemoryInformation = 29,
  SystemNotImplemented7 = 29,
  SystemNextEventIdInformation = 30,
  SystemNotImplemented8 = 30,
  SystemEventIdsInformation = 31,
  SystemNotImplemented9 = 31,
  SystemCrashDumpInformation = 32,
  SystemExceptionInformation = 33,
  SystemCrashDumpStateInformation = 34,
  SystemKernelDebuggerInformation = 35,
  SystemContextSwitchInformation = 36,
  SystemRegistryQuotaInformation = 37,
  SystemLoadAndCallImage = 38,
  SystemPrioritySeparation = 39,
  SystemPlugPlayBusInformation = 40,
  SystemNotImplemented10 = 40,
  SystemDockInformation = 41,
  SystemNotImplemented11 = 41,
  /* SystemPowerInformation = 42, Conflicts with POWER_INFORMATION_LEVEL 1 */
  SystemInvalidInfoClass2 = 42,
  SystemProcessorSpeedInformation = 43,
  SystemInvalidInfoClass3 = 43,
  SystemCurrentTimeZoneInformation = 44,
  SystemTimeZoneInformation = 44,
  SystemLookasideInformation = 45,
  SystemSetTimeSlipEvent = 46,
  SystemCreateSession = 47,
  SystemDeleteSession = 48,
  SystemInvalidInfoClass4 = 49,
  SystemRangeStartInformation = 50,
  SystemVerifierInformation = 51,
  SystemAddVerifier = 52,
  SystemSessionProcessesInformation     = 53,
  SystemInformationClassMax
}
SYSTEM_INFORMATION_CLASS;

typedef HANDLE (*PSGETPROCESSID)(PEPROCESS process);

typedef PPEB (*PSGETPROCESSPB)(PEPROCESS process);

//
// External functions.
//

NTSTATUS NTAPI ZwQuerySystemInformation(UINT SystemInformationClass,
                                        PVOID SystemInformation,
                                        ULONG SystemInformationLength,
                                        PULONG ReturnLength);

//
// Find process ID with given image name.
//
// processName - name of process image to find e.g. csgo.exe (IN).
//
// RETURNS: Process ID or
//          0 if error or process not found.
//

DWORD FindProcessPid(UNICODE_STRING *processName)
{
  DWORD pid = 0;

  SYSTEM_PROCESS_INFORMATION *processInfo = NULL;
  SYSTEM_PROCESS_INFORMATION *currentProc = NULL;

  ULONG processInfoSize = 1024 * 32;

  NTSTATUS ntStatus = 0;

  DBG_ENTER("FindProcessPid");

  //
  // Init guess 32k buffer for processes info data.
  //

  processInfo = (SYSTEM_PROCESS_INFORMATION *)
                    ExAllocatePoolWithTag(NonPagedPool,
                                              processInfoSize,
                                                  MY_POOLTAG);

  FAILEX(processInfo == NULL, "ERROR: Out of memory.\n");

  //
  // Get size of processInfo[] table.
  //

  DBG_MSG("Retrieving processInfo[] size...\n");

  ZwQuerySystemInformation(SystemProcessInformation, processInfo,
                               processInfoSize, &processInfoSize);

  //
  // Reellocate processInfo[] buffer if guess 32k is too small.
  //

  if (processInfoSize > 32 * 1024)
  {
    DBG_MSG("Reallocating processInfo[] to [%d] bytes...\n", processInfoSize);

    ExFreePool(processInfo);

    processInfo = (SYSTEM_PROCESS_INFORMATION *)
                      ExAllocatePoolWithTag(NonPagedPool,
                                                processInfoSize,
                                                    MY_POOLTAG);

    FAILEX(processInfo == NULL, "ERROR: Out of memory.\n");
  }

  //
  // Retrieve processInfo[] table.
  //

  DBG_MSG("Retrieving processInfo[] table...\n");

  NTFAIL(ZwQuerySystemInformation(SystemProcessInformation, processInfo,
                                      processInfoSize, &processInfoSize));

  //
  // Iterate over all processes.
  // Search for target process.
  //

  currentProc = processInfo;

  while(pid == 0 && currentProc -> NextEntryDelta)
  {
    DBG_MSG("PID #%8d : ", currentProc -> ProcessId);

    DBG_MSG("%.*ls\n",
                currentProc -> ProcessName.Length,
                    currentProc -> ProcessName.Buffer);

    //
    // Compare process names.
    //

    if (processName -> Length == currentProc -> ProcessName.Length
          && RtlEqualMemory(processName -> Buffer,
                                currentProc -> ProcessName.Buffer,
                                    processName -> Length))
    {
      DBG_MSG("Found target PID #%d.\n", currentProc -> ProcessId);

      pid = currentProc -> ProcessId;
    }

    //
    // Go to next process in list.
    //

    currentProc = (PSYSTEM_PROCESS_INFORMATION)
                      ((PBYTE)(currentProc) + currentProc -> NextEntryDelta);
  }

  //
  // Clean up.
  //

  fail:

  if (pid == 0)
  {
    DBG_MSG("ERROR: Cannot get PID of process '%.*ls'.\n",
                 processName -> Length, processName -> Buffer);
  }

  if (processInfo)
  {
    ExFreePool(processInfo);
  }

  DBG_LEAVE("FindProcessPid");

  return pid;
}

//
// Get pointer to local player from Client!0x9dcd04.
//
// clientDllBase - base where Client.dll loaded (IN).
//
// RETURNS: Pointer to LocalPlayer or
//          NULL if error.
//

DWORD GetLocalPlayer(void *clientDllBase)
{
  DWORD localPlayer = 0;

  PDWORD ptr = (PDWORD) ((PCHAR) (clientDllBase) + 0x9DCD04);

  DBG_ENTER("GetLocalPlayer");

  if (MmIsAddressValid(ptr))
  {
    localPlayer = *ptr;

    DBG_MSG("LocalPlayer = [0x%x]\n.", localPlayer);
  }
  else
  {
    DBG_MSG("ERROR: Cannot read LocalPlayer from %p.\n", ptr);
  }

  DBG_LEAVE("GetLocalPlayer");

  return localPlayer;
}

//
// Get pointers table of another players from client!0xa871d4 address.
//
// players       - table where to store retrieved pointers (OUT).
// clientDllBase - base where Client.dll loaded (IN).
//

void GetPlayers(DWORD players[64], void *clientDllBase)
{
  DWORD dwPlayerAddress = 0;

  int i = 0;

  DBG_ENTER("GetPlayers");

  for(i = 0; i < 64; i ++)
  {
    PDWORD ptr = (PDWORD) ((PCHAR) clientDllBase + 0xA871D4 + (i * 0x10));

    if (MmIsAddressValid(ptr))
    {
      players[i] = *ptr;

      DBG_MSG("Player[%d] = [0x%x]\n.", i, players[i]);
    }
    else
    {
      DBG_MSG("ERROR: Cannot read Player[%d] from %p.\n", i, ptr);
    }
  }

  DBG_LEAVE("GetPlayers");
}

//
// Get incross ID for local player.
//
// player - pointer to player retrieved from GetLocalPlayer() before (IN).
//
// RETURNS: InCrossId or
//          -1 if error.
//

int GetInCrossId(DWORD player)
{
  int inCrossId = -1;

  PDWORD ptr = (PDWORD) ((PCHAR) player + 0x23B4);

  DBG_ENTER("GetInCrossId");

  if (MmIsAddressValid(ptr))
  {
    inCrossId = *ptr;

    DBG_MSG("InCrossId = [%d].\n", inCrossId);
  }
  else
  {
    DBG_MSG("ERROR: Cannot read InCrossId from %p.\n", ptr);
  }

  DBG_LEAVE("GetInCrossId");

  return inCrossId;
}

//
//
//

int GetTeam(DWORD player)
{
  int iTeam = 0;

  PDWORD ptr = (PDWORD) ((PCHAR) player + 0xf0);

  DBG_ENTER("GetTeam");

  if (MmIsAddressValid(ptr))
  {
    iTeam = *ptr;

    DBG_MSG("Team for player PTR %x = [%d].\n", player, iTeam);
  }
  else
  {
    DBG_MSG("ERROR: Cannot read team for player %x from PTR %p.\n", player, ptr);
  }

  DBG_LEAVE("GetTeam");

  return iTeam;
}

//
// - Iterate modules loaded by target process
// - find base of 'client.dll'
// - read data from 'client.dll' module.
//
// RETURNS: 0 if OK and fire needed,
//          1 if OK and fire not needed,
//         -1 if error.
//

int ReadFromClientModule(DWORD pid)
{
  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

  PEPROCESS pEproc = NULL;

  PKAPC_STATE pKapcState = NULL;

  PPEB pPeb = NULL;

  PLIST_ENTRY pDllListHead = NULL;

  UNICODE_STRING usMethodName;

  PSGETPROCESSPB pPsGetProcessPeb = NULL;

  PLIST_ENTRY pDllListEntry = NULL;

  PLDR_DATA_TABLE_ENTRY pDll = NULL;

  int exitCode = -1;

  int fireNeeded = 0;

  DBG_ENTER("ReadFromClientModule");

  //
  // Check args.
  //

  FAILEX(pid == 0, "ERROR: Wrong target PID#0.\n");

  //
  // Get the EPROCESS pointer for target process.
  //

  DBG_MSG("Retrieving EPROCESS pointer for process ID#%d...\n", pid);

  NTFAIL(PsLookupProcessByProcessId((HANDLE) pid, &pEproc));

  FAIL(MmIsAddressValid(pEproc) == FALSE);

  //
  //
  //

  pKapcState = ExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC_STATE), MY_POOLTAG);

  FAIL(MmIsAddressValid(pKapcState) == FALSE);

  //
  // Get the address of PsGetProcessPeb
  //

  DBG_MSG("Importing PsGetProcessPeb()...\n");

  RtlInitUnicodeString(&usMethodName, L"PsGetProcessPeb" );

  pPsGetProcessPeb = (PSGETPROCESSPB) MmGetSystemRoutineAddress(&usMethodName);

  FAIL(MmIsAddressValid(pPsGetProcessPeb) == FALSE);

  //
  // Get pointer to PEB of target process.
  //

  DBG_MSG("Getting pointer to PEB...\n");

  pPeb = pPsGetProcessPeb(pEproc);

  //
  // Attach to process's stack
  //

  DBG_MSG("Attaching to target process...\n");

  KeStackAttachProcess(pEproc, pKapcState);

  FAIL(MmIsAddressValid(pPeb) == FALSE);

  //
  // Get DLL list from PEB.
  //

  pDllListHead = &((PPEB_LDR_DATA)(pPeb -> Ldr)) -> InMemoryOrderModuleList;

  FAIL(MmIsAddressValid(pDllListHead) == FALSE);

  //
  // Walk thorugh DLL list
  //

  pDllListEntry = pDllListHead->Flink;

  while(MmIsAddressValid(pDllListEntry) && (pDllListEntry != pDllListHead))
  {
    //
    // Get each DLL list entry
    //

    pDll = (PLDR_DATA_TABLE_ENTRY)
               CONTAINING_RECORD(pDllListEntry,
                                     LDR_DATA_TABLE_ENTRY,
                                         InMemoryOrderLinks);

    if(MmIsAddressValid(pDll))
    {
      int i         = 0;
      int lastSlash = 0;

      char dllName[1024];

      RtlStringCchPrintfA(dllName, 1024 - 1, "%S", pDll -> FullDllName.Buffer);

      DBG_MSG("[%s][%p]\n", dllName, pDll -> DllBase);

      //
      // Remove path prefix from image name.
      //

      for (i = 0; dllName[i]; i++)
      {
        if (dllName[i] == '\\')
        {
          lastSlash = i;
        }
      }

      //
      // Compare image name with 'client.dll'.
      //

      if (RtlEqualMemory(dllName + lastSlash + 1, "client.dll", sizeof("client.dll")))
      {
        DWORD localPlayer = 0;

        DWORD players[64] = {0};

        DWORD clientBase = (DWORD) pDll -> DllBase;

        int inCrossId = -1;

        DBG_MSG("Client.dll found at base '0x%x'.\n", pDll -> DllBase);

        //
        // Compute data pointers in client.dll.
        //

        localPlayer = GetLocalPlayer((PVOID) clientBase);

        GetPlayers(players, (PVOID) clientBase);

        inCrossId = GetInCrossId(localPlayer);

        //
        // We need to make sure iInCrossId is a valid player ID so we check here.
        //

        fireNeeded = 0;

        if(inCrossId > 0 && inCrossId <= 64 )
        {
          // If their is a player in the crosshair we need to make sure they're an enemy.
          // Because player Ids don't start at 0 we need to minus one from the variable and then get that player entity's address out of dwPlayers and compare our team ID with the player in our crosshairs team ID.
          // If they're not on our team, they're evil and must die.
          if(GetTeam(localPlayer) != GetTeam(players[inCrossId - 1]))
          {
            fireNeeded = 1;

            DBG_MSG("Fire needed.\n");
          }
          else
          {
            fireNeeded = 0;

            DBG_MSG("Fire NOT needed.\n");
          }
        }

        //
        // Don't list others DLLs.
        //

        break;
      }

      //
      // Next DLL.
      //

      pDllListEntry = pDllListEntry -> Flink;
    }
  }

  //
  // Set up exit code.
  //
  //  0 = fire needed
  //  1 = fire NOT needed
  // -1 = internal error
  //

  if (fireNeeded)
  {
    exitCode = 0;
  }
  else
  {
    exitCode = 1;
  }

  //
  // Clean up.
  //

  fail:

  //
  // Detach from process's stack
  //

  if (MmIsAddressValid(pKapcState))
  {
    KeUnstackDetachProcess(pKapcState);
  }

  if (MmIsAddressValid(pKapcState))
  {
    ExFreePool(pKapcState);
  }

  //
  // Dereference EPROCESS pointer
  //

  if (MmIsAddressValid(pEproc))
  {
    ObDereferenceObject(pEproc);
  }

  DBG_LEAVE("ReadFromClientModule");

  return exitCode;
}
