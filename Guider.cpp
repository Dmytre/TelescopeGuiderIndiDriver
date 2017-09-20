#include "Guider.h"
#include "Logging.h"
#include <memory>
#include "indicom.h"
#include "unistd.h"
#include "COMInteract.h"

std::auto_ptr<GuidingDriver> guidingDevice(0);


/**************************************************************************************
** Initilize SimpleDevice object
***************************************************************************************/
void ISInit()
{
	
 static int isInit=0;
 
 if (isInit)
 {
	return;
 }
 if (guidingDevice.get() == 0)
 {
     isInit = 1;
     guidingDevice.reset(new GuidingDriver());
     IDLog("ISInit() - initialized\n");
 }
}


/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties (const char *dev)
{
	IDLog("ISGetProperties() called\n");
	ISInit();
	guidingDevice->ISGetProperties(dev);
	
}


/**************************************************************************************

** We init our properties here. The only thing we want to init are the Debug controls

***************************************************************************************/
bool GuidingDriver::initProperties()
{
	IDLog("GuidingDriver::initProperties() - called\n");
	/* Init Default Device Properties */
	INDI::DefaultDevice::initProperties();
	initGuiderProperties(getDeviceName(), MOTION_TAB);

	
	IUFillNumber(&GuideRateN[0], "GUIDE_RATE_WE", "W/E Rate", "%g", 0, 1, 0.1, 0.3);
	IUFillNumber(&GuideRateN[1], "GUIDE_RATE_NS", "N/S Rate", "%g", 0, 1, 0.1, 0.3);
	IUFillNumberVector(&GuideRateNP, GuideRateN, 2, getDeviceName(), "GUIDE_RATE", "Guiding Rate", MOTION_TAB, IP_RW, 0, IPS_IDLE);
	registerProperty(&GuideRateNP, INDI_NUMBER);
	
	
	setDriverInterface(getDriverInterface() | GUIDER_INTERFACE);
	
	return true;
}


void GuidingDriver::ISGetProperties(const char *dev)
{
	IDLog("GuidingDriver::ISGetProperties() called\n");
	INDI::DefaultDevice::ISGetProperties(dev);
	
	if (isConnected())
	{
		IDLog("GuidingDriver::ISGetProperties() add guiding properties since connected\n");
		defineNumber(&GuideNSNP);
		defineNumber(&GuideWENP);
		defineNumber(&GuideRateNP);
	}
}

bool GuidingDriver::updateProperties(void)
{
	IDLog("GuidingDriver::updateProperties() called\n");
	INDI::DefaultDevice::updateProperties();
	
	if (isConnected())
	{
		IDLog("GuidingDriver::updateProperties() add guiding properties since connected\n");
		defineNumber(&GuideNSNP);
		defineNumber(&GuideWENP);
	}
}


/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
	IDLog("ISNewSwitch(%s, %d) - called\n", name, n);
	
	
	ISInit();
	guidingDevice->ISNewSwitch(dev, name, states, names, n);
}


/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
	IDLog("ISNewText() - called\n");
	ISInit();
	guidingDevice->ISNewText(dev, name, texts, names, n);
}


/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
	IDLog("ISNewNumber(): name=%s\n", name);
	
	ISInit();
	
	guidingDevice->ISNewNumber(dev, name, values, names, n);
}


/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
	IDLog("ISNewBLOB() - blob from client: %s\n", name);
    ISInit();
    guidingDevice->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}
/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice (XMLEle *root)
{
  INDI_UNUSED(root);
}

