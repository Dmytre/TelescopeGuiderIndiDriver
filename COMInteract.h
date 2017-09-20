/* Here the functionality to initialize,     */
/* send and receive information via COM port */
/* will be implemented                       */
#ifndef __COMINTERACT_
#define __COMINTERACT_

#define COMM_GUIDE              "GUIDE#"
#define COMM_STOP               "STOP#"
#define COMM_GET_DEV_STATUS     "GET_DEV_STATUS#"
#define COMM_GET_GUIDING_STATUS "GET_GUIDING_STATUS#"

#define COMM_GUIDE_NORTH        "MOVE_DEC_POSITIVE"
#define COMM_GUIDE_SOUTH        "MOVE_DEC_NEGATIVE"
#define COMM_GUIDE_WEST         "MOVE_RA_POSITIVE"
#define COMM_GUIDE_EAST         "MOVE_RA_NEGATIVE"

#define MAX_COMM_RESULT_LENGTH   10
#define MAX_COMM_LENGTH          40

#define RES_OK        0
#define RES_NOK       1
#define RES_BADSTATE  2

#define COMID_UNUSED -1

class COMPort
{
private:
    int comHandle;
    int baudRate;

	
public:

	

    COMPort();
    int Initialize(const char *portId);
    int Close();
    int SendCommand(const char *command);
    int ReadResult(char* resultBuff_p, int* length);
};

#endif
