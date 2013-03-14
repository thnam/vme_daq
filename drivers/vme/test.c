#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "CAENVMEtypes.h"
#include "CAENVMElib.h"
#include "mvmestd.h"
#include "v1718.h"
#include "v792n.h"
#define V792_BASE  0x54320000

int main(int argc, char** argv)
{
	CVBoardTypes  VMEBoard;
	short Link = 0;
	short Device = 0;
	VMEBoard = cvV1718;
	int BHandle;

	//Initialization of the V2718 module
	if( CAENVME_Init(VMEBoard, Link, Device, &BHandle) != cvSuccess ) 
	{
		printf("\n\n Error opening the device\n");
		exit(1);
	}

	
	MVME_INTERFACE *myvme;
	int status = mvme_open(&myvme, 0);
	mvme_sysreset(myvme);
	mvme_set_am(myvme, MVME_AM_A32_ND);
	usleep(100);

	v792n_SoftReset(myvme,V792_BASE);
	v792n_Setup(myvme,V792_BASE,2); //enable empty events, over range & under thr.
	v792n_OnlineSet(myvme, V792_BASE);
	v792n_Status(myvme,V792_BASE);
	/*v1718_PulserConfSet(myvme,v1718_pulserA,1000,200,0);*/
	//CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);
  //CAENVME_SetPulserConf(BHandle,
			//cvPulserA,3,3 ,cvUnit25ns,1,cvManualSW,cvInputSrc1);
	//CVTimeUnits unit;
	//CVIOSources io,reset;
	//unsigned char per,wid,no;
	//CAENVME_GetPulserConf(BHandle,v1718_pulserA,&per,&wid,&unit,&no,&io,&reset);
	//printf("pulser A: %d %d %d %d %d %d \n",per,wid,unit,no,io,reset);
	/*v1718_PulserStart(myvme,v1718_pulserA);*/
	//status = CAENVME_StartPulser(BHandle,cvPulserA);
	//CAENVME_DecodeError(status);
	//printf("%d\n",status);
	//usleep(100);
	/*v792n_Status(myvme,V792_BASE);*/
	//WORD buff16;
	//buff16 = v792n_Read16(myvme,V792_BASE,0x1000);
	//printf("%X\n",buff16);

	// Close
	CAENVME_End(BHandle);
	return 0;
}

