/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"

#include "synchconsole.h"

void SysHalt()
{
  kernel->interrupt->Halt();
}

void SysPrintInt(int val)
{
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, into synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
  kernel->synchConsoleOut->PutInt(val);
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, return from synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysCreate(char *filename)
{
  // return value
  // 1: success
  // 0: failed
  return kernel->fileSystem->Create(filename);
}
//edited------------------
OpenFileId SysOpen(char *filename)
{
  // return kernel->fileSystem->Open(filename);
  
  // kernel->fileSystem->Open(filename);
  if(kernel->fileSystem->Open(filename) == NULL)
    return -1;
  else
    return kernel->fileSystem->fileTableID;
}

int SysClose(OpenFileId id)
{
  // return kernel->fileSystem->Open(filename);
  
  // kernel->fileSystem->Open(filename);
  if(kernel->fileSystem->Close(id) < 0)
    return  -1;
  else
    return 1;

}

int SysWrite(char* from, int size, OpenFileId id)
{
  return kernel->fileSystem->WriteFile(from, size, id);
}
int SysRead(char* to, int size, OpenFileId id)
{
    return kernel->fileSystem->ReadFile(to, size, id);
}
//edited----------------
#endif /* ! __USERPROG_KSYSCALL_H__ */
