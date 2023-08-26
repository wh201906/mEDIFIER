# Switch connection between Bluetooth hosts

[中文](./switch_host_zh_CN.md)  
Sometimes you might need to switching the audio connection between two paired hosts. Typically, it requires these steps:  

1. Long-press the "pairing" button on your device  
2. Connect to the device in system settings, which needs a few clicks  

These steps take some time, and wear your "pairing" button gradually.  
I found that the some of the devices can be connected to one host for audio, then be controlled by the other host using BLE. According to this interesting feature, I wrote some code to simplify the switching process. Now you can switch the audio connection to an Android host in the mEDIFIER app.  

# Steps (for Android only)

1. Open this app  
2. [Connect to your device in mEDIFIER](./connect.md), using BLE  
3. Go to the device panel  
4. Click `Connect Audio`  

It's possible to switch the audio connection in 4 clicks:  
Open mEDIFIER -> Click `Connect`(Connect without searching) -> Go to device panel -> Click `Connect Audio`  
