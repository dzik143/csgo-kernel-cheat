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

//
// Includes.
//

#include <cstdio>
#include <windows.h>
#include "Debug.h"
#include "Defines.h"
#include "Utils.h"

void FireWeapon()
{
  if(GetAsyncKeyState(FIRE_KEY) < 0)
  {
    //
    // Press the left mouse button.
    //

    mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0 );
    Sleep( 5 ); //Release it after five milliseconds

    mouse_event( MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );
    Sleep( 5 ); //Delay the next shot for five milliseconds.
  }
}

//
// Entry point.
//

int main()
{
  HANDLE driver;

  DWORD readed = 0;

  int status = -1;

  int exitCode = -1;

  //
  // Load driver into memory.
  //

  FAIL(LoadDriver());

  //
  // Open driver.
  // Driver should find csgo.exe PID if exist in CreateHandler.
  //

  driver = CreateFile("\\\\.\\csgo-trigger", GENERIC_READ | GENERIC_WRITE,
                          0, NULL, OPEN_EXISTING, 0, NULL);

  FAILEX(driver == INVALID_HANDLE_VALUE, "ERROR: Cannot open driver.\n");

  //
  // Wait for csgo app.
  //
  // Driver returns status, which means:
  //  0 = fire needed
  //  1 = fire NOT needed
  // -1 = error, CS:GO not running or internal error.
  //

  DBG_MSG("Waiting for CS:GO application...\n");

  while(status == -1)
  {
    ReadFile(driver, &status, sizeof(status), &readed, NULL);

    Sleep(50);
  }

  //
  // Fall into main loop.
  //

  DBG_MSG("CS:GO app detected. Failing into main loop...\n");

  while(true)
  {
    //
    // Read data from client.dll via driver.
    // Driver should return data in ClientData structure.
    //

    ReadFile(driver, &status, sizeof(status), &readed, NULL);

    if (status == 0)
    {
      FireWeapon();
    }
  }

  //
  // Error handler.
  //

  exitCode = 0;

  fail:

  CloseHandle(driver);

  UnloadDriver();

  system("pause");

  return 0;
}
