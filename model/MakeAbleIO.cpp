/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/**
  * Class definition for MicroBit NEXT IO.
  * Represents a collection of all I/O pins on the device.
  */

#include "CodalConfig.h"
#include "CodalDmesg.h"
#include "MakeAbleIO.h"
#include "PinMap.h"

using namespace codal;

/**
  * Constructor.
  *
  * Create a representation of all given I/O pins on the edge connector
  *
  * Accepts a sequence of unique ID's used to distinguish events raised
  * by MicroBitPin instances on the default EventModel.
  */
MakeAbleIO::MakeAbleIO(NRF52ADC &a) :
    jacdac(ID_PIN_P0, PIN_JACDAC_DATA, PIN_CAPABILITY_AD),
    led(ID_PIN_P1, PIN_STATUS_LED, PIN_CAPABILITY_AD),
    ledR(ID_PIN_P2, PIN_LEDR, PIN_CAPABILITY_AD),
    ledG(ID_PIN_P3, PIN_LEDG, PIN_CAPABILITY_AD),
    ledB(ID_PIN_P4, PIN_LEDB, PIN_CAPABILITY_AD),
    ilimFault(ID_PIN_P5, PIN_ILIM_FAULT, PIN_CAPABILITY_AD),
    ilimCtrl(ID_PIN_P6, PIN_ILIM_CTRL, PIN_CAPABILITY_AD),
    vbusSense(ID_PIN_P7, PIN_VBUS_SENSE, PIN_CAPABILITY_AD),
    cc1(ID_PIN_P8, PIN_CC1, PIN_CAPABILITY_AD),
    cc2(ID_PIN_P9, PIN_CC2, PIN_CAPABILITY_AD),
    nc(ID_PIN_P10, PIN_NC, PIN_CAPABILITY_DIGITAL),
    irq1(nc)
{
    pins = 11;
    NRF52Pin::adc = &a;

    // Ensure all internal pins are configured with no pull resistors.
    for (int i=11; i<pins; i++)
        pin[i].setPull(PullMode::None);

    savedStatus = ManagedBuffer(pins);
}

#ifdef NRF_P1
#define PORT (name < 32 ? NRF_P0 : NRF_P1)
#define PIN ((name) & 31)
#define NUM_PINS 48
#else
#define PORT (NRF_P0)
#define PIN (name)
#define NUM_PINS 32
#endif

/**
 * Puts the component in (or out of) sleep (low power) mode.
 */
int MakeAbleIO::setSleep(bool doSleep)
{
    int name;

    if (doSleep)
    {
        // Record current state of pins, so we can return the configuration to the same state later.       
        for (int i = 0; i < pins; i++)
        {
            name = pin[i].name;
            savedStatus[i] = 0;

            // Disable any interrupts associated with this GPIO pin
            if ((PORT->PIN_CNF[PIN] & GPIO_PIN_CNF_SENSE_Msk) == (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos))
                savedStatus[i] |= IO_SAVED_STATUS_DETECT_LOW_ENABLED;

            // Disable any interrupts associated with this GPIO pin
            if ((PORT->PIN_CNF[PIN] & GPIO_PIN_CNF_SENSE_Msk) == (GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos))
                savedStatus[i] |= IO_SAVED_STATUS_DETECT_HIGH_ENABLED;

            pin[i].setDetect(GPIO_PIN_CNF_SENSE_Disabled);

#if CONFIG_ENABLED(DEEP_SLEEP_GPIO_OUTPUTS)
            // If this pin is in GPIO output mode, record its value and set to high impedance input.
            // Other components are reponsible for storing/restoring the state of higher level peripherals.
            if (pin[i].isOutput())
            {
                savedStatus[i] |= PORT->OUT & (1 << PIN) ? IO_SAVED_STATUS_OUTPUT_HI : IO_SAVED_STATUS_OUTPUT_LO;
                PORT->PIN_CNF[PIN] = 0x00000002;
            }
#endif

        }
    }

    // Restore any saved GPIO state
    if (!doSleep)
    {
        for (int i = 0; i < pins; i++)
        {
            name = pin[i].name;

#if CONFIG_ENABLED(DEEP_SLEEP_GPIO_OUTPUTS)
            // Re-enable GPIO outputs that were placed into high impedance mode.
            if (savedStatus[i] & IO_SAVED_STATUS_OUTPUT_HI){
                PORT->OUTSET = 1 << PIN;
                PORT->PIN_CNF[PIN] = PORT->PIN_CNF[PIN] | 1;
            }

            if (savedStatus[i] & IO_SAVED_STATUS_OUTPUT_LO){
                PORT->OUTCLR = 1 << PIN;
                PORT->PIN_CNF[PIN] = PORT->PIN_CNF[PIN] | 1;
            }
#endif

            // Re-enable any interrupts associated with this GPIO pin
            if(savedStatus[i] & IO_SAVED_STATUS_DETECT_LOW_ENABLED)
                pin[i].setDetect(GPIO_PIN_CNF_SENSE_Low);

            if(savedStatus[i] & IO_SAVED_STATUS_DETECT_HIGH_ENABLED)
                pin[i].setDetect(GPIO_PIN_CNF_SENSE_High);
        }
    }
   
    return DEVICE_OK;
}
