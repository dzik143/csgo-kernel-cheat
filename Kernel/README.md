# Kernel driver.

Driver does:

  - Find csgo.exe PID on OpenHandler (called when CreateFile called in ring3).

  - Find client.dll, open and read data from it on ReadHandler (called when
    ReadFile called in ring3). Data is packed into ClientData struct.

