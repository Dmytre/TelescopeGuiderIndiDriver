#include "COMInteract.h"
#include "indicom.h"
#include "string.h"


COMPort::COMPort()
{
    comHandle = COMID_UNUSED;
    baudRate = 9600;
}

int COMPort::Initialize(const char *portId)
{
    if(TTY_OK != tty_connect(portId, baudRate, 8, 0, 1, &comHandle))
    {
        return RES_NOK;
    }
    
    return RES_OK;
}

int COMPort::Close()
{
    tty_disconnect(comHandle);
    comHandle = COMID_UNUSED;
    
    return RES_OK;
}

int COMPort::SendCommand(const char* command)
{
    int bytesWritten = 0;
    int result = TTY_OK;
    
    result = tty_write(comHandle, command, strlen(command), &bytesWritten);
    
    if((TTY_OK != result) || (bytesWritten != strlen(command)))
    {
        /* Something wrong */
        return RES_NOK;
    }
  
    return RES_OK;
}

int COMPort::ReadResult(char* resultBuff_p, int* length_p)
{
    
    if(comHandle <= 0)
    {
        return RES_BADSTATE;
    }
    
    int readResult = TTY_OK;

    *length_p = 0;
//    readResult = tty_read_section(comHandle, resultBuff_p, '#', 1, length_p);
    readResult = tty_read_section(comHandle, resultBuff_p, '\n', 1, length_p);
    
    if(TTY_OK != readResult)
    {
        /* Something went wrong */
        return RES_NOK;
    }
    
    /* Drop the \n symbol */
    *length_p -= 1;
     
    return RES_OK;
}

