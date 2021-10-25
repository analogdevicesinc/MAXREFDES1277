
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
* @file           PEC.cpp
* @brief          Contains the implemention of the PEC calculation and PEC validation functions. 
* @version        1.0
* @notes          Complies in arduino IDE complier. Tested using Ardunio 1.8.12.
*****************************************************************************/
#include "PEC.h"

int poly=0b10110010; //PEC/CRC polynomial

/**
* @brief      Function is used PEC code generation.
* @details    PEC code generation 
* @param[in]  "n" Gives the number of variable list of arguments.
* @param[in]  "..." Variable list of arguments for PEC calculation.
* @return     returns "remainder"- calculated PEC value.   
*/
 
int PEC::pec_code(int n, ...)
{
  va_list arguments;
  va_start ( arguments, n );
  int remainder=0x00;
 for (int j=0;j<n;j++){
    remainder=remainder^va_arg(arguments, int);
    for (int i=0;i<8;i++){
      int bitStatus = remainder & 1;// Just to compare the last bit we are operating and with 0x01
      if (bitStatus==1){
        remainder = remainder>>1;
        remainder=remainder^poly;
      }else{
        remainder=remainder>>1;
      }
    }
}
va_end ( arguments );
return remainder;
};

/**
* @brief      Function is used for PEC checking of retured transaction.
* @details    PEC Validation of returned SPI values.
* @param[in]  "Start" Starting array index of recived SPI transaction for PEC calculation.
* @param[in]  "End" Ending array index of recived SPI transaction for PEC calculation.
* @param[in]  "return_value[]" array which has the returned SPI transaction.   
* @return     returns "1" if PEC is matched and "0" if PEC is not matched.   
*/
int PEC::PEC_Check(int Start, int End,int return_value[])
{
  int remainder=0x00;
  for (int q=0;q<=End;q++){

   remainder=remainder^return_value[q];
   for (int i=0;i<8;i++){
     int bitStatus = remainder & 1;
     if (bitStatus==1){
       remainder = remainder>>1;
       remainder=remainder^poly;
     }else{
       remainder=remainder>>1;
     }
   }
}
 if (remainder!=return_value[6])
 {
  return 0;
 }
 else{
  return 1;
 }
};
