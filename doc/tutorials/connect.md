# Connect to device

[中文](./connect_zh_CN.md)  
## Windows
### RFCOMM (preferred)

1. Pair your device in system settings
2. Make sure your device is not connected to other Bluetooth host  
(Disconnected or connected to the host you are using)
3. Open this app
4. Go to `Device` panel
5. Click `Search RFCOMM`, wait until your device name appears
6. Click your device name in the list, the MAC address at the bottom will be updated
7. Click `Connect`
8. Go to `Generic Device` panel after `Device Connected` is shown at the bottom
9. (Optional) Click `Read Settings` to sync settings from device

### BLE (not recommended)
It's nearly impossible to connect to device on Windows using BLE due to the limitation in Qt API and the randomized Bluetooth MAC addresses on some devices. However, you could still try these steps if you want.

1. Pair your device in system settings
2. Make sure your device is connected to any Bluetooth host, but not controlled by other Edifier Connect app or mEDIFIER app  
3. Remove all devices named `EDIFIER BLE` in the system settings
4. Connect to the device named `EDIFIER BLE` in the system settings  
(This might fail for some devices)
5. Open this app
6. Go to `Device` panel
7. Click `Search BLE`, wait for a while
8. Click the device named `EDIFIER BLE`, the MAC address at the bottom will be updated
9. Click `Connect`
10. Go to `Generic Device` panel after `Device Connected` is shown at the bottom
11. (Optional) Click `Read Settings` to sync settings from device

## Android
### RFCOMM

1. Pair your device in system settings
2. Make sure your device is not connected to other Bluetooth host  
(Disconnected or connected to the host you are using)
3. Open this app
4. Go to `Device` panel
5. Click `Search RFCOMM`, wait until your device name appears
6. Click your device name in the list, the MAC address at the bottom will be updated
7. Click `Connect`
8. Go to `Generic Device` panel after `Device Connected` is shown
9. (Optional) Click `Read Settings` to sync settings from device

### BLE (preferred)

1. Pair your device in system settings
2. Make sure your device is connected to any Bluetooth host, but not controlled by other Edifier Connect app or mEDIFIER app  
3. Open this app
4. Go to `Device` panel
5. Click `Search BLE`, wait for a while
6. Click the device named `EDIFIER BLE`, the MAC address at the bottom will be updated
7. Click `Connect`
8. Go to `Generic Device` panel after `Device Connected` is shown
9. (Optional) Click `Read Settings` to sync settings from device

## Linux & macOS
This app theoretically works on Linux and macOS, but I don't have the environments to test it. You could try the steps for Windows and Android above.

## Connect without searching
It's possible to connect to a paired device without searching.  
This app remembers the MAC address and the connection type you used last time. You can try to click `Connect` without searching. It works sometimes.
