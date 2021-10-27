
/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
* 
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
/**
* @file           Initialization.cpp
* @brief          Contains the initialization sequence commands. 
* @version        1.0
* @notes          Complies in arduino IDE complier. Tested using Ardunio 1.8.12.
*****************************************************************************/
#include "Initialization.h"

/**
* @brief      Function is used for initialization of Arduino microcontroller.
* @details    The pin definition for enabling MAX17841, slave select,SPI mode,SPI frequency selection for SPI communication.
* @return     None  
*/
void Initialization::Arduino_Mega_init()
{
  Serial.begin(BAUDRATE);
  Serial.println("MAXREFDES1277 Voltage measurement Code Version1");
  pinMode(SHDNL_MAX17841_1,OUTPUT);
  pinMode(SS1, OUTPUT);    // Setting the Slave Select Pin as Output
  digitalWrite(SS1, HIGH); // disable Slave Select Pin
  digitalWrite(SHDNL_MAX17841_1,HIGH); // Enable MAX17841
  delay(2);
  SPI.begin();            // Initializing the SPI in Micro-controller
  SPI.beginTransaction(SPISettings(SPI_FREQUENCY , MSBFIRST, SPI_MODE0));// Initializing SPI frequency, SPI mode
}
/**
 * @brief      Runs Initialization sequence for waking MAX17853/2 BMS IC.
 * @details    Sends Wake up, Hello all, clears status/ FMEA alerts and enables measurments in cell channels. 
 * @return     None
 */
void Initialization::MAX1785X_init_sequence()
{
  wakeup();// Wake up instructions for start up
  helloall();// Send Hello all command
  clear_status_fmea(); // Clears status and FMEA
  enable_measurement();// SPI commands to enable measurement
}

/**
 * @brief      Function is wakeup SPI commands for BMS IC.
 * @details    Used to wake up the UART slave devices and initialize BMS ICs in daizy chain.
 * @return     None.
 */
void Initialization::wakeup()
{
  bms_SPI.SPI_commands(2,WRITE_CONFIGURATION_3, KEEP_ALIVE_160_MICRO_SEC );//Write the configuration 3 register and set keep-alive period to 160us
  bms_SPI.SPI_commands(2,WRITE_RX_INTERRUPT_ENABLE, 0x88);//Write Rx_interrupt enable register and set RX_Error_int_enable and RX_Overflow_INT_Enable bits
  bms_SPI.SPI_commands(1, CLR_RX_BUF); // Clear Receive Buffer
  bms_SPI.SPI_commands(2,WRITE_CONFIGURATION_2, 0x30); //WRITE_CONFIGURATION_2 and Enable transmit preambles mode
  bms_SPI.SPI_commands(1, READ_RX_STATUS,  NULL_XX); //Read RX_Status regiter and check for RX_Status for 0x21, Otherwise repeat transactions
  bms_SPI.SPI_commands(2,WRITE_CONFIGURATION_2, 0x10); //WRITE_CONFIGURATION_2 and Disable transmit preambles mode
  bms_SPI.SPI_commands(1,READ_RX_STATUS); //Read RX_Status register
  bms_SPI.SPI_commands(1, CLR_TX_BUF);//Clear transmit buffer
  bms_SPI.SPI_commands(1, CLR_RX_BUF);//Clear receive buffer
}

/**
 * @brief      Function is used for setup of BMS address and initialization.
 * @details    The HELLOALL command initializes the daisy chain device addresses. 
               The host performs this assignment by specifying a seed address in the HELLOALL command sequence. 
               As the command propagates up the daisy-chain, each slave device assigns its own address. 
 * @return     None.
 */
void Initialization::helloall()
{
  bms_SPI.SPI_commands(5, WR_LD_Q0, 0x03, HELLOALL,NULL_XX,NULL_XX ); //Send the Helloall command to initialize the address of the BMS daizy chain
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0); // Write Next queue
  SPI_return=bms_SPI.SPI_commands(6, RD_NXT_MSG,NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX);
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);// Read RX_interrupt flags
  bms_SPI.SPI_commands(2,READ_RX_STATUS , NULL_XX); // Read RX status
  
Serial.print("Number of devices in daisy chain is : ");
Serial.println(SPI_return[3]);
}

