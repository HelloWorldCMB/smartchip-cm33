#include <stdio.h>
#include "Common_Include.h"
#include "CAN_type.h"
#include "CAN_test.h"
//#include "common.h"


void CAN_bit_timing_input(CAN_Param *CAN_DEV, BOOL FD)
{
    BOOL flag;
		char buf[32];
    if(FD)
    {
        fLib_printf("Non-FD mode(0) or FD mode(1):");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->FD_mode = atoi((char*)buf);

        fLib_printf("\n");
    }
    else
        fLib_printf("Only Classical FD (Non-FD) support in this test!");
    
    /* Constrains
        1.Non-FD: 
            Prop_seg = 1~8
            PS1_seg = 1~8
            PS2_seg = 1~8
            SJW = 1~4
          FD:
            NProp_seg = 1~64
            NPS1_seg = 1~32
            NPS2_seg = 1~32
            NSJW = 1~32
            DProp_seg = 0~8
            DPS1_seg = 1~8
            DPS2_seg = 1~8
            DSJW = 1~4
        2. The number of Tq must be 8 - 25.
        3. PS2 = MAX(PS1, IPT)
    */
    if(CAN_DEV->FD_mode == 0 || FD == DISABLE)
    {
        fLib_printf("Prescaler (2-256):");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPRE = atoi((char*)buf);

        fLib_printf("\r\nPropagation segment (1-8):");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPROP = atoi((char*)buf);

        fLib_printf("\r\nPhase 1 segment (1-8):");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPS1 = atoi((char*)buf);

        
        //CAN_DEV->NPS2 = CAN_MAX(CAN_DEV->NPS1, IPT_Tq);
        //fLib_printf("\r\nPhase 2 segment is %d", CAN_DEV->NPS2); 
        fLib_printf("\r\nPhase 2 segment (1-8):");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPS2 = atoi((char*)buf);
			
        
        fLib_printf("\r\nSynchronization Jump Width (1-%d):", min_t(CAN_DEV->NPS2,4));
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NSJW = atoi((char*)buf);

    }
    else
    {
        fLib_printf("\r\nNominal Prescaler (2-256): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPRE = atoi((char*)buf);

        fLib_printf("\r\nNominal Propagation segment (1-64): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPROP = atoi((char*)buf);
			
        fLib_printf("\r\nNominal Phase 1 segment (1-32): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPS1 = atoi((char*)buf);
			
        
        //CAN_DEV->NPS2 = CAN_MAX(CAN_DEV->NPS1, IPT_Tq);
        //fLib_printf("\r\nNominal Phase 2 segment is %d", CAN_DEV->NPS2);
        fLib_printf("\r\nNominal Phase 2 segment (1-32): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NPS2 = atoi((char*)buf);
			
        
        fLib_printf("\r\nNominal Synchronization Jump Width (1-%d): ", CAN_DEV->NPS2);
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->NSJW = atoi((char*)buf);
        
        fLib_printf("\r\nData Prescaler (2-256): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->DPRE = atoi((char*)buf);
				
        fLib_printf("\r\nData Propagation segment (0-8): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->DPROP = atoi((char*)buf);
				
        fLib_printf("\r\nData Phase 1 segment (1-8): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->DPS1 = atoi((char*)buf);
				
        
        //CAN_DEV->DPS2 = CAN_MAX(CAN_DEV->DPS1, IPT_Tq);
        //fLib_printf("\r\nData Phase 2 segment is %d", CAN_DEV->DPS2);
        fLib_printf("\r\nData Phase 2 segment (1-8): ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->DPS2 = atoi((char*)buf);
				
        
        fLib_printf("\r\nData Synchronization Jump Width (1-%d): ", min_t(CAN_DEV->DPS2,4));
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  CAN_DEV->DSJW = atoi((char*)buf);
				
    }
    fLib_printf("\n");
}
 
void CAN_frameType_input(CAN_Param *CAN_DEV)
{
    int frame;
	  char buf[32];
    fLib_printf("\r\nFrame type:");
    if(CAN_DEV->FD_mode == 1)
    {
        fLib_printf("\r\n(0)FD 2.0A      (1)FD 2.0B      (2)FD 2.0A BRS  (3)FD 2.0B BRS: ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  frame = atoi((char*)buf);
			
        CAN_DEV->FrameType = (Frame_Type)frame;
        CAN_DEV->FrameType += 4; //adjust the frame type number
    }
    else
    {
        fLib_printf("\r\n(0)2.0A data    (1)2.0A remote  (2)2.0B data    (3)2.0B remote: ");
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  frame = atoi((char*)buf);
        CAN_DEV->FrameType = (Frame_Type)frame;
    }
    fLib_printf("\n");
}
#ifdef APB_REG_TEST
int CAN_register_test()
{
    int test_item;
		char buf[32];
	
    while(1)
    {
        fLib_printf("\r--------------------------------------------------------------------------------\n");
        fLib_printf("0. Register default value\n");
        fLib_printf("1. Register attribution test\n");
        fLib_printf("2. Quit\n");
        
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  test_item = atoi((char*)buf);			
        fLib_printf("\n");
        
        if(test_item == 0)
        {
            CAN_Register_default_value_test();
        }
        else if(test_item == 1)
        {
            CAN_Register_Attribute_test();
        }
        else 
            return 0;
    }
}
#endif

int CAN_mode_test()
{
    int test_item, mode;
    int ret;
	  char buf[32];
    
    while(1)
    {
        fLib_printf("\r--------------------------------------------------------------------------------\n");
        fLib_printf("0. Mode Change test\n");
        fLib_printf("1. Mode Change test (Burn-in)\n");
        fLib_printf("2. Quit\n");
        
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  test_item = atoi((char*)buf);			
        fLib_printf("\n");        
        
        if(test_item == 0)
        {
            while(mode > 4)
            {
                fLib_printf("Choose mode: (0)Loopback mode (1)Normal mode (2)Listen mode \n(3)Sleep mode (4)Random mode\n");
                fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			          mode = atoi((char*)buf);			
                fLib_printf("\n");
            }
            ret = CAN_Mode_Change_test(mode);
            if(ret != 0)
                fLib_printf("%s",err_msg[ret]);
            else
                fLib_printf("Succussful!\n");
        }
        if(test_item == 1)
        {
            
        }
        else 
            return 0;
    }
}

int CAN_simple_test()
{
    //int npre, nprop, nps1, nps2, nsjw, dpre, dprop, dps1, dps2, dsjw;
    CAN_Param CAN_DEV;
    int test_item, subitem, ret, freerun;
    float SPP;
    UINT32 bitrate;
	  char buf[32];
    
    //fLib_printf("%d\n",CAN_DEV.NPS1);//test dump
    
    
    while(1)
    {
        fLib_printf("\r--------------------------------------------------------------------------------\n");
        fLib_printf("Simple Test:\n");
        fLib_printf("0. Simple tranceive test (FTCAN to FTCAN)\n");
        fLib_printf("1. Data Length test (FTCAN to FTCAN)\n");
        fLib_printf("2. Quit\n");
        
        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  test_item = atoi((char*)buf);			
        fLib_printf("\n");
        fLib_printf("\n");
        
        if(test_item == 0)
        {
            fLib_printf("(0)Polling or (1)Interrupt? ");
						fLib_gets(DEBUG_CONSOLE, (char*)buf);
						subitem = atoi((char*)buf);			
						fLib_printf("\n");					
            fLib_printf("\r\nFree Run? (0:no 1:yes) ");
						fLib_gets(DEBUG_CONSOLE, (char*)buf);
						freerun = atoi((char*)buf);						
            fLib_printf("\n");
            
            CAN_DEV.FreeRun = freerun;
            
            CAN_bit_timing_input(&CAN_DEV, ENABLE);
            CAN_frameType_input(&CAN_DEV);
            
            if(subitem == 0)
            {
                ret = CAN_Simple_Transmission_test(&CAN_DEV);
            }
            else
            {
                ret = CAN_Simple_Transmission_Interrupt_test(&CAN_DEV);
            }
        }
        else if(test_item == 1)
        {
            CAN_bit_timing_input(&CAN_DEV, ENABLE);
            
            fLib_printf("Free Run? (0:no 1:yes) ");
						fLib_gets(DEBUG_CONSOLE, (char*)buf);
						freerun = atoi((char*)buf);
						
            fLib_printf("\n");
            
            CAN_DEV.FreeRun = freerun;
            
            ret = CAN_data_length_test(&CAN_DEV);
        }
        else
            return 0;
        
        /* dump the result message */
        if(ret != 0)
            fLib_printf("\n\r%s",err_msg[ret]);
        else
            fLib_printf("\n\rSuccussful!\n");
    }
}

int CAN_bit_timing_test()
{
    CAN_Param CAN_DEV;
    int test_item, ret, bitrate1, bitrate2, SPP_d1, SPP_d2;
    char opt1 = 'Y', opt2 = 'Y', opt3 = 'N';
    float SPP_f1, SPP_f2;
    
    while(1)
    {
        fLib_printf("\r\n--------------------------------------------------------------------------------\n");
        fLib_printf("0. (Non-FD) Specific bit rate and percentage of sample point test\n");
        fLib_printf("1. (Non-FD) Brute-force bit timing test (Bosch 2.0A/B, TQ=8~25)\n");
        fLib_printf("2. (Non-FD) Brute-force bit timing test (Bosch 2.0A/B, TQ=8~129)\n");
        fLib_printf("3. (FD) Specific bit rate and percentage of sample point test\n");
        fLib_printf("4. (FD) Brute-force bit timing test\n");
        fLib_printf("5. Option settings\n");
        scanf("%d",&test_item);
        fLib_printf("\n");
        
        if(test_item == 0)
        { 
            fLib_printf("Bit rate? (Bps) ");
            scanf("%d",&bitrate1);
            fLib_printf("\r\nSample point percentage x? (x/1000) ");
            scanf("%d",&SPP_d1); //UNDO, the scanf function in no-OS.lite for float type is't support.
            fLib_printf("\n");
            
            SPP_f1 = (float)SPP_d1/1000;
            
            ret = CAN_Bit_Timing_Classical_test1(bitrate1, SPP_f1, opt1, opt2, opt3);
        }
        else if(test_item == 1)
        {
            fLib_printf("\n");
            ret = CAN_Bit_Timing_Classical_test2();
        }
        else if(test_item == 2)
        {
            fLib_printf("\n");
            ret = CAN_Bit_Timing_Classical_test3();
        }
        else if(test_item == 3)
        {
            fLib_printf("Nominal Bit rate? (Bps) ");
            scanf("%d",&bitrate1);
            fLib_printf("\r\nNominal Sample point percentage x? (x/1000) ");
            scanf("%d",&SPP_d1);
            fLib_printf("\r\nData Bit rate? (Bps) ");
            scanf("%d",&bitrate2);
            fLib_printf("\r\nData Sample point percentage x? (x/1000) ");
            scanf("%d",&SPP_d2);
            fLib_printf("\n");
            
            SPP_f1 = (float)SPP_d1/1000;
            SPP_f2 = (float)SPP_d2/1000;
            
            ret = CAN_Bit_Timing_FD_test1(bitrate1, bitrate2, SPP_f1, SPP_f2, opt1, opt2, opt3);
        }
        else if(test_item == 4)
        {
            fLib_printf("\n");
            ret = CAN_Bit_Timing_FD_test2();
        }
        else if(test_item == 5)
        {
            fLib_printf("In the classical CAN, the number of Tq is only 8 to 25? (Y or N) ");
            scanf("%c",&opt1);
            fLib_printf("\r\nFor Bosch spec, NPS1(DPS1) is only equal to NPS2(DPS2)? (Y or N) ");
            scanf("%c",&opt2);
            fLib_printf("\r\nOnly present the largest SJW? (Y or N) ");
            scanf("%c",&opt3);
            fLib_printf("\n");
            
            continue;
        }
        else 
            return 0;
        
        /* dump the result message */
        if(ret != 0)
            fLib_printf("\n\r%s",err_msg[ret]);
        else
            fLib_printf("\n\rFinish!\n");
    }
}

int CAN_SpacialCase_test()
{
    int test_item, ret, flag, freerun;
    CAN_Param CAN_DEV;
    
    while(1)
    {
        fLib_printf("\r\n--------------------------------------------------------------------------------\n");
        fLib_printf("0. ACK error case (single node on CAN bus)\n");
        fLib_printf("1. CRC error case (use PCAN sending ISO CRC format to FTCAN)\n");
        fLib_printf("2. Overrun case\n");
        fLib_printf("3. PEAK CAN error generator test\n");
        fLib_printf("4. Arbitration lost test\n");
        fLib_printf("5. Timestamp test\n");
        fLib_printf("6. Frame Type test\n");
        fLib_printf("7. Random Burn-in test\n");
        fLib_printf("8. Quit\n");
        
        scanf("%d",&test_item);
        fLib_printf("\n");
        
        if(test_item == 0)
        {
            ret = CAN_ACKError_test();
            
            if(ret != 0)
                fLib_printf("\n\r%s",err_msg[ret]);
            else
                fLib_printf("\n\rSuccussful!\n");
        }
        if(test_item == 1)
        {
            ret = CAN_CRCError_test();
            
            if(ret != 0)
                fLib_printf("\n\r%s",err_msg[ret]);
            else
                fLib_printf("\n\rSuccussful!\n");
            
        }
        if(test_item == 2)
        {
            CAN_bit_timing_input(&CAN_DEV, ENABLE);
            
            ret = CAN_Overrun_case_burnin_test(&CAN_DEV);
            
            /* dump the result message */
            if(ret != 0)
                fLib_printf("\n\r%s",err_msg[ret]);
        }
        if(test_item == 3)
        {
            /*先詢問FTCAN是要作transmitter還是receiver*/
            fLib_printf("FTCAN as Transmitter(1) or Receiver(0)? ");
            scanf("%d",&flag);
            fLib_printf("\n");
            if(flag == 1)
            {
                CAN_GenError_Transmitter_test();
            }
            else if(flag == 0)
            {
                CAN_GenError_Receiver_test();
            }
        }
        if(test_item == 4)
        {
            CAN_Arbitration_random_test();
        }
        if(test_item == 5)
        {
            fLib_printf("\r\nFree Run? (0:no 1:yes) ");
            //scanf("%d",&freerun);
					  freerun=1;
            fLib_printf("\n");
            
            CAN_DEV.FreeRun = freerun;
            
            CAN_bit_timing_input(&CAN_DEV, ENABLE);
            CAN_frameType_input(&CAN_DEV);
            
            CAN_TimeStamp_test(&CAN_DEV);
        }
        if(test_item == 6)
        {
            fLib_printf("\r\nFree Run? (0:no 1:yes) ");
            //scanf("%d",&freerun);
					freerun=1;
            fLib_printf("\n");
            
            CAN_DEV.FreeRun = freerun;
            
            CAN_bit_timing_input(&CAN_DEV, ENABLE);
            //CAN_frameType_input(&CAN_DEV, ENABLE);
            
            CAN_Filter_Frame_Type_test(&CAN_DEV);
        }
        if(test_item == 7)
        {
            CAN_Random_BurnIn_test();
        }
        else 
            return 0;
    }
}

int CAN_Identify_use()
{
    CAN_Param CAN_DEV;
    int test_item, ret;
    char buf[32];
    while(1)
    {
        fLib_printf("\r--------------------------------------------------------------------------------\n");
        fLib_printf("Choose cases:\n");
        fLib_printf("0. (PASS) Non-FD 2.0A data frame in 500K 75%%\n");
        fLib_printf("1. (PASS) Non-FD 2.0A data frame in 1M 70%%\n");
        fLib_printf("2. (PASS) FD 2.0A with BRS in 500K 75%% to 1M 85%%\n");
        fLib_printf("3. (PASS) FD 2.0A with BRS in 1M 75%% to 2M 60%%\n");
        fLib_printf("4. (PASS) FD 2.0A with BRS in 500K 75%% to 2M 60%%\n");
        fLib_printf("5. (FAIL) FD 2.0A with BRS in 500K 75%% to 5M 60%%\n");
        fLib_printf("6. (FAIL) FD 2.0A with BRS in 500K 75%% to 5M 70%%\n");
        fLib_printf("7. (PASS) FD 2.0A with BRS in 500K 75%% to 5M 80%%\n");
        fLib_printf("8. Quit\n");
        
        //scanf("%d",&test_item);
			  fLib_gets(DEBUG_CONSOLE, (char*)buf);
		    test_item = atoi((char*)buf);		
        fLib_printf("\n");
        
        if(test_item == 0)
        {
            CAN_DEV.FD_mode = 0;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 10;
            CAN_DEV.NPROP = 9;
            CAN_DEV.NPS1 = 5;
            CAN_DEV.NPS2 = 5;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 0;
            CAN_DEV.DPROP = 0;
            CAN_DEV.DPS1 = 0;
            CAN_DEV.DPS2 = 0;
            CAN_DEV.DSJW = 0;
            /* frame type */
            CAN_DEV.FrameType = CAN_A_Data;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 1)
        {
            CAN_DEV.FD_mode = 0;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 5;
            CAN_DEV.NPROP = 7;
            CAN_DEV.NPS1 = 6;
            CAN_DEV.NPS2 = 6;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 0;
            CAN_DEV.DPROP = 0;
            CAN_DEV.DPS1 = 0;
            CAN_DEV.DPS2 = 0;
            CAN_DEV.DSJW = 0;
            /* frame type */
            CAN_DEV.FrameType = CAN_A_Data;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 2)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 10;
            CAN_DEV.NPROP = 9;
            CAN_DEV.NPS1 = 5;
            CAN_DEV.NPS2 = 5;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 5;
            CAN_DEV.DPROP = 8;
            CAN_DEV.DPS1 = 8;
            CAN_DEV.DPS2 = 3;
            CAN_DEV.DSJW = 3;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 3)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 5;
            CAN_DEV.NPROP = 9;
            CAN_DEV.NPS1 = 5;
            CAN_DEV.NPS2 = 5;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 5;
            CAN_DEV.DPROP = 1;
            CAN_DEV.DPS1 = 4;
            CAN_DEV.DPS2 = 4;
            CAN_DEV.DSJW = 4;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 4)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 10;
            CAN_DEV.NPROP = 9;
            CAN_DEV.NPS1 = 5;
            CAN_DEV.NPS2 = 5;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 5;
            CAN_DEV.DPROP = 1;
            CAN_DEV.DPS1 = 4;
            CAN_DEV.DPS2 = 4;
            CAN_DEV.DSJW = 4;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 5)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            //CAN_DEV.NPRE = 10;
            //CAN_DEV.NPROP = 9;
            //CAN_DEV.NPS1 = 5;
            //CAN_DEV.NPS2 = 5;
            //CAN_DEV.NSJW = 4;
            CAN_DEV.NPRE = 5;
            CAN_DEV.NPROP = 19;
            CAN_DEV.NPS1 = 10;
            CAN_DEV.NPS2 = 10;
            CAN_DEV.NSJW = 10;
            /* Daba bit rate */
            CAN_DEV.DPRE = 2;
            CAN_DEV.DPROP = 1;
            CAN_DEV.DPS1 = 4;
            CAN_DEV.DPS2 = 4;
            CAN_DEV.DSJW = 4;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 6)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            CAN_DEV.NPRE = 10;
            CAN_DEV.NPROP = 9;
            CAN_DEV.NPS1 = 5;
            CAN_DEV.NPS2 = 5;
            CAN_DEV.NSJW = 4;
            /* Daba bit rate */
            CAN_DEV.DPRE = 2;
            CAN_DEV.DPROP = 3;
            CAN_DEV.DPS1 = 3;
            CAN_DEV.DPS2 = 3;
            CAN_DEV.DSJW = 3;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else if(test_item == 7)
        {
            CAN_DEV.FD_mode = 1;
            /* Nominal bit rate */
            //CAN_DEV.NPRE = 10;
            //CAN_DEV.NPROP = 9;
            //CAN_DEV.NPS1 = 5;
            //CAN_DEV.NPS2 = 5;
            //CAN_DEV.NSJW = 4;
            CAN_DEV.NPRE = 5;
            CAN_DEV.NPROP = 19;
            CAN_DEV.NPS1 = 10;
            CAN_DEV.NPS2 = 10;
            CAN_DEV.NSJW = 10;
            /* Data bit rate */
            CAN_DEV.DPRE = 2;
            CAN_DEV.DPROP = 5;
            CAN_DEV.DPS1 = 2;
            CAN_DEV.DPS2 = 2;
            CAN_DEV.DSJW = 2;
            /* frame type */
            CAN_DEV.FrameType = CAN_FD_A_BRS;
            /* Free run */
            CAN_DEV.FreeRun = 0;

            ret = CAN_Simple_Transmission_test(&CAN_DEV);
        }
        else
            return 0;
        
        /* dump the result message */
        if(ret != 0)
            fLib_printf("\n\r%s",err_msg[ret]);
        else
            fLib_printf("\n\rSuccussful!\n");
    }
}



int CAN_test_menu()
{
    int test_item;
	  char buf[32];

/* 根據CAN_test.h印出目前CAN1, CAN2, CAN3的資訊 */

    fLib_printf("CAN1: FTCAN(APB)  , Base Address: 0x56300000\n");
    fLib_printf("CAN2: FTCAN(APB)  , Base Address: 0x54900000\n");
//    fLib_printf("CAN3: FTCAN(APB)  , Base Address: 0x99000000");    

    while(1)
    {
        fLib_printf("\r\n--------------------------------------------------------------------------------\n");
        fLib_printf("(Main Menu) Choose CAN test item:\n");
#ifdef APB_REG_TEST			
        fLib_printf("0. Regitser test\n");
#endif
        fLib_printf("1. Mode test\n");
        fLib_printf("2. Simple test\n");
        fLib_printf("3. Bit timing test\n");
        fLib_printf("4. Special Case test\n");
        fLib_printf("5. Identify test use case\n");

        fLib_gets(DEBUG_CONSOLE, (char*)buf);
 			  test_item = atoi((char*)buf);

        fLib_printf("\n");
#ifdef APB_REG_TEST        
        if(test_item == 0)
        {
            CAN_register_test();
        }
#else
				if(0){}
#endif			
        else if(test_item == 1)
        {
            CAN_mode_test();
        }
        else if(test_item == 2)
        {
            CAN_simple_test();
        }
        else if(test_item == 3)
        {
            CAN_bit_timing_test();
        }
        else if(test_item == 4)
        {
            CAN_SpacialCase_test();
        }
        else if(test_item == 5)
        {
            CAN_Identify_use();
        }
        else
            return 0;
    }
}
