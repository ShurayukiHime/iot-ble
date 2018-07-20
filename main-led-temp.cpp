/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "LEDService.h"
#include "ble/services/HealthThermometerService.h"

DigitalOut  led1(LED1, 1);
InterruptIn button(BLE_BUTTON_PIN_NAME);

// Change your device name below
const char DEVICE_NAME[] = "GAPButt";

/* We can arbiturarily choose the GAPButton service UUID to be 0xAA00
 * as long as it does not overlap with the UUIDs defined here:
 * https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx */
#define GAPButtonUUID 0xAA00
const uint16_t uuid16_list[] = {GAPButtonUUID, LEDService::LED_SERVICE_UUID, GattService::UUID_HEALTH_THERMOMETER_SERVICE};

static EventQueue eventQueue(/* event count */ 16 * EVENTS_EVENT_SIZE);
LEDService *ledServicePtr;
static HealthThermometerService *thermometerServicePtr;
float sensor_value = 0;
float humidity_value = 0;
DigitalOut alivenessLED(LED1, 0);
DigitalOut actuatedLED(LED2, 0);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void) params;
    BLE::Instance().gap().startAdvertising();
}

void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == ledServicePtr->getValueHandle()) && (params->len == 1)) {
        actuatedLED = *(params->data);
    }
}

void print_error(ble_error_t error, const char* msg)
{
    printf("%s: ", msg);
    switch(error) {
        case BLE_ERROR_NONE:
            printf("BLE_ERROR_NONE: No error");
            break;
        case BLE_ERROR_BUFFER_OVERFLOW:
            printf("BLE_ERROR_BUFFER_OVERFLOW: The requested action would cause a buffer overflow and has been aborted");
            break;
        case BLE_ERROR_NOT_IMPLEMENTED:
            printf("BLE_ERROR_NOT_IMPLEMENTED: Requested a feature that isn't yet implement or isn't supported by the target HW");
            break;
        case BLE_ERROR_PARAM_OUT_OF_RANGE:
            printf("BLE_ERROR_PARAM_OUT_OF_RANGE: One of the supplied parameters is outside the valid range");
            break;
        case BLE_ERROR_INVALID_PARAM:
            printf("BLE_ERROR_INVALID_PARAM: One of the supplied parameters is invalid");
            break;
        case BLE_STACK_BUSY:
            printf("BLE_STACK_BUSY: The stack is busy");
            break;
        case BLE_ERROR_INVALID_STATE:
            printf("BLE_ERROR_INVALID_STATE: Invalid state");
            break;
        case BLE_ERROR_NO_MEM:
            printf("BLE_ERROR_NO_MEM: Out of Memory");
            break;
        case BLE_ERROR_OPERATION_NOT_PERMITTED:
            printf("BLE_ERROR_OPERATION_NOT_PERMITTED");
            break;
        case BLE_ERROR_INITIALIZATION_INCOMPLETE:
            printf("BLE_ERROR_INITIALIZATION_INCOMPLETE");
            break;
        case BLE_ERROR_ALREADY_INITIALIZED:
            printf("BLE_ERROR_ALREADY_INITIALIZED");
            break;
        case BLE_ERROR_UNSPECIFIED:
            printf("BLE_ERROR_UNSPECIFIED: Unknown error");
            break;
        case BLE_ERROR_INTERNAL_STACK_FAILURE:
            printf("BLE_ERROR_INTERNAL_STACK_FAILURE: internal stack faillure");
            break;
    }
    printf("\r\n");
}

void updatePayload(void)
{
    uint8_t temp = static_cast<unsigned int>(sensor_value);
    uint8_t hum = static_cast<unsigned int>(humidity_value);
    
    // Update the count in the SERVICE_DATA field of the advertising payload
    uint8_t service_data[4];
    service_data[0] = GAPButtonUUID & 0xff;
    service_data[1] = GAPButtonUUID >> 8;
    service_data[2] = temp;
    service_data[3] = hum;
    ble_error_t err = BLE::Instance().gap().updateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, (uint8_t *)service_data, sizeof(service_data));
    if (err != BLE_ERROR_NONE) {
        print_error(err, "Updating payload failed");
    }
}

void blinkCallback(void)
{
    led1 = !led1;
}

void checkTemp(void)
{
    sensor_value = BSP_TSENSOR_ReadTemp();
    humidity_value = BSP_HSENSOR_ReadHumidity();    
    //eventQueue.call(updatePayload);
    uint8_t temp = static_cast<unsigned int>(sensor_value);
    thermometerServicePtr->updateTemperature(temp);
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *context)
{
    BLE&        ble = context->ble;
    ble_error_t err = context->error;

    if (err != BLE_ERROR_NONE) {
        print_error(err, "BLE initialisation failed");
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);

    /* Setup primary service. */
    bool initialValueForLEDCharacteristic = false;
    ledServicePtr = new LEDService(ble, initialValueForLEDCharacteristic);
    uint8_t temp = static_cast<unsigned int>(sensor_value);
    thermometerServicePtr = new HealthThermometerService(ble, temp, HealthThermometerService::LOCATION_EAR);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::THERMOMETER_EAR);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    // Blink LED every 500 ms to indicate system aliveness
    eventQueue.call_every(500, blinkCallback);
    
    BSP_TSENSOR_Init();
    BSP_HSENSOR_Init();
    // this line not good
    sensor_value = BSP_TSENSOR_ReadTemp();

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble_error_t err = ble.init(bleInitComplete);
    if (err != BLE_ERROR_NONE) {
        print_error(err, "BLE initialisation failed");
        return 0;
    }

    eventQueue.call_every(1000, checkTemp);
    eventQueue.dispatch_forever();

    return 0;
}
