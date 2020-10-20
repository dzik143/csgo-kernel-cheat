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
#include <ntstrsafe.h>
#include "Debug.h"
#include "Handlers.h"
#include "Utils.h"

//
// Global variables.
//

//
// PID of csgo.exe process.
//

static DWORD TargetPid = 0;

//
// Create Handler. Related with CreateFile() used from ring3.
//
// - Find PID of target csgo.exe process
//

NTSTATUS CreateHandler(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  UNICODE_STRING csgoString;

  DBG_ENTER("CreateHandler");

  //
  // Get PID of CSGO.exe process.
  //


  DBG_MSG("Searching for 'csgo.exe' PID...\n");

  RtlInitUnicodeString(&csgoString, L"csgo.exe");

  TargetPid = FindProcessPid(&csgoString);

  FAIL(TargetPid == 0);

  DBG_MSG("'csgo.exe' PID is %d.\n", TargetPid);

  //
  // Error handler.
  //

  fail:

  DBG_LEAVE("CreateHandler");

  return ntStatus;
}

//
// Close Handler. Related with CloseHandle() used from ring3.
//

NTSTATUS CloseHandler(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  DBG_ENTER("CloseHandler");
  DBG_LEAVE("CloseHandler");

  return ntStatus;
}

//
// IOCTL Handler. Related with DeviceIoControl() used from ring3.
//

NTSTATUS IoControlHandler(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  DBG_ENTER("IoControlHandler");
  DBG_LEAVE("IoControlHandler");

  return ntStatus;
}

//
// Read Handler. Related with ReadFile() used from ring3.
//
// - Find client.dll module in csgo.exe process and read data from it.
//

NTSTATUS ReadHandler(PDEVICE_OBJECT deviceObject, PIRP Irp)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  PIO_STACK_LOCATION ioStackLocation = NULL;

  int len = 0;
  int i   = 0;

  BYTE click = 0;

  char *buf = NULL;

  int fireNeeded = -1;

  DBG_ENTER("ReadHandler");

  //
  // Get csgo PID if not found yet.
  //

  if (TargetPid == 0)
  {
    UNICODE_STRING csgoString;

    DBG_MSG("Searching for 'csgo.exe' PID...\n");

    RtlInitUnicodeString(&csgoString, L"csgo.exe");

    TargetPid = FindProcessPid(&csgoString);
  }

  if (TargetPid)
  {
    //
    // Find client.dll module in CSGO.exe's modules and read data from it.
    //

    fireNeeded = ReadFromClientModule(TargetPid);
  }

  //
  // Get user buffer.
  //

  ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

  len = ioStackLocation -> Parameters.Read.Length;
  buf = Irp -> UserBuffer;

  DBG_MSG("Read buf len is %d.\n", len);

  //
  // Fill user buffer.
  //

  if (len < sizeof(fireNeeded))
  {
    DBG_MSG("ERROR: User buffer too small to handle ClientData.\n");
  }
  else
  {
    RtlCopyMemory(buf, &fireNeeded, sizeof(fireNeeded));
  }

  DBG_LEAVE("ReadHandler");

  return ntStatus;
}

//
// Write Handler. Related with WriteFile() used from ring3.
// For test only.

NTSTATUS WriteHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
  NTSTATUS ntStatus = STATUS_SUCCESS;

  PIO_STACK_LOCATION ioStackLocation = NULL;

  char *buf = NULL;

  int len = 0;

  DBG_ENTER("WriteHandler");

  //
  // Get user buffer.
  //

  ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

  len = ioStackLocation -> Parameters.Read.Length;
  buf = Irp -> UserBuffer;

  //
  // Dump incoming data to log.
  //

  if (buf && len > 0 && buf[len - 1] == 0)
  {
    DBG_MSG("WriteHandler : Received [%s].\n", buf);
  }
  else
  {
    DBG_MSG("WriteHandler : Received [WRONG].\n");
  }

  DBG_LEAVE("WriteHandler");

  return ntStatus;
}

//
// Handler for unsupported functions.
//

NTSTATUS UnsupportedHandler(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  DBG_MSG("Unsupported function called.\n");

  return STATUS_NOT_SUPPORTED;
}
