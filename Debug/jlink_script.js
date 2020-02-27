/*****************************************************************************
 *                   SEGGER Microcontroller GmbH & Co. KG                    *
 *            Solutions for real time microcontroller applications           *
 *****************************************************************************
 *                                                                           *
 *               (c) 2015 SEGGER Microcontroller GmbH & Co. KG               *
 *                                                                           *
 *           Internet: www.segger.com   Support: support@segger.com          *
 *                                                                           *
 *****************************************************************************/

int ConfigTargetSettings(void)
{
    if (JLINK_SetDevice("STM32G070CB") >= 0)
    {
        JLINK_SYS_Report("Set device OK\r\n");
    }
    else
    {
        JLINK_SYS_Report("Set device ERROR\r\n");
    }
    return 0;
}

int InitTarget(void)
{
    return 0;
}