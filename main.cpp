#include <Arduino.h>
#include "BLEDevice.h"
//#include "BLEScan.h"

#include "ssid.h"
#include "bc85.h"

// { // BC85
static BLEUUID serviceUUID(BLEUUID((uint16_t)0x1810));
//static BLEUUID serviceUUID("00001810-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID(BLEUUID((uint16_t)0x2A35));
// }

static BLEAddress (*pServerAddress);
static BLEClient *bclient = NULL;
static BLERemoteCharacteristic *pRemoteCharacteristic;

int state = 0;
static bc85 *bp = new bc85();

/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID))
        {
            advertisedDevice.getScan()->stop();
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            Serial.print("BLE Addres: ");
            Serial.println(advertisedDevice.getAddress().toString().c_str());

            Serial.println("Found our device!");
            state = 1;
        }
    }
};

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *bclient)
    {
        Serial.println("onConnect");
    }

    void onDisconnect(BLEClient *bclient)
    {
        Serial.println("onDisconnect");
    }

    void onOpen(BLEClient *bclient)
    {
        Serial.println("onOpen");
        state = 2;
    }

    void onClose(BLEClient *bclient)
    {
        Serial.println("onClose");
        state = 1;
    }
};

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    if (!isNotify)
        Serial.print("Indication callback for characteristic:");
    else
        Serial.print("Notification callback for characteristic:");

    for (int i = 0; i < length; i++)
    {
        Serial.print(pData[i]);
        Serial.print(" ");
    }
    Serial.println();

    bp->process(pData, length);
}

void ble_scan()
{
    //pServerAddress = nullptr;
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(10, true);
}

void blec_register()
{
    bclient = BLEDevice::createClient();
    bclient->setClientCallbacks(new MyClientCallback());
    bclient->app_register();

    Serial.println("Registered a new client");
}

bool blec_open(BLEAddress pAddress)
{
    // Connect to the remote BLE Server.
    if (bclient->open(pAddress))
        return true;

    return false;
}

bool blec_enable_callback(BLEUUID sUUID, BLEUUID cUUID, notify_callback ncb)
{
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = bclient->getService(sUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(sUUID.toString().c_str());
        return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(cUUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(cUUID.toString().c_str());
        return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());

    Serial.println("Registering for indication/notification callback");
    pRemoteCharacteristic->registerForNotify(ncb, false);

    return true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");

    BLEDevice::init("");
    blec_register();
}

void loop()
{
    if (bclient != NULL)
    {
        switch (state)
        {
        case 0:
            ble_scan();
            break;

        case 1: // disconnected
            if (!bclient->isConnected())
            {
                if (pServerAddress == nullptr)
                {
                    state = 0;
                    Serial.println("No matching server, go back to scanning");
                }
                else
                {
                    Serial.print("Opening a virtual connection to ");
                    Serial.println(pServerAddress->toString().c_str());
                    if (blec_open(*pServerAddress))
                    {
                        Serial.println("Opened virtual connection to BLE Server.");
                    }
                    else
                    {
                        state = 0;
                        Serial.println("Connected, but unable to open virtual connection, go back to scanning");
                    }
                }
            }
            else
            {
                Serial.println("onDisconnect received, but isConnected() == true");
            }
            break;
        case 2: //connected, register notifications
            if (blec_enable_callback(serviceUUID, charUUID, notifyCallback))
            {
                Serial.println("Success! Waiting for callbacks");
                state = 3;
            }
            else
                state = 1;
            break;
        }
    }
}
