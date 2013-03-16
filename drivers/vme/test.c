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
void showbits(uint32_t data);

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
	/*v792n_Setup(myvme,V792_BASE,2); //enable empty events, over range & under thr.*/
	//v792n_OnlineSet(myvme, V792_BASE);
	v792n_Status(myvme,V792_BASE);
	sleep(5);
	/*CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvManualSW);*/
	v1718_PulserConfSet(myvme,v1718_pulserA,1000,200,1);
	CVTimeUnits unit;
	CVIOSources io,reset;
	unsigned char per,wid,no;
	WORD buff16;
	CAENVME_GetPulserConf(BHandle,v1718_pulserA,&per,&wid,&unit,&no,&io,&reset);
	printf("pulser A: %d %d %d %d %d %d \n",per,wid,unit,no,io,reset);
	v1718_PulserStart(myvme,v1718_pulserA);
	/*CAENVME_ReadRegister(BHandle,cvPulserA0,&buff16);*/
	/*showbits((uint32_t)buff16);*/
	//usleep(10000000);
	/*v1718_PulserStop(myvme,v1718_pulserA);*/
	v792n_Status(myvme,V792_BASE);
	//buff16 = v792n_Read16(myvme,V792_BASE,0x1000);

	// Close
	CAENVME_End(BHandle);
	return 0;
}
void showbits(uint32_t data)
{

    int ii;
    for (ii=31;ii>=28;ii--)
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=27;ii>=24;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=23;ii>=20;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=19;ii>=16;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=15;ii>=12;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=11;ii>=8;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=7;ii>=4;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');

    for (ii=3;ii>=0;ii--) 
        (data&(1<<ii))?putchar('1'):putchar('0');
    putchar(' ');
}

