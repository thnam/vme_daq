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
#include "v792.h"
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

	/*v792_SoftReset(myvme,V792_BASE);*/
	v792_Setup(myvme,V792_BASE,2); //enable empty events, over range & under thr.
	int lam = v792_DataReady(myvme,V792_BASE);
	printf("lam: %d\n",lam);
	/*v792_EvtCntReset(myvme,V792_BASE);*/
	/*v792_DataClear(myvme,V792_BASE);*/

	//v792_Status(myvme,V792_BASE);
	WORD pat = v792_ControlRegister1Read(myvme,V792_BASE);
	showbits((uint32_t)pat); 
	printf("\n");
	v792_BusErrorEnable(myvme,V792_BASE);
	pat = v792_ControlRegister1Read(myvme,V792_BASE);
	showbits((uint32_t)pat);
	printf("\n");
	CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvManualSW);
	v1718_PulserConfSet(myvme,v1718_pulserA,1000,200,1);
	v1718_PulserStart(myvme,v1718_pulserA);
	v1718_PulserStop(myvme,v1718_pulserA);

	lam = v792_DataReady(myvme,V792_BASE);
	printf("lam: %d\n",lam);

	DWORD buff[100];
	int i;
	int nword_read;

	//DWORD nevt;
	//v792_EvtCntRead(myvme,V792_BASE,&nevt);
	//printf("nevt: %d\n",nevt);
	do
		usleep(1);
	while (!(v792_DataReady(myvme,V792_BASE)));
	v792_EventReadBLT(myvme,V792_BASE,buff,&nword_read);

	printf("nword_read %d\n",nword_read/4);

	//int i;
	for (i = 0; i < nword_read/4; i++) 
	{
		v792_printEntry((v792_Data*)&buff[i]);
		//showbits(buff[i]);
		//printf("\n");
	}

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

