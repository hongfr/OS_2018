#include "syscall.h"
int main(void)
{
    int fileID= Open("file0.test");
    int fileID2= Open("file0.test");
    if (fileID2==-1) MSG("Failed on duplicate opening file");
    MSG("Duplicate success on opening file0.test");
    if (fileID==-1) MSG("Failed on opening file");
    MSG("Success on opening file0.test");
    Halt();
}
