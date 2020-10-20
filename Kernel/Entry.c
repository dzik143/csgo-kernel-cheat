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

#include <wdm.h>
#include <windef.h>
#include "Handlers.h"
#include "Debug.h"

#define DEVICE_NAME     L"\\Device\\csgo-trigger"
#define DOS_DEVICE_NAME L"\\DosDevices\\csgo-trigger"

VOID UnloadHandler(PDRIVER_OBJECT driverObject);

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);

/*
 * These compiler directives tell the Operating System how to load the
 * driver into memory. The "INIT" section is discardable as you only
 * need the driver entry upon initialization, then it can be discarded.
 */

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, UnloadHandler)

//
// Driver entry point.
//
// - Create device to open driver from ring3.
// - Create DOS symbolic name for device to use it with CreateFile.
// - Set up drivers handlers.
//
// RETURNS: STATUS_SUCCESS if OK.
//

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  UINT i = 0;

  PDEVICE_OBJECT deviceObject = NULL;

  UNICODE_STRING driverName;
  UNICODE_STRING dosDeviceName;

  DBG_ENTER("DriverEntry");

  //
  // Create DOS device to open driver from ring3.
  //

  RtlInitUnicodeString(&driverName, DEVICE_NAME);
  RtlInitUnicodeString(&dosDeviceName, DOS_DEVICE_NAME);

  DBG_MSG("Creating device...\n");

  NTFAIL(IoCreateDevice(driverObject, 0, &driverName, FILE_DEVICE_UNKNOWN,
                            FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject));

  deviceObject -> Flags &= (~DO_DEVICE_INITIALIZING);

  //
  // Create a Symbolic Link to the device.
  //

  DBG_MSG("Creating device symbolic name...\n");

  IoCreateSymbolicLink(&dosDeviceName, &driverName);

  //
  // Set up drivers handlers.
  //

  for(i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
  {
    driverObject -> MajorFunction[i] = UnsupportedHandler;
  }

  driverObject -> MajorFunction[IRP_MJ_CLOSE]          = CloseHandler;
  driverObject -> MajorFunction[IRP_MJ_CREATE]         = CreateHandler;
  driverObject -> MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControlHandler;
  driverObject -> MajorFunction[IRP_MJ_READ]           = ReadHandler;
  driverObject -> MajorFunction[IRP_MJ_WRITE]          = WriteHandler;

  driverObject -> DriverUnload = UnloadHandler;

  //
  // Error handler.
  //

  ntStatus = STATUS_SUCCESS;

  fail:

  DBG_LEAVE("DriverEntry");

  return ntStatus;
}

//
// Unload handler.
//
// - Remove DOS symbolic link.
// - Remove device.
//
// driverObject - pointer to driver object received in DriverEntry before (IN).
//

VOID UnloadHandler(PDRIVER_OBJECT driverObject)
{
  UNICODE_STRING dosDeviceName;

  DBG_ENTER("UnloadHandler");

  RtlInitUnicodeString(&dosDeviceName, DOS_DEVICE_NAME);

  DBG_MSG("Deleting dos device...\n");

  IoDeleteSymbolicLink(&dosDeviceName);

  DBG_MSG("Deleting device object...\n");

  IoDeleteDevice(driverObject -> DeviceObject);

  DBG_LEAVE("UnloadHandler");
}
