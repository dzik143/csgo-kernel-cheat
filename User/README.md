# Ring3 code.

This code open driver and reads client.dll data from it i.e.

  - finding csgo.exe PID is hidden in driver
  - finding client.dll module is hidden in driver
  - reading data from client.dll is hidden in ReadHandler in driver (used with ReadFile from ring3).
  - mouse clicking is still ring3 depending on data returned from driver.
