
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>   // added
#define  Max_Calib              1048575
#define  T0                     0.000000002
#define  MTC_Port               0000
#define  IP_Address             "00.123.4.56" //real IP address of the counter goes here
#define  T_DATAbits             0x3FFF00000000
#define  M_COUNTER1bits         0xFFFF000000000000
#define  M_COUNTER2bits         0X1FFFFFF



static void Measurement(ssize_t BytesSent,ssize_t ByteReceived, int SendingSocket, unsigned char CLK_S_Cal,  unsigned char CLK_EN_Cal, float Offset[], float Delay[], long int hexToBin, int ones);

static int Calibration(ssize_t BytesSent,ssize_t ByteReceived, int SendingSocket, uint32_t num, unsigned char CLK_S_Cal, unsigned char CLK_EN_Cal);

char binaryToHex(char buf);

long int MJDcalc();
long UTCcalc();
uint64_t Endian_Swap64(void *X);

int main()
{
    int    Stamps, Rate, Clock, Mode, ones=0, SendingSocket;
    float Buf;
    long int hex=0, hexToBin=0;
    float Offset[6], Delay[7];
    uint32_t num=0;
    //uint64_t num64=0;
    unsigned char CLK_EN_Cal=(unsigned char)0x00, CLK_S_Cal=(unsigned char)0x00;
    char buf[256];
    FILE  *Config;
    /////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////CONNECT TO MTC 108/////////////////////////////////////
    ssize_t  BytesSent=0, ByteReceived=0;
	struct sockaddr_in server;
	//Create socket
	SendingSocket = socket(AF_INET , SOCK_STREAM , 0);
	if (SendingSocket == -1)
	{
		printf("Could not create socket");
	}

    server.sin_addr.s_addr = inet_addr(IP_Address);
	server.sin_family = AF_INET;
	server.sin_port = htons(MTC_Port);

	//Connect to remote server
	if (connect(SendingSocket , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	puts("Connected");

   ///////////////////////GET VALUES FROM CONFIG. FILE////////////////////////
   ///////////////////////////////////////////////////////////////////////////
    Config = fopen("Name of config file.txt", "r");

     if (Config == NULL){
        printf("Error Reading File\n");
        exit (0);
    }


    }

    fclose(Config);
    //////////////////////////////////////////CALIBRATION FUNC.//////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Calibration(BytesSent, ByteReceived, SendingSocket, num, CLK_S_Cal, CLK_EN_Cal);
    ///////////////////////////////////////// MEASUREMENT FUNC.//////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Measurement(BytesSent, ByteReceived, SendingSocket, CLK_S_Cal, CLK_EN_Cal, Offset, Delay, hexToBin, ones);
    ////////////////////////////////finish sending info before this code/////////////////////////////////////////////////
     if(close(SendingSocket) != 0)
          printf("Cannot close socket connection\n");
     else
          printf("Disconnecting from MTC 108 Counter...\n");

return 0;
}

 static void Measurement(ssize_t BytesSent,ssize_t ByteReceived, int SendingSocket, unsigned char CLK_S_Cal,  unsigned char CLK_EN_Cal, float Offset[], float Delay[], long int hexToBin, int ones)
{
    int mnum=0;
    uint64_t mnum64=0;
    unsigned char MTC_Meas[4] = { (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00};
    char MJD[100];

    unsigned char cmd_set_s_meas[5] =       { (unsigned char)0x02, (unsigned char)0xFF, (unsigned char)CLK_S_Cal, (unsigned char)0x00, (unsigned char)0x00 }; // CH0 and CH1 in measurement mode, external clock
    unsigned char cmd_meas[5]=              { (unsigned char)0x01, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 }; // start measurement - Continuous mode
    unsigned char cmd_rd_mem_data[5]=       { (unsigned char)0xF7, (unsigned char)0x01, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 };// reading of measurement data from memory; number of measurements to read 1
    unsigned char cmd_rd_mem_count[5] =     { (unsigned char)0xF8, (unsigned char)0x01, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 }; // read the mem_count
    unsigned char cmd_meas_en[5]=           { (unsigned char)0x03, (unsigned char)hexToBin, (unsigned char)CLK_EN_Cal, (unsigned char)0x00, (unsigned char)0x00 };
    unsigned char cmd_meas_ctrl[5]=         { (unsigned char)0x04, (unsigned char)0xFF, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x80 };

    uint64_t  T_DATA, M_COUNTER_S, M_COUNTER1, M_COUNTER2;
    int r=1, p=0, a=0, test=0, con_clear=0, hr, min, sec, i=0, iter;
    float n9=16383;

    double T_Stamp, T_DATA_S, CZ, MJDecimal, UTC_h, UTC_m, UTC_s;
    double *TI_Stamp_START;    /// TI tasks declarations (config. file) -- IMPORTANT for TI calculation

    TI_Stamp_START=(double*)malloc((ones-1)*sizeof(double));
    if ( TI_Stamp_START == NULL) {
	printf("Memory allocation error.\n");
	exit (0);
    }


    FILE** f = malloc(sizeof(FILE*) * (8));
    if ( f == NULL) {
	printf("Memory allocation error.\n");
	exit (0);
    }

    int *TI_START_NO;    //


     TI_START_NO=(int*)malloc((ones-1)*sizeof(int));
    if ( TI_START_NO == NULL) {
	printf("Memory allocation error.\n");
	exit (0);
    }

    while(i<ones-1)
    {

     TI_START_NO[i]=0;

     i++;
    }

    int *TI_STOP_NO;  //
    TI_STOP_NO=(int*)malloc((ones-1)*sizeof(int));
        if ( TI_STOP_NO == NULL) {
	printf("Memory allocation error.\n");
	exit (0);
    }
        a=0;
     for (i=1, iter=1; i<256; i<<=1, iter++)
            {
             if (hexToBin & i)
             {
                 if(iter>1)
                 {
                    TI_STOP_NO[a]=iter-1;
                    a++;
                 }
             }
            }
    i=0;
    while(i<ones-1)
    {
     i++;
    }

    bool *TI_Stamp_START_present;
    TI_Stamp_START_present=(bool*)malloc((ones-1)*sizeof(bool));
    if ( TI_Stamp_START_present == NULL) {
	printf("Memory allocation error.\n");
	exit (0);
    }
    i=0;
    while(i<ones-1)
    {
     TI_Stamp_START_present[i]=0;
     i++;
    }
    i=0;
    while(i<ones-1)
    {
     i++;
    }

    int T_NO=0;

    time_t rawtime;
    struct tm *UTC;

    unsigned char cmd_set_dac_ch01[5] = { (unsigned char)0x05, (unsigned char)0xFC, (unsigned char)0x08, (unsigned char)0xFC, (unsigned char)0x08 }; // set trigger thresholds to + 0.5V for ch 1 and 2
    unsigned char cmd_set_dac_ch23[5] = { (unsigned char)0x05, (unsigned char)0xFC, (unsigned char)0x08, (unsigned char)0xFC, (unsigned char)0x18 }; // set trigger thresholds to + 0.5V for ch 3 and 4
    unsigned char cmd_set_dac_ch45[5] = { (unsigned char)0x05, (unsigned char)0xFC, (unsigned char)0x08, (unsigned char)0xFC, (unsigned char)0x28 }; // set trigger thresholds at + 0.5V for ch 5 and 6
    unsigned char cmd_set_dac_ch67[5] = { (unsigned char)0x05, (unsigned char)0xFC, (unsigned char)0x08, (unsigned char)0xFC, (unsigned char)0x38 }; // set trigger thresholds at + 0.5V for ch 7 and 8

    BytesSent = send(SendingSocket, cmd_set_dac_ch01, sizeof(cmd_set_dac_ch01), 0);
    (void)usleep(500);
    BytesSent = send(SendingSocket, cmd_set_dac_ch23, sizeof(cmd_set_dac_ch23), 0);
    (void)usleep(500);
    BytesSent = send(SendingSocket, cmd_set_dac_ch45, sizeof(cmd_set_dac_ch45), 0);
    (void)usleep(500);
    BytesSent = send(SendingSocket, cmd_set_dac_ch67, sizeof(cmd_set_dac_ch67), 0);
    (void)usleep(500);

    BytesSent = send(SendingSocket, cmd_set_s_meas, sizeof(cmd_set_s_meas), 0);
    BytesSent = send(SendingSocket, cmd_meas_ctrl, sizeof(cmd_meas_ctrl), 0);
    BytesSent = send(SendingSocket, cmd_meas_en, sizeof(cmd_meas_en), 0);
    BytesSent = send(SendingSocket, cmd_meas, sizeof(cmd_meas), 0);
    (void)usleep(1000);
    i=0;

          for(;;)//infinite loop for the continuous mode, you stop it by closing the program
            {
                BytesSent = send(SendingSocket, cmd_rd_mem_count, sizeof(cmd_rd_mem_count), 0);
                (void)usleep(50);
                ByteReceived = recv(SendingSocket, MTC_Meas, 4, 0);
                (void)usleep(50);

                if(ByteReceived<0)
                {
                 printf("Error with counter communication: %s\n", strerror( errno ));
                }
                memmove(&mnum, MTC_Meas, 4);
               if(mnum>0){
                         ///////////////////////////////Reading measurements per channel////////////////////////////////////////////////////////////////////
                      for(test=0;test<mnum;test++)
                        {
                            /// Get system time
                            /// Read all collected stamps stored in the memory
                            BytesSent = send(SendingSocket, cmd_rd_mem_data, sizeof(cmd_rd_mem_data), 0);
                            ByteReceived = recv(SendingSocket, MTC_Meas, 8, 0);
                            memmove(&mnum64, MTC_Meas, 8);
                            // Calculate Time stamp
                            T_NO = (int)((mnum64 & 0x000000000E000000) >> 25);
                            T_DATA = (uint64_t)((mnum64 & T_DATAbits) >> 32);
                            M_COUNTER2 = ((mnum64 & M_COUNTER2bits)<<16);
                            M_COUNTER1 = ((mnum64 & M_COUNTER1bits)>>48);
                            M_COUNTER_S = (M_COUNTER1|M_COUNTER2);
                            T_DATA_S = (double)(T_DATA / n9);
                            T_Stamp = (double)(2*(M_COUNTER_S + T_DATA_S));
                            /// Measurement tasks  = to config. file
                            for(i=0;i<=mnum-1;i++)   /// Calculate the time interval for each task
                            {
                               if (T_NO==TI_STOP_NO[i])   //stop
                               {
                                if (TI_STOP_NO[i]!=0)// this is to avoid calculating Ch1->CH1 (we don't need that)
                                {
                                  if (TI_Stamp_START_present[i])
                                  {
                                   ///Here goes some math formula to calculate timestamps
                                    snprintf(MJD, sizeof(MJD),"MTC108_CH%d_%ld.dat", TI_STOP_NO[i]+1, MJDcalc());
                                       f[i] = fopen(MJD, "a+"); ///Open Time interval file and save data
                                        if (f[i] == NULL)
                                            {
                                                printf("Error opening the file: %s\n", strerror( errno ));
                                            }
                                    printf ("\nMJD:%.6f, CH%d: %-.3f, Timestamp: %02d/%02d/%02d %02d:%02d:%02d", (double)(MJDcalc() + MJDecimal), TI_STOP_NO[i]+1, CZ, UTC->tm_mday, UTC->tm_mon + 1, UTC->tm_year + 1900, hr, min, sec);
                                    fprintf (f[i],"%.6f   %-.3f  %02d %02d %02d\n", (double)(MJDcalc() + MJDecimal), CZ, hr, min, sec);
                                   (void)fflush(f[i]);
                                    (void)fclose(f[i]);
                                    con_clear++;
                                    if(con_clear>100)
                                    {
                                    (void)system("clear");
                                    con_clear=0;
                                    }
                            r++;
                                  }
                               }
                                }
                                if (T_NO==TI_START_NO[i])   //start
                                {
                                   TI_Stamp_START[i] = T_Stamp;
                                   TI_Stamp_START_present[i] = true;
                                }
                            }
                        }
                        printf("\n");
                 }
    }
}

 static int Calibration(ssize_t BytesSent,ssize_t ByteReceived, int SendingSocket, uint32_t num, unsigned char CLK_S_Cal,  unsigned char CLK_EN_Cal)
{
     unsigned char MTC_Cal[4] =              { (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00};
     unsigned char cmd_set_s_calib[5] =      { (unsigned char)0x02, (unsigned char)0x00, (unsigned char)CLK_S_Cal, (unsigned char)0x00, (unsigned char)0x00 }; // calibration channels adjustment, external clock
     unsigned char cmd_res[5]=               { (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 }; //reset
     unsigned char cmd_cal_count[5] =        { (unsigned char)0xF0, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 }; //meter of calibration measurements
     unsigned char cmd_cal_ctrl[5]=          { (unsigned char)0x04, (unsigned char)0xFF, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x80 };// positive polarity CH0 and CH1, offset compensation disabled///////////////independent mode or source???
     unsigned char cmd_cal_en[5]=            { (unsigned char)0x03, (unsigned char)0xFF, (unsigned char)CLK_EN_Cal, (unsigned char)0x00, (unsigned char)0x00 };//CH0 and Ch1 Active, external clock active, input trig off
     unsigned char cmd_cal_ready[5] =        { (unsigned char)0xF6, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00 }; //meter of calibration measurements

                    BytesSent = send(SendingSocket, cmd_set_s_calib, sizeof(cmd_set_s_calib), 0);
                    (void)usleep(50);
                    BytesSent = send(SendingSocket, cmd_cal_en, sizeof(cmd_cal_en), 0);
                    (void)usleep(10);
                    BytesSent = send(SendingSocket, cmd_cal_ctrl, sizeof(cmd_cal_ctrl), 0);
                    (void)usleep(10);

                /////////////////////////////////////////////////////////////////
                    BytesSent = send(SendingSocket, cmd_res, sizeof(cmd_res), 0);
                    (void)usleep(10);
                    printf("CALIBRATING...");
                    MTC_Cal[2]=(unsigned char)0x00;

                    while (MTC_Cal[2]!=(unsigned char)0xFF)
                        {
                            BytesSent = send(SendingSocket, cmd_cal_count, sizeof(cmd_cal_count), 0);
                            ByteReceived = recv(SendingSocket, MTC_Cal, 4, 0);
                           // memcpy(&num, MTC_Cal, 4);
                            memmove(&num, MTC_Cal, 4);
                            printf("\rCALIBRATION PROGRESS: %d -> 1048575   ",(int)num);
                            (void)usleep(50);
                            // wait on flags
                            BytesSent = send(SendingSocket, cmd_cal_ready, sizeof(cmd_cal_ready), 0);
                            ByteReceived = recv(SendingSocket, MTC_Cal, 4, 0);
                        }
                    printf("\nCALIBRATION COMPLETE. \n");
                    return(0);
}

long int MJDcalc()
{
    int  M, Day, M1;
    double B, C, D, MJ, Y, A;
    long    MJDFMDATE;
    time_t  t = time(NULL);
    struct  tm tm = *gmtime(&t);

    Y = (double)tm.tm_year + 1900;
    M = tm.tm_mon + 1;
    Day = tm.tm_mday;

        if(M<3)
            {
                Y = Y-1;
                M1 = M+12;
            }
        else
            {
                M1 = M;
            }
//////////////////////////////////////////////////////Calculate MJD/////////////////////////////////////////////////////////////////
    A =  trunc(Y/100);
    B = 2 - A + trunc(A/4);
    C = trunc(365.25*Y) - 694025;
    D = trunc(30.600*(M1+1));
    MJ = B + C + D + Day +15019;
    MJDFMDATE = (long int)trunc(MJ);
    return  MJDFMDATE;
}
