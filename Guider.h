/* Main driver */
#include <defaultdevice.h>
#include <indiguiderinterface.h>
#include "COMInteract.h"

class GuidingDriver : public INDI::DefaultDevice, public INDI::GuiderInterface
{
public:
	static const unsigned int MAX_PULSE_VALUE = 2000;
	GuidingDriver();
	
	IPState GuideNorth (float ms);
	IPState GuideSouth (float ms);
	IPState GuideWest  (float ms);
	IPState GuideEast  (float ms);
	void    GuideComplete (INDI_EQ_AXIS axis);
	
	bool initProperties();
	void ISGetProperties(const char *dev);
	bool updateProperties(void);
	
	bool ISNewSwitch (	const char *dev,
				const char *name,
				ISState *states,
				char *names[],
				int n);
				
	bool ISNewNumber (	const char *dev,
				const char *name,
				double values[],
				char *names[],
				int n);
	
protected:
	bool Connect();
	bool Disconnect();
	const char *getDefaultName();
	
private:
	INumberVectorProperty GuideRateNP;
	INumber GuideRateN[2];
	
	int COMPortHandle;
        COMPort DevicePort;

};