/**
 * @brief      Function is used for clering the fmea alerts.
 * @details    This clears all fmea faults on startup.
 * @return     None.
 */
void Initialization::clear_status_fmea()
{  
  //SPI TRANSACTION : WRITE ADDR_STATUS1 TO 0X00 TO CLEAR FLAGS
  
  PEC_VALUE=pec.pec_code(4, WRITEALL, ADDR_STATUS1, 0x00, 0x00);//PEC calculation for BITS WRITEALL, ADDR_STATUS1N, 0x00, 0x00
  bms_SPI.SPI_commands(8, WR_LD_Q0, 0x06, WRITEALL, ADDR_STATUS1,0x00, 0x00,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0); // Write Next queue
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 

  //SPI TRANSACTION : READ ADDR_STATUS1 FOR RETURNED RESULTS
  
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_STATUS1,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_STATUS1,DATA_CHECK
  bms_SPI.SPI_commands(7, WR_LD_Q0, 0x06,READALL,ADDR_STATUS1,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);//SPI command for Read Load Que
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);// Read RX_interrupt flags
  bms_SPI.SPI_commands(2,READ_RX_STATUS , NULL_XX); // Read RX status

  //SPI TRANSACTION : WRITE ADDR_FMEA1 TO 0X00 TO CLEAR FLAGS
  
  PEC_VALUE=pec.pec_code(4, WRITEALL, ADDR_FMEA1, 0x00, 0x00);//PEC calculation for BITS WRITEALL, ADDR_FMEA1, 0x00, 0x00
  bms_SPI.SPI_commands(8, WR_LD_Q0, 0x06, WRITEALL, ADDR_FMEA1,0x00, 0x00,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0); // Write Next queue
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 

  //SPI TRANSACTION : READ ADDR_FMEA1 FOR RETURNED RESULTS
 
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_FMEA1,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_FMEA1,DATA_CHECK
  bms_SPI.SPI_commands(7, WR_LD_Q0, 0x06,READALL,ADDR_FMEA1,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);

  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);// Read RX_interrupt flags
  
  bms_SPI.SPI_commands(2,READ_RX_STATUS , NULL_XX); // Read RX status
  
  //SPI TRANSACTION : WRITE ADDR_FMEA2 TO 0X00 TO CLEAR FLAGS
  
  PEC_VALUE=pec.pec_code(4, WRITEALL, ADDR_FMEA2, 0x00, 0x00);//PEC calculation for BITS WRITEALL, ADDR_FMEA2,0x00, 0x00
  bms_SPI.SPI_commands(8, WR_LD_Q0, 0x06, WRITEALL, ADDR_FMEA2,0x00, 0x00,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0); 
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 

  //SPI TRANSACTION : READ ADDR_FMEA2 FOR RETURNED RESULTS
 
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_FMEA2,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_MEASUREEN,DATA_CHECK
  bms_SPI.SPI_commands(7, WR_LD_Q0, 0x06,READALL,ADDR_FMEA2,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);//SPI command for Read Load Que
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);// Read RX_interrupt flags
  bms_SPI.SPI_commands(2,READ_RX_STATUS , NULL_XX); // Read RX status

}
/**
 * @brief      Function is used for enabling cell voltage measurement.
 * @details    Enables measurement of the respective cell in the acquisition mode.
 * @return     None.
 */
void Initialization::enable_measurement()
{ 
  //SPI TRANSACTION : WRITE MEASURE_ENABLE1 WITH ALL ENABLE ie 0xFF 0xFF
  
  PEC_VALUE=pec.pec_code(4, WRITEALL, ADDR_MEASUREEN1, 0xFF, 0xFF);//PEC calculation for BITS WRITEALL, ADDR_MEASUREEN1, 0xFF, 0xFF
  bms_SPI.SPI_commands(8, WR_LD_Q0, 0x06, WRITEALL, ADDR_MEASUREEN1,0xFF, 0xFF,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0); // Write Next queue
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 

 //SPI TRANSACTION : READ MEASURE_ENABLE1 
 
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_MEASUREEN1,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_MEASUREEN,DATA_CHECK
  bms_SPI.SPI_commands(7, WR_LD_Q0, 0x06,READALL,ADDR_MEASUREEN1,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);// Read RX_interrupt flags
  bms_SPI.SPI_commands(2,READ_RX_STATUS , NULL_XX); // Read RX status
}
