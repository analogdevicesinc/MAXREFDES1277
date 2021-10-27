
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
* @file           V_measure.cpp
* @brief          Contains functions for voltage measurement. 
* @version        1.0
* @notes          Complies in arduino IDE complier. Tested using Ardunio 1.8.12.
*****************************************************************************/
 
#include "V_measure.h"

/**
 * @brief      Voltage measuremnt sequence.
 * @details    Function is voltage measurment sequence for sending Single scan, cell voltage calculation and block voltage calculation.
 * @return     None.
 */
void V_measure::V_measure_sequence()
{ 
  single_scan();
  delay(100);
  cell_V_cal();
  block_V_cal();
}

/**
 * @brief      Single scan sequence
 * @details    Function is used to request a new measurement sequence (scan) and initiate a data transfer.
 * @return     None.
 */
void V_measure::single_scan()
{ 
  //SPI TRANSACTION : Reading status of ADDR_SCANCTRL 
  
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_SCANCTRL,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_SCANCTRL,DATA_CHECK
  bms_SPI.SPI_commands(7,WR_LD_Q0,0x06,READALL,ADDR_SCANCTRL,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);//SPI command for Read Load Que
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, 0x00);
  bms_SPI.SPI_commands(2,READ_RX_STATUS, 0x00);

 //SPI TRANSACTION : Writing ADDR_SCANCTRL with Oversampling 
 
  PEC_VALUE=pec.pec_code(4,WRITEALL,ADDR_SCANCTRL,0x31,0x10);//PEC calculation for BITS WRITEALL, ADDR_MEASUREEN, 0xFF, 0xFF
  bms_SPI.SPI_commands(8,WR_LD_Q0,0x05,WRITEALL,ADDR_SCANCTRL,0x31,0x10,PEC_VALUE,ALIVE_COUNTER);//only 128
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(6, RD_NXT_MSG,NULL_XX,NULL_XX, NULL_XX, NULL_XX, NULL_XX);
  PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
  bms_SPI.SPI_commands(2, READ_RX_INTERRUPT_FLAGS, NULL_XX);
  bms_SPI.SPI_commands(2, 0x01, NULL_XX);
}

/**
 * @brief      Cell Voltage calculation.
 * @details    Function is for individual voltage measurement. Reads each cell register and converts the raw value to cell voltage.
 * @return     None.
 */
void V_measure::cell_V_cal()
{
  Serial.println("MAXREFDES1277 VOLTAGE MEASUREMENTS");
  Serial.println();
  Serial.println("Cell Voltages");
  for (int cell_pointer = ADDR_CELL1REG ; cell_pointer <= ADDR_CELL14REG  ; cell_pointer++)
  {
  //SPI TRANSACTION : READ CELL REGISTERS 
  
     PEC_VALUE=pec.pec_code(3, READALL,cell_pointer,DATA_CHECK);//PEC calculation for BITS READALL,cell_pointers,DATA_CHECK
     bms_SPI.SPI_commands(7,WR_LD_Q0, 0x06, READALL,cell_pointer,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER);
     bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
     SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);//SPI command for Read Load Que
     PEC_check_status=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 
     
     unsigned int raw = (SPI_return[3]) + (SPI_return[4] << 8); // local variable 
     float cell_voltage = (raw >> 2) *VOLTAGE_REF/ VOLTAGE_REF_HEX ;
     Serial.print("Cell ");
     Serial.print(cell_pointer-ADDR_CELL1REG+1);
     Serial.print(" HEX value : ");
     Serial.print(SPI_return[4],HEX);
     Serial.print(SPI_return[3],HEX);
     Serial.print(" Decimal value : ");
     Serial.println(cell_voltage, 3);  
  }
}

/**
 * @brief      Block voltage measurment.
 * @details    Function is for reading block voltage register and converts raw value to Block voltage.
 * @return     None.
 */
void V_measure::block_V_cal()
{
  PEC_VALUE=pec.pec_code(3,READALL,ADDR_BLOCKREG,DATA_CHECK);//PEC calculation for BITS READALL,ADDR_BLOCKREG,DATA_CHECK
  bms_SPI.SPI_commands(7,WR_LD_Q0,0x06,READALL,ADDR_BLOCKREG,DATA_CHECK,PEC_VALUE,ALIVE_COUNTER); 
  bms_SPI.SPI_commands(1, WR_NXT_LD_Q0);
  SPI_return=bms_SPI.SPI_commands(8, RD_NXT_MSG, NULL_XX, NULL_XX, NULL_XX, NULL_XX, NULL_XX,NULL_XX, NULL_XX);
  PEC_VALUE=pec.PEC_Check(1,5,SPI_return);//Checks the calculated and hardware returned PEC 

  unsigned int raw = (SPI_return[3]) + (SPI_return[4] << 8);// local variable 
  float block_voltage = (raw >> 2) * FULL_SCALE_DCIN / FULL_SCALE_DCIN_HEX;
  Serial.println();
  Serial.print("Block Voltage Hex Value: ");
  Serial.print(SPI_return[4],HEX);
  Serial.print(SPI_return[3],HEX);
  Serial.print(" Decimal Value:");
  Serial.println(block_voltage, 3);
  Serial.println();
}
