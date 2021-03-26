/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University, UK.

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


#include "MakeAble.h"
#include "Timer.h"
#include "MicroBitDevice.h"
#include "PinMap.h"

using namespace codal;

/**
  * Constructor.
  *
  * Create a representation of a micro:bit device, which includes member variables
  * that represent various device drivers used to control aspects of the micro:bit.
  */
MakeAble::MakeAble() :
#if CONFIG_ENABLED(DEVICE_BLE)
    bleManager(),
    ble( &bleManager),
#endif
    systemTimer(NRF_TIMER1, TIMER1_IRQn),
    adcTimer(NRF_TIMER2, TIMER2_IRQn),
    capTouchTimer(NRF_TIMER3, TIMER3_IRQn),
    timer(systemTimer),
    messageBus(),
    adc(adcTimer, 91),
    io(adc),
    i2c(PIN_I2C_SDA, PIN_I2C_SCL),
    internalFlash(MICROBIT_STORAGE_PAGE, 1, MICROBIT_CODEPAGESIZE),
    storage(internalFlash, 0),
    buttonA(io.reset, DEVICE_ID_BUTTON_A, DEVICE_BUTTON_ALL_EVENTS, ACTIVE_LOW),
    radio(),
    thermometer()
{
    // Clear our status
    status = 0;
}

/**
  * Post constructor initialisation method.
  *
  * This call will initialised the scheduler, memory allocator and Bluetooth stack.
  *
  * This is required as the Bluetooth stack can't be brought up in a
  * static context i.e. in a constructor.
  *
  * @code
  * uBit.init();
  * @endcode
  *
  * @note This method must be called before user code utilises any functionality
  *       contained within the GenuinoZero class.
  */
int MakeAble::init()
{
    if (status & DEVICE_INITIALIZED)
        return DEVICE_NOT_SUPPORTED;

    status |= DEVICE_INITIALIZED;

    // Bring up fiber scheduler.
    scheduler_init(messageBus);

    for(int i = 0; i < DEVICE_COMPONENT_COUNT; i++)
    {
        if(CodalComponent::components[i])
            CodalComponent::components[i]->init();
    }

    // Seed our random number generator
    seedRandom();

    // Create an event handler to trap any handlers being created for I2C services.
    // We do this to enable initialisation of those services only when they're used,
    // which saves processor time, memeory and battery life.
    messageBus.listen(DEVICE_ID_MESSAGE_BUS_LISTENER, DEVICE_EVT_ANY, this, &MakeAble::onListenerRegisteredEvent);

#if CONFIG_ENABLED(DMESG_SERIAL_DEBUG)
#if DEVICE_DMESG_BUFFER_SIZE > 0
    codal_dmesg_set_flush_fn(microbit_dmesg_flush);
#endif
#endif
    status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;

    // Set IRQ priorities for peripherals we use.
    // Note that low values have highest priority, and only 2, 3, 4, 5 and 7 are available with SoftDevice enabled.

    NVIC_SetPriority(TIMER1_IRQn, 7);         // System timer (general purpose)
    NVIC_SetPriority(TIMER2_IRQn, 5);         // ADC timer.
    NVIC_SetPriority(TIMER3_IRQn, 3);         // Cap touch.
    NVIC_SetPriority(TIMER4_IRQn, 3);         // Display and Light Sensing.

    NVIC_SetPriority(SAADC_IRQn, 5);          // Analogue to Digital Converter (microphone etc)
    NVIC_SetPriority(PWM0_IRQn, 5);           // General Purpose PWM on edge connector (servo, square wave sounds)
    NVIC_SetPriority(PWM1_IRQn, 4);           // PCM audio on speaker (high definition sound)
    NVIC_SetPriority(PWM2_IRQn, 3);           // Waveform Generation (neopixel)

    NVIC_SetPriority(RADIO_IRQn, 4);          // Packet radio
    NVIC_SetPriority(UARTE0_UART0_IRQn, 2);   // Serial port
    NVIC_SetPriority(GPIOTE_IRQn, 2);         // Pin interrupt events

#if CONFIG_ENABLED(DEVICE_BLE) && CONFIG_ENABLED(MICROBIT_BLE_PAIRING_MODE)
    int i=0;
    // Test if we need to enter BLE pairing mode
    // If a RebootMode Key has been set boot straight into BLE mode
    KeyValuePair* RebootMode = storage.get("RebootMode");
    KeyValuePair* flashIncomplete = storage.get("flashIncomplete");
    sleep(100);
    // Animation
    uint8_t x = 0; uint8_t y = 0;
    while ((buttonA.isPressed() && i<25) || RebootMode != NULL || flashIncomplete != NULL)
    {
        // display.image.setPixelValue(x,y,255);
        sleep(50);
        i++; x++;

        // Gradually fill screen
        if(x == 5){
          y++; x = 0;
        }

        if (i == 25 || RebootMode != NULL)
        {
            // Remove KV if it exists
            if(RebootMode != NULL){
                storage.remove("RebootMode");
            }
            delete RebootMode;
            delete flashIncomplete;

            // Start the BLE stack, if it isn't already running.
            bleManager.init( ManagedString( microbit_friendly_name()), getSerial(), messageBus, storage, true);
            
            // Enter pairing mode, using the LED matrix for any necessary pairing operations
            bleManager.pairingMode(display, buttonA);
        }
    }
#endif

#if CONFIG_ENABLED(DEVICE_BLE) && CONFIG_ENABLED(MICROBIT_BLE_ENABLED)
    // Start the BLE stack, if it isn't already running.
    bleManager.init( ManagedString( microbit_friendly_name()), getSerial(), messageBus, storage, false);
#endif

    // Deschedule for a little while, just to allow for any components that finialise initialisation
    // as a background task, and to allow the power mamanger to repsonse to background events from the KL27
    // before any user code begins running.
    
    sleep(10);

    return DEVICE_OK;
}

/**
  * A callback listener to disable default audio streaming to P0 if an event handler is registered on that pin.
  */
void MakeAble::onP0ListenerRegisteredEvent(Event evt)
{
}

/**
  * A listener to perform actions as a result of Message Bus reflection.
  *
  * In some cases we want to perform lazy instantiation of components, such as
  * the compass and the accelerometer, where we only want to add them to the idle
  * fiber when someone has the intention of using these components.
  */
void MakeAble::onListenerRegisteredEvent(Event evt)
{
    switch(evt.value)
    {
        case DEVICE_ID_THERMOMETER:
            // A listener has been registered for the thermometer.
            // The thermometer uses lazy instantiation, we just need to read the data once to start it running.
            //thermometer.updateSample();
            break;
    }
}

/**
  * A periodic callback invoked by the fiber scheduler idle thread.
  * We use this for any low priority, backgrounf housekeeping.
  *
  */
void MakeAble::idleCallback()
{
#if CONFIG_ENABLED(DMESG_SERIAL_DEBUG)
#if DEVICE_DMESG_BUFFER_SIZE > 0
    codal_dmesg_flush();
#endif
#endif
}

void microbit_dmesg_flush()
{
#if CONFIG_ENABLED(DMESG_SERIAL_DEBUG)
#if DEVICE_DMESG_BUFFER_SIZE > 0
    if (codalLogStore.ptr > 0 && microbit_device_instance)
    {
        for (uint32_t i=0; i<codalLogStore.ptr; i++)
            ((MicroBit *)microbit_device_instance)->serial.putc(codalLogStore.buffer[i]);

        codalLogStore.ptr = 0;
    }
#endif
#endif
}

