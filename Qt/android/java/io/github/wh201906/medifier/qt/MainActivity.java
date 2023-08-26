package io.github.wh201906.medifier.qt;

import org.qtproject.qt5.android.bindings.QtActivity;

import android.util.Log;
import android.view.WindowManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.net.Uri;
import android.bluetooth.*;
import android.os.Process;

import java.util.Set;
import java.lang.String;
import java.util.ArrayList;

public class MainActivity extends QtActivity
{
    private static final String LOG_TAG = "MainActivity";
    private ProfileProxyHelper mPPHelper = null;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(LOG_TAG, "qt.mainactivity");
        mPPHelper = new ProfileProxyHelper(getApplicationContext());
    }

    public String[] getBondedDevices(boolean isBLE)
    {
        BluetoothAdapter BTAdapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> bondedDevice = BTAdapter.getBondedDevices();
        ArrayList<String> list = new ArrayList<String>();
        for (BluetoothDevice bt : bondedDevice)
        {
            int deviceType = bt.getType();
            if (!isBLE && (deviceType == BluetoothDevice.DEVICE_TYPE_CLASSIC || deviceType == BluetoothDevice.DEVICE_TYPE_DUAL))
                list.add(bt.getAddress() + " " + bt.getName());
            else if (isBLE && (deviceType == BluetoothDevice.DEVICE_TYPE_LE || deviceType == BluetoothDevice.DEVICE_TYPE_DUAL))
                list.add(bt.getAddress() + " " + bt.getName());
        }
        String[] result = (String[]) list.toArray(new String[list.size()]);
        return result;
    }

    public void connectToDevice(String address)
    {
        mPPHelper.connectToDevice(address);
    }
}
