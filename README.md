# [ARCHIVE/2014] Kernel driver for CS:GO cheater
- **ARCHIVAL** code written in 2014,
- shows example how to **READ/WRITE PROCESS MEMORY** (user-mode, [ring 3](https://en.wikipedia.org/wiki/Protection_ring)) from **KERNEL DRIVER** (kernel-mode, [ring 0](https://en.wikipedia.org/wiki/Protection_ring)),
- tested on Win XP/32 only,
- **NOT** tested on later CS:GO versions (and for almost sure does not work anymore).

# What does kernel driver do?

  - create symbolic device *\\Device\\csgo-trigger*,
  - register driver handlers:
    - [IRP_MJ_READ](https://docs.microsoft.com/pl-pl/windows-hardware/drivers/kernel/irp-mj-read): find *csgo.exe* process, then **READ FROM ITS MEMORY**,
    - [IRP_MJ_WRITE](https://docs.microsoft.com/pl-pl/windows-hardware/drivers/kernel/irp-mj-write): find *csgo.exe* process, then **WRITE TO ITS MEMORY**.
  
# What does user-mode app do?

  - open kernel driver via symbolic name *\\.\csgo-trigger* (registered during [driver entry](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/writing-a-driverentry-routine) routine),
  - read from CS:GO memory by asking a driver, 
  - emit fake mouse click (fire weapon) depending on game state.
  
# How does user app communicate with kernel driver?

  - Kernel drivers can registers its **SYMBOLIC NAME** (*csgo-trigger*),
  - then user-app can comminicate with driver **LIKE WITH A FILE**:
    - call [CreateFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea) to **OPEN** a driver, then the driver receives **IRP_MJ_CREATE** request,
    - call [ReadFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile) to **READ** data delivered by driver, then the driver receives **IRP_MJ_READ** request and can pass arbitrary data in response,
    - call [WriteFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile) to **WRITE** own data to the driver, then the driver receives **IRP_MJ_WRITE** request and can process received data in arbitrary way,
    - call [CloseHandle](https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle) to **CLOSE** a driver handle if it's not needed anymore, then the driver receives **IRP_MJ_CLOSE** request.

