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

#ifndef MAKEABLE_IO_H
#define MAKEABLE_IO_H

#include "CodalConfig.h"
#include "NRF52Pin.h"
#include "NRF52ADC.h"
#include "TouchSensor.h"
#include "PinMap.h"

//
// Component IDs for each pin.
// The can be user defined, but uniquely identify a pin when using the eventing APIs/
//
#define ID_PIN_P0        (DEVICE_ID_IO_P0 + 0)
#define ID_PIN_P1        (DEVICE_ID_IO_P0 + 1)
#define ID_PIN_P2        (DEVICE_ID_IO_P0 + 2)
#define ID_PIN_P3        (DEVICE_ID_IO_P0 + 3)
#define ID_PIN_P4        (DEVICE_ID_IO_P0 + 4)
#define ID_PIN_P5        (DEVICE_ID_IO_P0 + 5)
#define ID_PIN_P6        (DEVICE_ID_IO_P0 + 6)
#define ID_PIN_P7        (DEVICE_ID_IO_P0 + 7)
#define ID_PIN_P8        (DEVICE_ID_IO_P0 + 8)
#define ID_PIN_P9        (DEVICE_ID_IO_P0 + 9)
#define ID_PIN_P10       (DEVICE_ID_IO_P0 + 10)
#define ID_PIN_P11       (DEVICE_ID_IO_P0 + 11)
#define ID_PIN_P12       (DEVICE_ID_IO_P0 + 12)
#define ID_PIN_P13       (DEVICE_ID_IO_P0 + 13)
#define ID_PIN_P14       (DEVICE_ID_IO_P0 + 14)
#define ID_PIN_P15       (DEVICE_ID_IO_P0 + 15)
#define ID_PIN_P16       (DEVICE_ID_IO_P0 + 16)
#define ID_PIN_P17       (DEVICE_ID_IO_P0 + 17)
#define ID_PIN_P18       (DEVICE_ID_IO_P0 + 18)
#define ID_PIN_P19       (DEVICE_ID_IO_P0 + 19)
#define ID_PIN_P20       (DEVICE_ID_IO_P0 + 20)
#define ID_PIN_SDA       (DEVICE_ID_IO_P0 + 24)
#define ID_PIN_SCL       (DEVICE_ID_IO_P0 + 25)
#define ID_PIN_ROW1      (DEVICE_ID_IO_P0 + 26)
#define ID_PIN_ROW2      (DEVICE_ID_IO_P0 + 27)
#define ID_PIN_ROW3      (DEVICE_ID_IO_P0 + 28)
#define ID_PIN_ROW4      (DEVICE_ID_IO_P0 + 29)
#define ID_PIN_ROW5      (DEVICE_ID_IO_P0 + 30)
#define ID_PIN_USBTX     (DEVICE_ID_IO_P0 + 31)
#define ID_PIN_USBRX     (DEVICE_ID_IO_P0 + 32)
#define ID_PIN_IRQ1      (DEVICE_ID_IO_P0 + 33)
#define ID_PIN_MIC       (DEVICE_ID_IO_P0 + 34)
#define ID_PIN_P35       (DEVICE_ID_IO_P0 + 35)
#define ID_PIN_P36       (DEVICE_ID_IO_P0 + 36)
#define ID_PIN_P37       (DEVICE_ID_IO_P0 + 37)
#define ID_PIN_P38       (DEVICE_ID_IO_P0 + 38)
#define ID_PIN_P39       (DEVICE_ID_IO_P0 + 39)
#define ID_PIN_P40       (DEVICE_ID_IO_P0 + 40)
#define ID_PIN_P41       (DEVICE_ID_IO_P0 + 41)
#define ID_PIN_P42       (DEVICE_ID_IO_P0 + 42)
#define ID_PIN_P43       (DEVICE_ID_IO_P0 + 43)
#define ID_PIN_P44       (DEVICE_ID_IO_P0 + 44)
#define ID_PIN_P45       (DEVICE_ID_IO_P0 + 45)
#define ID_PIN_P46       (DEVICE_ID_IO_P0 + 46)
#define ID_PIN_P47       (DEVICE_ID_IO_P0 + 47)

//
// Saved status values to restore GPIO configurations after deep sleep
//
#define IO_SAVED_STATUS_OUTPUT_NA               0
#define IO_SAVED_STATUS_OUTPUT_LO               1
#define IO_SAVED_STATUS_OUTPUT_HI               2
#define IO_SAVED_STATUS_DETECT_LOW_ENABLED      4
#define IO_SAVED_STATUS_DETECT_HIGH_ENABLED     8

namespace codal
{
    /**
     * Represents a collection of all I/O pins exposed by the device.
     */
    class MakeAbleIO : public CodalComponent
    {
        private:
            ManagedBuffer     savedStatus;

        public:
            // Number of pins in use.
            int               pins;

            // Enumeration of all pins, ordered by edge connector.
            NRF52Pin          pin[0];

            NRF52Pin          jacdac;
            NRF52Pin          led;
            NRF52Pin          ledR;
            NRF52Pin          ledG;
            NRF52Pin          ledB;
            NRF52Pin          ilimFault;
            NRF52Pin          ilimCtrl;
            NRF52Pin          vbusSense;
            NRF52Pin          cc1;
            NRF52Pin          cc2;
            NRF52Pin          nc;
            NRF52Pin&          irq1;

            /**
             * Constructor.
             *
             * Create a representation of all given I/O pins on the edge connector
             *
             * Accepts a sequence of unique ID's used to distinguish events raised
             * by MicroBitPin instances on the default EventModel.
             */
            MakeAbleIO(NRF52ADC &a);

            /**
             * Puts the component in (or out of) sleep (low power) mode.
             */
            virtual int setSleep(bool doSleep) override;
    };
}

#endif
