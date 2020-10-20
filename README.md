# [ARCHIVE/2014] Kernel driver for CS:GO cheater
- **ARCHIVAL** code written in 2014,
- shows example how to **READ/WRITE PROCESS MEMORY** (user-mode, [ring 3](https://en.wikipedia.org/wiki/Protection_ring)) from **KERNEL DRIVER** (kernel-mode, [ring 0](https://en.wikipedia.org/wiki/Protection_ring)),
- tested on Win XP/32 only,
- **NOT** tested on later CS:GO versions (and for almost sure does not work anymore).

# What does kernel driver do?

  - create symbolic device *\\Device\\csgo-trigger*,
  - register driver handlers:
    - [IRP_MJ_CREATE](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-create): finds out **CSGO.EXE PID**,
    - [IRP_MJ_READ](https://docs.microsoft.com/pl-pl/windows-hardware/drivers/kernel/irp-mj-read): finds out *client.dll*, then **READ FROM ITS MEMORY**,
    - [IRP_MJ_WRITE](https://docs.microsoft.com/pl-pl/windows-hardware/drivers/kernel/irp-mj-write): finds out *client.dll*, then **WRITE TO ITS MEMORY**,
  

# What does user-mode app do?

  - open kernel driver via symbolic name *\\.\csgo-trigger* (registered during [driver entry](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/writing-a-driverentry-routine) routine),
  - read from CS:GO memory by asking a driver, 
  - emit fake mouse click (fire weapon) depending on game state.
  
# How does user app commucate with kernel driver?

  - Kernel drivers can registers its **SYMBOLIC NAME** (*csgo-trigger*),
  - user-app can use this name open a driver using [CreateFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea) - then the driver receives **IRP_MJ_CREATE** request,
  - user-app can reads data delivered by driver using [ReadFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile) - then the driver receives **IRP_MJ_READ** request and can pass arbitrary data in response,
  - user-app can writes own data to the driver using [WriteFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile) - then the driver receives **IRP_MJ_WRITE** request and can process received data in arbitrary way.

