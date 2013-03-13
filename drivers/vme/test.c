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
	//mvme_set_am(myvme, MVME_AM_A32);

	WORD buff16;
	//buff16 = v792n_Read16(myvme,V792_BASE,0x1000);
	CAENVME_ReadCycle(BHandle,V792_BASE+0x1000, &buff16, cvA32_U_DATA,2);
	printf("%X\n",buff16);

	// Close
	CAENVME_End(BHandle);
	return 0;
}

