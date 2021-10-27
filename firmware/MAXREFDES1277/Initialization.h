
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
* @file           Initialization.h
* @brief          Contains header files required for Initialization.cpp file. 
* @version        1.0
* @notes          Complies in arduino IDE complier. Tested using Ardunio 1.8.12.
*****************************************************************************/
 
#ifndef INITIALIZATION_H
#define INITIALIZATION_H
#include "Arduino.h"
#include "PEC.h"
#include "BMS_SPI.h"
#include "Max17841.h"
#include "Max17852.h" 

#define BAUDRATE               115200 // Baud rate for Serial UART communication PC to Microcontroller
#define SPI_FREQUENCY          400000 //SPI frequency
#define SS1                    53    //Slave select in microcontroller for uno use 10 mega 53
#define SS2                    49    //Optional Slave select if Dual UART is used
#define SHDNL_MAX17841_1       48 //Shut down pin for MAX17841_1
#define SHDNL_MAX17841_2       46 //Optional Shut down pin for MAX17841_2 if Dual UART communication is used

class Initialization
{ 
  public:
  void Arduino_Mega_init();
  void MAX1785X_init_sequence();
  void wakeup();
  void helloall();
  void clear_status_fmea();
  void enable_measurement();
};
extern Initialization initialization;
#endif
