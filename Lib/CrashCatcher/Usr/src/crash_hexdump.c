//
// Created by Cyril Fougeray on 2019-08-27.
//

#include <CrashCatcher.h>
#include "sys_ll_serial.h"
#include "user_error.h"


/** Disable IRQ */
#define DISABLE_IRQ()   __asm volatile ( " cpsid i " ::: "memory" )

static void _uart_init()
{
    (void)SYS_LL_UartInit(SYS_LL_SERIAL_0);
}

static void _print(uint8_t* buf, uint8_t len)
{
    (void)SYS_LL_UartSend(SYS_LL_SERIAL_0, buf, len);
}

static void dumpHexDigit(uint8_t nibble)
{
    static const char hexToASCII[] = "0123456789ABCDEF";

    ERR_ASSERT( nibble < 16 );
    _print((uint8_t *) &hexToASCII[nibble], 1);
}

static void dumpByteAsHex(uint8_t byte)
{
    dumpHexDigit(byte >> 4);
    dumpHexDigit(byte & 0xF);
}

static void dumpBytes(const uint8_t* pMemory, size_t elementCount)
{
    size_t i;
    uint8_t new_line[2] = "\r\n";

    for (i = 0 ; i < elementCount ; i++)
    {
        /* Only dump 16 bytes to a single line before introducing a line break. */
        if (i != 0 && (i & 0xF) == 0)
            _print(new_line, sizeof(new_line));
        dumpByteAsHex(*pMemory++);
    }
}

static void dumpHalfwords(const uint16_t* pMemory, size_t elementCount)
{
    size_t i;
    uint8_t new_line[2] = "\r\n";

    for (i = 0 ; i < elementCount ; i++)
    {
        uint16_t val = *pMemory++;
        /* Only dump 8 halfwords to a single line before introducing a line break. */
        if (i != 0 && (i & 0x7) == 0)
            _print(new_line, sizeof(new_line));
        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

static void dumpWords(const uint32_t* pMemory, size_t elementCount)
{
    size_t i;
    uint8_t new_line[2] = "\r\n";

    for (i = 0 ; i < elementCount ; i++)
    {
        uint32_t val = *pMemory++;
        /* Only dump 4 words to a single line before introducing a line break. */
        if (i != 0 && (i & 0x3) == 0)
            _print(new_line, sizeof(new_line));
        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

/* Called at the beginning of crash dump. You should provide an implementation which prepares for the dump by opening
   a dump file, prompting the user to begin a crash dump, or whatever makes sense for your scenario. */
void CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)
{
    DISABLE_IRQ();

    _uart_init();

    uint8_t crash_start_flag[17] = "\r\n\r\n###CRASH###\r\n";
    _print(crash_start_flag, sizeof(crash_start_flag));
}

/* Called to obtain an array of regions in memory that should be dumped as part of the crash.  This will typically
   be all RAM regions that contain volatile data.  For some crash scenarios, a user may decide to also add peripheral
   registers of interest (ie. dump some ethernet registers when you are encountering crashes in the network stack.)
   If NULL is returned from this function, the core will only dump the registers. */
const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
{
    // static const CrashCatcherMemoryRegion regions[] ={
    //     {0x20000000, 0x20000100, CRASH_CATCHER_BYTE}, /* RAM content */
    //     {0xFFFFFFFF, 0xFFFFFFFF, CRASH_CATCHER_BYTE}
    // };
    // return regions;
    return 0;
}

/* Called to dump the next chunk of memory to the dump (this memory may point to register contents which has been copied
   to memory by CrashCatcher already.  The element size will be 8-bits, 16-bits, or 32-bits.  The implementation should
   use reads of the specified size since some memory locations may only support the indicated size. */
void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)
{
    uint8_t new_line[2] = "\r\n";

    switch (elementSize)
    {
        case CRASH_CATCHER_BYTE:
            dumpBytes(pvMemory, elementCount);
            break;
        case CRASH_CATCHER_HALFWORD:
            dumpHalfwords(pvMemory, elementCount);
            break;
        case CRASH_CATCHER_WORD:
            dumpWords(pvMemory, elementCount);
            break;
    }
    _print(new_line, sizeof(new_line));
}

/* Called at the end of crash dump. You should provide an implementation which cleans up at the end of dump. This could
   include closing a dump file, blinking LEDs, infinite looping, and/or returning CRASH_CATCHER_TRY_AGAIN if
   CrashCatcher should prepare to dump again incase user missed the first attempt. */
CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)
{
    uint8_t crash_end_flag[12] = "###END###\r\n";
    _print(crash_end_flag, sizeof(crash_end_flag));

    for(;;);

    return CRASH_CATCHER_EXIT;
}

/*EOF*/