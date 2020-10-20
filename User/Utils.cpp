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

#include <windows.h>
#include "Debug.h"
#include "Defines.h"

//
// Transform relative path into full path.
// E.g. it transforms "/somedir" into "<where currnt bin lives>/somedir".
//
// normPath - full, generated path (OUT).
// pathSize - size of path buffer in bytes (IN).
// relative - relative path postfix to add to base path (IN).
// quiet    - do not write logs if set to 1 (IN).
//
// RETURNS: 0 if OK.
//

int ExpandRelativePath(char *path, int pathSize, const char *relative)
{
  int exitCode = -1;

  int lastSlash = 0;

  //
  // Check is it already absolute?
  //

  DBG_MSG("Expanding path [%s]...\n", relative);

  //
  // Get full path to current binary.
  //

  FAIL(GetModuleFileName(NULL, path, pathSize) == FALSE);

  //
  // Remove filename from path.
  //

  for (int i = 0; path[i] && i < pathSize; i++)
  {
    if (path[i] == '\\' || path[i] == '/')
    {
      lastSlash = i;
    }
  }

  path[lastSlash] = 0;

  //
  // Add relative postfix if any.
  //

  if (relative)
  {
    strncat(path, "\\", pathSize);
    strncat(path, relative, pathSize);
  }

  exitCode = 0;

  DBG_MSG("Path [%s] expanded to [%s].\n", relative, path);

  //
  // Error handler.
  //

  fail:

  if (exitCode)
  {
    fprintf(stderr, "ERROR: Cannot expand path '%s'. Error is %d.\n",
                relative, GetLastError());
  }

  return exitCode;
}

//
// Load kernel driver.
//
// RETURNS: 0 if OK.
//

int LoadDriver()
{
  SC_HANDLE scman;

  SC_HANDLE service;

  SERVICE_STATUS ss;

  char driverPath[MAX_PATH] = {0};

  int exitCode = -1;

  //
  // Expand relative trigger.sys file to absolute path.
  //

  FAIL(ExpandRelativePath(driverPath, sizeof(driverPath) - 1, DRIVER_FILE));

  //
  // Open Service Manager.
  //

  scman = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

  FAILEX(scman == NULL,
             "ERROR: Cannot open service manager. Error is %d.\n",
                 GetLastError());

  //
  // Delete old driver if already exist.
  //

  service = OpenService(scman, DRIVER_NAME,
                            SERVICE_START | DELETE | SERVICE_STOP);
  if (service)
  {
    ControlService(service, SERVICE_CONTROL_STOP, &ss);

    DeleteService(service);

    CloseServiceHandle(service);

    DBG_MSG("Deleted old driver.\n");
  }

  //
  // Create driver service.
  //

  service = CreateService(scman, DRIVER_NAME, DRIVER_FULL_NAME,
                              SERVICE_START | DELETE | SERVICE_STOP,
                                  SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
                                      SERVICE_ERROR_IGNORE, driverPath,
                                          NULL, NULL, NULL, NULL, NULL);

  FAILEX(service == NULL,
             "ERROR: Cannot create service. Error is %d.\n",
                 GetLastError());

  //
  // Start driver service.
  //

  FAILEX(StartService(service, 0, NULL) == FALSE,
             "ERROR: Cannot start driver. Error code is %d.\n",
                 GetLastError());

  DBG_MSG("Driver loaded.\n");

  //
  // Clean up.
  //

  exitCode = 0;

  fail:

  CloseServiceHandle(service);
  CloseServiceHandle(scman);

  return exitCode;
}

//
// Unload kernel driver.
//
// RETURNS: 0 if OK.
//

void UnloadDriver()
{
  SC_HANDLE scman;

  SC_HANDLE service;

  SERVICE_STATUS ss;

  //
  // Open Service Manager.
  //

  scman = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

  FAILEX(scman == NULL,
             "ERROR: Cannot open service manager. Error is %d.\n",
                 GetLastError());

  //
  // Open driver service.
  //

  service = OpenService(scman, DRIVER_NAME,
                            SERVICE_START | DELETE | SERVICE_STOP);
  if (service)
  {
    ControlService(service, SERVICE_CONTROL_STOP, &ss);

    DeleteService(service);

    CloseServiceHandle(service);

    DBG_MSG("Driver unloaded.\n");
  }

  //
  // Clean up.
  //

  fail:

  CloseServiceHandle(service);
  CloseServiceHandle(scman);
}