GuidingDriver::GuidingDriver()
{
	IDLog("GuidingDriver::GuidingDriver() - CONSTRUCTOR called\n");
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool GuidingDriver::Connect()
{
    IDLog("GuidingDriver::Connect() - CONNECT function called\n");

    //ToDO: Did not work: Call it to Reset CONNECT/DISCONNECT switches
    IDLog("GuidingDriver::Connect() - Resetting DefDevice properties\n");
    INDI::DefaultDevice::initProperties();
    /* Set device CONNECTION property */
    //ToDo: setConnected(true);
    
    /*Here we must init and connect to COM port */
    char portId[] = "/dev/ttyUSB0";
    IDLog("Connect to device: PORT=%s...",portId);
    if(RES_OK != DevicePort.Initialize(portId))
    {
        /* Failed to connect to device */
        IDLog("FAILED!!!\n");
	return false;
    }
    else
    {
	IDLog("SUCCESS\n");
	/*Give a timeout before using the port */
	sleep(2);
	
	IDLog("Checking device status ..");
        if(RES_OK != DevicePort.SendCommand(COMM_GET_DEV_STATUS))
	{
	    IDLog("FAILED to send command %s\n", COMM_GET_DEV_STATUS);
	    DevicePort.Close();
	    return false;
	}
	

	
	char commandResult[20];
	int bytesRead = 0;
	
	/* Check device status */
        if(RES_OK != DevicePort.ReadResult(commandResult, &bytesRead))
	{
	    IDLog("FAILED to get command result\n");
	    DevicePort.Close();
	    return false;
	}
	else
	{
	    commandResult[bytesRead] = '\0';
	    IDLog("READ SUCCESS; res=%s, bytesRead=%d\n", commandResult, bytesRead);
	}
	
        /* Send GUIDE command to make device track with celestial speed */
        if(RES_OK != DevicePort.SendCommand(COMM_GUIDE))
	{
	    IDLog("FAILED to send command %s\n", COMM_GUIDE);
	    DevicePort.Close();
	    return false;
	}
	else
	{
	    IDLog("Start tracking ...");
	    if(RES_OK != DevicePort.SendCommand(COMM_GET_DEV_STATUS))
	    {
	        IDLog("FAILED to send command %s\n", COMM_GET_DEV_STATUS);
	        DevicePort.Close();
	        return false;
	    }
	    
	    sleep(2);
	    
	    if(RES_OK != DevicePort.ReadResult(commandResult, &bytesRead))
	    {
	        IDLog("FAILED; Read error\n");
	        DevicePort.Close();
	        return false;
	    }
	    else
	    {
	        commandResult[bytesRead] = '\0';
	        IDLog("READ SUCCESS; res=%s, bytesRead=%d\n", commandResult, bytesRead);
	    
	        if (0 != strncmp("RCSTATE:1#\r", commandResult, bytesRead))
	        {
	            /* Device is in the wrong state */
		    IDLog("DEVICE TRACKING RESULT FAILED; Wrong state(%s)\n", commandResult);
	          /* DevicePort.Close();
	            return false;*/
	        }
	        else
	        {
	            IDLog("SUCCESS\n");
	        }
	    }
	    
	}
    }
    
    IDMessage(getDeviceName(), "GuidingDriver connected successfully!");

    //ToDo: This function is callsed inside DefaultDevice::ISNewSwitch(CONNECTION).
    setConnected(true);
    return true;
}



/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool GuidingDriver::Disconnect()
{
    IDLog("GuidingDriver::Disconnect() - DISCONNECT function called\n");
    
    /* Send STOP command to make device hold */
    IDLog("Sending STOP-command ...\n");
    if(RES_OK != DevicePort.SendCommand(COMM_STOP))
    {
        IDLog("FAILED to send command %s\n", COMM_STOP);
    }
    
    //ToDo: Maybe the problem is because we need a delay here
    sleep(2);
    
    DevicePort.Close();
    
    IDMessage(getDeviceName(), "GuidingDriver disconnected successfully!");
    
    //ToDo: This function is callsed inside DefaultDevice::ISNewSwitch(CONNECTION).
    //setConnected(false);
    
    return true;
}


/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char * GuidingDriver::getDefaultName()
{
    return "Guiding Device (by Dmytre)";
}


/** Default Device calls:

setConnected(true)  + Connect    when CONNECTION.CONNECT=ISS_ON 
setConnected(false) + Disconnect when CONNECTION.DISCONNECT=ISS_ON */
bool GuidingDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{

//ToDo: Debug the connection switch
    ISwitchVectorProperty *svp = guidingDevice->getSwitch(name);
    if((NULL != svp) && (0 == strcmp(name,"CONNECTION")))
    {
        for(int i=0;i<n; i++)
        {
	    if(0==strcmp(names[i],"CONNECT"))
	    {
		if(states[i] == ISS_ON)
		{
		    IDLog("  CONNECT = ISS_ON\n");
		}else
		{
		    IDLog("  CONNECT = ISS_OFF\n");
		}
	    }

	    if(0==strcmp(names[i],"DISCONNECT"))
	    {
		if(states[i] == ISS_ON)
		{
		    IDLog("  DISCONNECT = ISS_ON\n");
		}else
		{
		    IDLog("  DISCONNECT = ISS_OFF\n");
		}
	    }
        }
    }
    
    DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Client sent new number to process
***************************************************************************************/
bool GuidingDriver::ISNewNumber (
			const char *dev,
			const char *name,
			double values[],
			char *names[],
			int n)
{
	IDLog("GuidingDriver::ISNewNumber() - name=%s", name);
	
	if (!strcmp(name, GuideNSNP.name) || !strcmp(name, GuideWENP.name))
	{
		processGuiderProperties(name, values, names, n);
		return true;
	}
	
	return INDI::DefaultDevice::ISNewNumber(dev,name,values,names,n);
}



/* GUIDING FUNCTIONS */
IPState GuidingDriver::GuideNorth (float ms)
{
    IDLog("GuidingDriver::GuideNorth(%f)(%d)\n", ms, int(ms));
    if(int(ms) > MAX_PULSE_VALUE)
    {
    	IDLog("ERROR, too big pulse value\n");
    	return IPS_ALERT;
    }
    
    char combinedComm[40];
    sprintf(combinedComm, "%s:%d#\0", COMM_GUIDE_NORTH, int(ms));
     
    IDLog("Command to send %s with length= %d\n", combinedComm, strlen(combinedComm));
	
    /* Send DEC_GUIDE_POSITIVE:int(ms)# command to device */
    if(RES_OK != DevicePort.SendCommand(combinedComm))
    {
        IDLog("FAILED to send guide pulse %s\n", combinedComm);
    }
	
    //TBD: Need to return IPS_BUSY?
    return IPS_OK;
}

IPState GuidingDriver::GuideSouth (float ms)
{
    IDLog("GuidingDriver::GuideSouth(%f)(%d)\n", ms, int(ms));
    if(int(ms) > MAX_PULSE_VALUE)
    {
    	IDLog("ERROR, too big pulse value\n");
    	return IPS_ALERT;
    }
    
    char combinedComm[40];
    sprintf(combinedComm, "%s:%d#\0", COMM_GUIDE_SOUTH, int(ms));
     
    IDLog("Command to send %s with length= %d\n", combinedComm, strlen(combinedComm));
	
    /* Send DEC_GUIDE_NEGATIVE:int(ms)# command to device */
    if(RES_OK != DevicePort.SendCommand(combinedComm))
    {
        IDLog("FAILED to send guide pulse %s\n", combinedComm);
    }
	
    //TBD: Need to return IPS_BUSY?
    return IPS_OK;
}

IPState GuidingDriver::GuideWest (float ms)
{
    IDLog("GuidingDriver::GuideWest(%f)(%d)\n", ms, int(ms));
    
    /* Check that pulse value is in range */
    if(int(ms) > MAX_PULSE_VALUE)
    {
    	IDLog("ERROR, too big pulse value\n");
    	return IPS_ALERT;
    }
    
    /* Build guiding command with ms parameter */
    char combinedComm[40];
    sprintf(combinedComm, "%s:%d#\0", COMM_GUIDE_WEST, int(ms));
     
    IDLog("Command to send %s with length= %d\n", combinedComm, strlen(combinedComm));
	
    /* Send RA_GUIDE_POSITIVE:int(ms)# command to device */
    if(RES_OK != DevicePort.SendCommand(combinedComm))
    {
        IDLog("FAILED to send guide pulse %s\n", combinedComm);
        return IPS_ALERT;
    }
	
    //TBD: Need to return IPS_BUSY?
    return IPS_OK;
}

IPState GuidingDriver::GuideEast (float ms)
{
    IDLog("GuidingDriver::GuideEast(%f)(%d)\n", ms, int(ms));
    
    /* Check that pulse value is in range */
    if(int(ms) > MAX_PULSE_VALUE)
    {
    	IDLog("ERROR, too big pulse value\n");
    	return IPS_ALERT;
    }
    
    /* Build guiding command with ms parameter */
    char combinedComm[40];
    sprintf(combinedComm, "%s:%d#\0", COMM_GUIDE_EAST, int(ms));
     
    IDLog("Command to send %s with length= %d\n", combinedComm, strlen(combinedComm));
	
    /* Send RA_GUIDE_NEGATIVE:int(ms)# command to device */
    if(RES_OK != DevicePort.SendCommand(combinedComm))
    {
        IDLog("FAILED to send guide pulse %s\n", combinedComm);
        return IPS_ALERT;
    }
	
    //TBD: Need to return IPS_BUSY?
    return IPS_OK;
}

void GuidingDriver::GuideComplete (INDI_EQ_AXIS axis)
{
	IDLog("GuidingDriver::GuideComplete() - GUIDE COMPLETE callback\n");
    INDI::GuiderInterface::GuideComplete(axis);
	return;
}
