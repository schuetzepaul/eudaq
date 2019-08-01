#include "QDCControl.hh"

QDCControl::QDCControl()
    : i(0)
{
    // assign stuff
}

QDCControl::~QDCControl()
{

}



//Private Functions

void QDCControl::write_reg(uint16_t reg_addr, uint16_t data)
{
    CVErrorCodes ret;
    ret = CAENVME_WriteCycle(handle, BaseAddress + reg_addr, &data, cvA32_U_DATA, cvD16);
    if(ret != cvSuccess) {
        sprintf(ErrorString, "Cannot write at address %08X\n", (uint32_t)(BaseAddress + reg_addr));
        VMEerror = 1;
    }
    if (ENABLE_LOG)
        fprintf(logfile, " Writing register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
}

uint16_t QDCControl::read_reg(uint16_t reg_addr)
{
    uint16_t data=0;
    CVErrorCodes ret;
    ret = CAENVME_WriteCycle(handle, BaseAddress + reg_addr, &data, cvA32_U_DATA, cvD16);
    if(ret != cvSuccess) {
        sprintf(ErrorString, "Cannot write at address %08X\n", (uint32_t)(BaseAddress + reg_addr));
        VMEerror = 1;
    }
    if (ENABLE_LOG)
        fprintf(logfile, " Writing register at address %08X; data=%04X; ret=%d\n", (uint32_t)(BaseAddress + reg_addr), data, (int)ret);
    return(data);
}

int QDCControl::SaveHistograms(uint32_t histo[32][4096], int numch)
{
    int i, j;
    for(j=0; j<numch; j++) {
        FILE *fout;
        char fname[100];

        char path[255];
        sprintf(fname, "%s\\Histo_%d.txt",path,  j);
        fout = fopen(fname, "w");
        for(i=0; i<4096; i++)
            fprintf(fout, "%d\n", (int)histo[j][i]);
        fclose(fout);
    }
    return 0;
}

void QDCControl::findModelVersion(uint16_t model, uint16_t vers, char *modelVersion, int *ch)
{
    switch (model) {
    case 792:
        switch (vers) {
        case 0x11:
            strcpy(modelVersion, "AA");
            *ch = 32;
            return;
        case 0x13:
            strcpy(modelVersion, "AC");
            *ch = 32;
            return;
        case 0xE1:
            strcpy(modelVersion, "NA");
            *ch = 16;
            return;
        case 0xE3:
            strcpy(modelVersion, "NC");
            *ch = 16;
            return;
        default:
            strcpy(modelVersion, "-");
            *ch = 32;
            return;
        }
        break;
    case 965:
        switch (vers) {
        case 0x1E:
            strcpy(modelVersion, "A");
            *ch = 16;
            return;
        case 0xE3:
        case 0xE1:
            strcpy(modelVersion, " ");
            *ch = 32;
            return;
        default:
            strcpy(modelVersion, "-");
            *ch = 32;
            return;
        }
        break;
    case 775:
        switch (vers) {
        case 0x11:
            strcpy(modelVersion, "AA");
            *ch = 32;
            return;
        case 0x13:
            strcpy(modelVersion, "AC");
            *ch = 32;
            return;
        case 0xE1:
            strcpy(modelVersion, "NA");
            *ch = 16;
            return;
        case 0xE3:
            strcpy(modelVersion, "NC");
            *ch = 16;
            return;
        default:
            strcpy(modelVersion, "-");
            *ch = 32;
            return;
        }
        break;
    case 785:
        switch (vers) {
        case 0x11:
            strcpy(modelVersion, "AA");
            *ch = 32;
            return;
        case 0x12:
            strcpy(modelVersion, "Ab");
            *ch = 32;
            return;
        case 0x13:
            strcpy(modelVersion, "AC");
            *ch = 32;
            return;
        case 0x14:
            strcpy(modelVersion, "AD");
            *ch = 32;
            return;
        case 0x15:
            strcpy(modelVersion, "AE");
            *ch = 32;
            return;
        case 0x16:
            strcpy(modelVersion, "AF");
            *ch = 32;
            return;
        case 0x17:
            strcpy(modelVersion, "AG");
            *ch = 32;
            return;
        case 0x18:
            strcpy(modelVersion, "AH");
            *ch = 32;
            return;
        case 0x1B:
            strcpy(modelVersion, "AK");
            *ch = 32;
            return;
        case 0xE1:
            strcpy(modelVersion, "NA");
            *ch = 16;
            return;
        case 0xE2:
            strcpy(modelVersion, "NB");
            *ch = 16;
            return;
        case 0xE3:
            strcpy(modelVersion, "NC");
            *ch = 16;
            return;
        case 0xE4:
            strcpy(modelVersion, "ND");
            *ch = 16;
            return;
        default:
            strcpy(modelVersion, "-");
            *ch = 32;
            return;
        }
        break;
    case 862:
        switch (vers) {
        case 0x11:
            strcpy(modelVersion, "AA");
            *ch = 32;
            return;
        case 0x13:
            strcpy(modelVersion, "AC");
            *ch = 32;
            return;
        default:
            strcpy(modelVersion, "-");
            *ch = 32;
            return;
        }
    }
}

int QDCControl::ConfigureDiscr(uint16_t OutputWidth, uint16_t Threshold[], uint16_t EnableMask)
{
    int i;

    BaseAddress = DiscrBaseAddr;
    // Set channel mask
    write_reg(0x004A, EnableMask);

    // set output width (same for all channels)
    write_reg(0x0040, OutputWidth);
    write_reg(0x0042, OutputWidth);

    // set CFD threshold
    for(i=0; i<16; i++)
        write_reg(i*2, Threshold[i]);

    if (VMEerror) {
        printf("Error during CFD programming: ");
        printf(ErrorString);
        return -1;
    } else {
        printf("Discriminator programmed successfully\n");
        return 0;
    }
    BaseAddress = QTPBaseAddr;
}


//Public Functions

bool QDCControl::Connect()
{
    // open VME bridge (V1718 or V2718)
    if (CAENVME_Init(cvV1718, link, bdnum, &handle) != cvSuccess) {
        if (CAENVME_Init(cvV2718, link, bdnum, &handle) != cvSuccess) {
            printf("Can't open VME controller\n");
            return false;
        }
    }
    return true;}

bool QDCControl::SetupChannels(std::string info)
{
    std::cout<<std::endl<<"SetupChannels Info: "<<info<<std::endl;
    //initialize channels
    ch_plot_num = 0;
    for(int num=0; num < 32; num++){
        ch_plot[num] = 0;
    }
    //convert info string to int channel array
    for(int num=0; num < info.size(); num++){
        if(info[num] == ','){
            ch_plot_num = ch_plot_num + 1;
        }
        else{
            ch_plot[ch_plot_num] = ch_plot[ch_plot_num] * 10 + (int)info[num] - 48;
        }
    }
    ch_plot_num = ch_plot_num + 1;
    int sum=0;
    for(int num=0;num<ch_plot_num;num++){
        sum = sum + ch_plot[num];
    }
    std::cout<<std::endl<<std::endl<<"SetupChannels: "<<sum<<std::endl<<std::endl;
    return true;
}

void QDCControl::SelectFileToWriteTo(std::string filename)
{
    OutputFile = filename;
}

void QDCControl::StartDataTaking()
{
    pnt = 0;  // word pointer
    wcnt = 0; // num of lword read in the MBLT cycle
    buffer[0] = DATATYPE_FILLER;

    // clear Event Counter
    write_reg(0x1040, 0x0);
    // clear QTP
    write_reg(0x1032, 0x4);
    write_reg(0x1034, 0x4);
    // Open output files
    if ((of_list=fopen(OutputFile.c_str(), "w")) == NULL)
        printf("Can't open file for writing\n");
}

bool QDCControl::StopDataTaking()
{

}

bool QDCControl::ReadData()
{
    // if needed, read a new block of data from the board
    if ((pnt == wcnt) || ((buffer[pnt] & DATATYPE_MASK) == DATATYPE_FILLER)) {
//        std::cout<<std::endl<<std::endl<<"handle: "<<handle<<std::endl<<"BaseAddress: "<<BaseAddress
//                <<std::endl<<"buffer: "<<(char *)buffer<<std::endl<<std::endl;
        printf(" handle= %d \n BaseAddress= %d \n cvA32_U_MBLT= %d \n (char *)buffer= %c \n \n",handle,BaseAddress,cvA32_U_MBLT,(char *)buffer);
        CAENVME_FIFOMBLTReadCycle(handle, BaseAddress, (char *)buffer, MAX_BLT_SIZE, cvA32_U_MBLT, &bcnt);
//        std::cout<<std::endl<<std::endl<<"bcnt: "<<bcnt<<std::endl<<std::endl;
        if (ENABLE_LOG && (bcnt>0)) {
            int b;
            fprintf(logfile, "Read Data Block: size = %d bytes\n", bcnt);
            for(b=0; b<(bcnt/4); b++)
                fprintf(logfile, "%2d: %08X\n", b, buffer[b]);
        }
        wcnt = bcnt/4;
        totnb += bcnt;
        pnt = 0;
    }
//    if (wcnt == 0){  // no data available
//        return false;
//    }
    // save raw data (board memory dump)
    if (of_raw != NULL)
        fwrite(buffer, sizeof(char), bcnt, of_raw);

    /* header */
    switch (DataType) {
    case DATATYPE_HEADER :
        if((buffer[pnt] & DATATYPE_MASK) != DATATYPE_HEADER) {
            //printf("Header not found: %08X (pnt=%d)\n", buffer[pnt], pnt);
            DataError = 1;
        } else {
            nch = (buffer[pnt] >> 8) & 0x3F;
            chindex = 0;
            nev++;
            memset(ADCdata, 0xFFFF, 32*sizeof(uint16_t));
            if (nch>0)
                DataType = DATATYPE_CHDATA;
            else
                DataType = DATATYPE_EOB;
        }
        break;

        /* Channel data */
    case DATATYPE_CHDATA :
        if((buffer[pnt] & DATATYPE_MASK) != DATATYPE_CHDATA) {
            //printf("Wrong Channel Data: %08X (pnt=%d)\n", buffer[pnt], pnt);
            DataError = 1;
        } else {
            if (brd_nch == 32)
                j = (int)((buffer[pnt] >> 16) & 0x3F);  // for V792 (32 channels)
            else
                j = (int)((buffer[pnt] >> 17) & 0x3F);  // for V792N (16 channels)
            histo[j][buffer[pnt] & 0xFFF]++;
            ADCdata[j] = buffer[pnt] & 0xFFF;
            ns[j]++;
            if (chindex == (nch-1))
                DataType = DATATYPE_EOB;
            chindex++;
        }
        break;

        /* EOB */
    case DATATYPE_EOB :
        if((buffer[pnt] & DATATYPE_MASK) != DATATYPE_EOB) {
            //printf("EOB not found: %08X (pnt=%d)\n", buffer[pnt], pnt);
            DataError = 1;
        } else {
            DataType = DATATYPE_HEADER;
            if (of_list != NULL) {
                fprintf(of_list, "Event Num. %d\n", buffer[pnt] & 0xFFFFFF);
                for(int num = 0; num < ch_plot_num; num++){
                    if (ADCdata[ch_plot[num]] != 0xFFFF)
                        fprintf(of_list, "Ch %2d: %d\n", ch_plot[num], ADCdata[ch_plot[num]]);
                }
            }
        }
        break;
    }
    pnt++;

    if (DataError) {
        pnt = wcnt;
        write_reg(0x1032, 0x4);
        write_reg(0x1034, 0x4);
        DataType = DATATYPE_HEADER;
        DataError=0;
        return false;
    }
    else{
        return true;
    }
}

bool QDCControl::WriteData()
{
    return true;
}

void QDCControl::test()
{
    std::cout<<std::endl<<std::endl<<"Test Output: "<<std::endl<<std::endl;
    int sum = 0;
    for(int num = 0; num < ch_plot_num; num++){
        sum = sum + ch_plot[num];
    }
    std::cout<<"Sum: "<<sum;
    std::cout<<std::endl<<std::endl<<"OutputFile: "<<OutputFile<<std::endl<<std::endl;
}
