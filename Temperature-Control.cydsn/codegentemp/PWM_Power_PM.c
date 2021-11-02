/*******************************************************************************
* File Name: PWM_Power_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PWM_Power.h"

static PWM_Power_backupStruct PWM_Power_backup;


/*******************************************************************************
* Function Name: PWM_Power_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Power_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_Power_SaveConfig(void) 
{

    #if(!PWM_Power_UsingFixedFunction)
        #if(!PWM_Power_PWMModeIsCenterAligned)
            PWM_Power_backup.PWMPeriod = PWM_Power_ReadPeriod();
        #endif /* (!PWM_Power_PWMModeIsCenterAligned) */
        PWM_Power_backup.PWMUdb = PWM_Power_ReadCounter();
        #if (PWM_Power_UseStatus)
            PWM_Power_backup.InterruptMaskValue = PWM_Power_STATUS_MASK;
        #endif /* (PWM_Power_UseStatus) */

        #if(PWM_Power_DeadBandMode == PWM_Power__B_PWM__DBM_256_CLOCKS || \
            PWM_Power_DeadBandMode == PWM_Power__B_PWM__DBM_2_4_CLOCKS)
            PWM_Power_backup.PWMdeadBandValue = PWM_Power_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_Power_KillModeMinTime)
             PWM_Power_backup.PWMKillCounterPeriod = PWM_Power_ReadKillTime();
        #endif /* (PWM_Power_KillModeMinTime) */

        #if(PWM_Power_UseControl)
            PWM_Power_backup.PWMControlRegister = PWM_Power_ReadControlRegister();
        #endif /* (PWM_Power_UseControl) */
    #endif  /* (!PWM_Power_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_Power_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Power_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Power_RestoreConfig(void) 
{
        #if(!PWM_Power_UsingFixedFunction)
            #if(!PWM_Power_PWMModeIsCenterAligned)
                PWM_Power_WritePeriod(PWM_Power_backup.PWMPeriod);
            #endif /* (!PWM_Power_PWMModeIsCenterAligned) */

            PWM_Power_WriteCounter(PWM_Power_backup.PWMUdb);

            #if (PWM_Power_UseStatus)
                PWM_Power_STATUS_MASK = PWM_Power_backup.InterruptMaskValue;
            #endif /* (PWM_Power_UseStatus) */

            #if(PWM_Power_DeadBandMode == PWM_Power__B_PWM__DBM_256_CLOCKS || \
                PWM_Power_DeadBandMode == PWM_Power__B_PWM__DBM_2_4_CLOCKS)
                PWM_Power_WriteDeadTime(PWM_Power_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_Power_KillModeMinTime)
                PWM_Power_WriteKillTime(PWM_Power_backup.PWMKillCounterPeriod);
            #endif /* (PWM_Power_KillModeMinTime) */

            #if(PWM_Power_UseControl)
                PWM_Power_WriteControlRegister(PWM_Power_backup.PWMControlRegister);
            #endif /* (PWM_Power_UseControl) */
        #endif  /* (!PWM_Power_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_Power_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Power_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_Power_Sleep(void) 
{
    #if(PWM_Power_UseControl)
        if(PWM_Power_CTRL_ENABLE == (PWM_Power_CONTROL & PWM_Power_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_Power_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_Power_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_Power_UseControl) */

    /* Stop component */
    PWM_Power_Stop();

    /* Save registers configuration */
    PWM_Power_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_Power_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_Power_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Power_Wakeup(void) 
{
     /* Restore registers values */
    PWM_Power_RestoreConfig();

    if(PWM_Power_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_Power_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
