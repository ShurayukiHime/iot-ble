uint8_t temp = static_cast<unsigned int>(sensor_value);
uint8_t hum = static_cast<unsigned int>(humidity_value);

uint8_t service_data[4];
service_data[0] = GAPButtonUUID & 0xff;
service_data[1] = GAPButtonUUID >> 8;
service_data[2] = temp;
service_data[3] = hum;

BLE::Instance().gap().updateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, (uint8_t *)service_data, sizeof(service_data));