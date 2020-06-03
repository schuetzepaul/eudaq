/*
    CAEN QDC V2718 control library
    G Morras - DESY summer student
    2019
    gonzalo.morras@desy.de
*/

#ifndef QDCControl_HH //header guard
#define QDCControl_HH

#include <string>
#include <iostream>
#include "CAENVMElib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <time.h>
#include <fstream>

#include <unistd.h>
#include <sys/time.h>
#define Sleep(x) usleep((x)*1000)

#include <CAENVMElib.h>
#include <CAENVMEtypes.h>

#include <cstdint>

/****************************************************/

#define MAX_BLT_SIZE		(256*1024)

#define DATATYPE_MASK		0x06000000
#define DATATYPE_HEADER		0x02000000
#define DATATYPE_CHDATA		0x00000000
#define DATATYPE_EOB		0x04000000
#define DATATYPE_FILLER		0x06000000

#define LSB2PHY				100   // LSB (= ADC count) to Physical Quantity (time in ps, charge in fC, amplitude in mV)

#define LOGMEAS_NPTS		1000

#define ENABLE_LOG			0

#define FILES_IN_LOCAL_FOLDER	0



class QDCControl
{
public:
    QDCControl();
    virtual ~QDCControl();
    bool Connect();
    bool SetupChannels(std::string Channels);
    void SelectFileToWriteTo(std::string filename);
    void SelectBinFileToWriteTo(std::string filename);
    bool StartDataTaking();
    bool StopDataTaking();
    int ReadData();
    bool disconnect();
    void resetVariables();
    //bool serializeData();
    std::vector<uint16_t> copyData() const {return data_vector;}

private:
    // Base Addresses
    uint32_t BaseAddress;
    uint32_t QTPBaseAddr = 0x00110000;
    uint32_t DiscrBaseAddr = 0x99990000;

    // handle for the V1718/V2718
    int32_t handle = -1;

    int VMEerror = 0;
    char ErrorString[100];
    FILE *logfile;

    //Variables from main
    int i, j, ch=0, chindex, wcnt, nch, pnt = 0, ns[32], bcnt, brd_nch = 32;
    int quit=0, totnb=0, nev=0, DataError=0, LogMeas=0, lognum=0;
    int link=0, bdnum=0;
    int DataType = DATATYPE_HEADER;
    int EnableHistoFiles = 0;		// Enable periodic saving of histograms (once every second)
    int EnableListFile = 0;			// Enable saving of list file (sequence of events)
    int EnableRawDataFile = 0;		// Enable saving of raw data (memory dump)
    int EnableSuppression = 1;		// Enable Zero and Overflow suppression if QTP boards
    uint16_t DiscrChMask = 0;		// Channel enable mask of the discriminator
    uint16_t DiscrOutputWidth = 10;	// Output wodth of the discriminator
    uint16_t DiscrThreshold[16] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};	// Thresholds of the discriminator
    uint16_t QTP_LLD[32] =	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char c;
    char tmpConfigFileName[100] = "config.txt";	// configuration file name
    char ConfigFileName[255] = "config.txt";	// configuration file name
    char histoFileName[255];
    char modelVersion[3];
    uint16_t fwrev, vers, sernum, model;
    uint16_t Iped = 100;			// pedestal of the QDC (or resolution of the TDC)
    uint32_t histo[32][4096];		// histograms (charge, peak or TAC)
    uint32_t buffer[MAX_BLT_SIZE/4];// readout buffer (raw data from the board)
    uint16_t ADCdata[32];			// ADC data (charge, peak or TAC)
    long CurrentTime, PrevPlotTime, PrevKbTime, ElapsedTime;	// time of the PC
    float rate = 0.0;				// trigger rate
    FILE *of_list=NULL;				// list data file
    FILE *of_raw=NULL;				// raw data file
    FILE *f_ini;					// config file
    FILE *gnuplot=NULL;				// gnuplot (will be opened in a pipe)
    FILE *fh;						// plotting data file

    //my variables
    int ch_plot_num, ch_plot[32], ch_rem, rem_num;
    std::vector<uint16_t> data_vector;
    std::string OutputFile, BinaryFile;
    std::ofstream OutputBinary;

    //Private functions
    void write_reg(uint16_t reg_addr, uint16_t data);
    uint16_t read_reg(uint16_t reg_addr);
    int SaveHistograms(uint32_t histo[32][4096], int numch);
    int ConfigureDiscr(uint16_t OutputWidth, uint16_t Threshold[16], uint16_t EnableMask);
    static void findModelVersion(uint16_t model, uint16_t vers, char *modelVersion, int *ch);
};
#endif
