/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <stdio.h>
#include "project.h"
#include "PIDControl.h"
#include "I2C.h"
#define LM75_ADDRESS (0x48)
#define SAMPLES_PER_SECOND 3

static uint16_t sampleWaitTimeInMilliseconds = 1000 / SAMPLES_PER_SECOND;

uint8_t responseData[0] ;
uint8_t addr = 0 ; 

static char outputBuffer[256];
static float setPoint = 30; // degrees celcius
char str[128] ;
CY_ISR_PROTO(ISR_UART_rx_handler);
void handleByteReceived(uint8_t byteReceived);
void Read2Bytes(uint8_t addr, uint8_t data[]);
float getTemp(void);
void increaseTemp();
void decreaseTemp();

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /*initialization/startup code here (e.g. MyInst_Start()) */
    isr_uart_rx_StartEx(ISR_UART_rx_handler);   
    UART_1_Start();
    PWM_Power_Start();
    I2C_Start();

    float Kp = 2.0f;
    float Ki = 1.0f/30.0f;
    float Kd = 0.0f;
    float integralMax = 3000;
    float integralMin = -3000;
    float temp = 0;
    float controlSignal = 0;

    float dt = ((float)sampleWaitTimeInMilliseconds) / 1000; // dt is measured in seconds
    PIDControl_init(Kp, Ki, Kd, integralMax, integralMin, dt);
    PIDControl_changeSetPoint(setPoint);

    UART_1_PutString("Temperature control application started\r\n");

    for(;;)
    {
        //Gets temperature from LM75
        temp = getTemp();
        
        float error = setPoint - temp;
        float proportionalPart = 0;
        float integralPart = 0;
        float derivativePart = 0;
        
        controlSignal = PIDControl_doStep(temp, &proportionalPart, &integralPart, &derivativePart);            
        snprintf(outputBuffer, sizeof(outputBuffer), "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f \r\n", 
                                                      setPoint, temp, error, controlSignal, Kp, Ki, Kd, 
                                                      proportionalPart, integralPart, derivativePart);
        if(controlSignal > 100){
            controlSignal = 100;
        }
        else if(controlSignal < 0){
            controlSignal = 0;
        }
        
        PWM_Power_WriteCompare(controlSignal);
        UART_1_PutString(outputBuffer);
        CyDelay(sampleWaitTimeInMilliseconds);
    }
}
CY_ISR(ISR_UART_rx_handler)
{
    uint8_t bytesToRead = UART_1_GetRxBufferSize();
    while (bytesToRead > 0)
    {
        uint8_t byteReceived = UART_1_ReadRxData();
        UART_1_WriteTxData(byteReceived); // echo back
        
        handleByteReceived(byteReceived);
        
        bytesToRead--;
    }
}
void handleByteReceived(uint8_t byteReceived)
{
    switch(byteReceived)
    {
        case 'u' :
        {
            increaseTemp();
        }
        break;
        case 'd' :
        {
            decreaseTemp();
        }
        break;
    }
}
void Read2Bytes(uint8_t addr, uint8_t data[])
{
    uint8_t status;
    I2C_MasterClearStatus();
    status = I2C_MasterSendStart(LM75_ADDRESS, I2C_WRITE_XFER_MODE) ;
    
    if (I2C_MSTR_NO_ERROR == status) {
        I2C_MasterWriteByte(addr); 
        I2C_MasterSendRestart(LM75_ADDRESS, I2C_READ_XFER_MODE);  
    }
    
    if (I2C_MSTR_NO_ERROR == status) {   
        data[0] = I2C_MasterReadByte(I2C_ACK_DATA); // Integer
        data[1] = I2C_MasterReadByte(I2C_NAK_DATA); // Fraction
    }
    I2C_MasterSendStop(); 
}

float getTemp(void){
    Read2Bytes(addr, responseData);
    float temp  = (float)(((responseData[0] << 8) | responseData[1]) >> 5) * 0.125;
    return temp;
}

void increaseTemp(){
    setPoint = setPoint + 5;
    UART_1_PutString(printf("TARGET TEMP CHANGED TO: %s\r\n", gcvt(setPoint,2,str)));
    PIDControl_changeSetPoint(setPoint);
}
void decreaseTemp(){
    setPoint = setPoint - 5;
    UART_1_PutString(printf("TARGET TEMP CHANGED TO: %s\r\n", gcvt(setPoint,2,str)));
    PIDControl_changeSetPoint(setPoint);
}
    //I2C Communication

/* [] END OF FILE */
