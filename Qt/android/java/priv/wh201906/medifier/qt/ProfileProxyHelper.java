package priv.wh201906.medifier.qt;

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothProfile.ServiceListener;
import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class ProfileProxyHelper implements ServiceListener
{
    private static final String TAG = "ProfileProxyHelper";
    private BluetoothDevice mDevice = null;

    @Override
    public void onServiceConnected(int profile, BluetoothProfile proxy)
    {
        Method connectMethod = getConnectMethod(profile);
        Method connectAudioMethod = getConnectAudioMethod();
        if (connectMethod == null)
            return;
        if (mDevice == null)
        {
            Log.e(TAG, "mDevice is not set");
            return;
        }
        try
        {
            connectMethod.setAccessible(true);
            if (profile == BluetoothProfile.HEADSET)
            {
                connectMethod.invoke((BluetoothHeadset) proxy, mDevice);
                if (connectAudioMethod == null)
                    return;
                connectAudioMethod.invoke((BluetoothHeadset) proxy);
            }
            else connectMethod.invoke((BluetoothA2dp) proxy, mDevice);
        } catch (InvocationTargetException e)
        {
            Log.e(TAG, "Unable to invoke method on proxy. " + e.toString());
        } catch (IllegalAccessException e)
        {
            Log.e(TAG, "Illegal Access! " + e.toString());
        }
    }

    @Override
    public void onServiceDisconnected(int profile)
    {
        ;
    }

    public void setTargetDevice(BluetoothDevice device)
    {
        mDevice = device;
    }

    private Method getConnectMethod(int profile)
    {
        try
        {
            if (profile == BluetoothProfile.HEADSET)
                // https://dl.google.com/developers/android/udc/non-sdk/hiddenapi-flags.csv (Android 14)
                // Landroid/bluetooth/BluetoothHeadset;->connect(Landroid/bluetooth/BluetoothDevice;)Z,sdk,system-api,test-api
                return BluetoothHeadset.class.getDeclaredMethod("connect", BluetoothDevice.class);
            else
                // https://dl.google.com/developers/android/udc/non-sdk/hiddenapi-flags.csv (Android 14)
                // Landroid/bluetooth/BluetoothA2dp;->connect(Landroid/bluetooth/BluetoothDevice;)Z,unsupported
                return BluetoothA2dp.class.getDeclaredMethod("connect", BluetoothDevice.class);
        } catch (NoSuchMethodException e)
        {
            Log.e(TAG, "Unable to find connect(BluetoothDevice) method in profile proxy.");
            return null;
        }
    }

    private Method getConnectAudioMethod()
    {
        try
        {
            // https://dl.google.com/developers/android/udc/non-sdk/hiddenapi-flags.csv (Android 14)
            // Landroid/bluetooth/BluetoothHeadset;->connectAudio()I,sdk,system-api,test-api
            return BluetoothHeadset.class.getDeclaredMethod("connectAudio");
        } catch (NoSuchMethodException e)
        {
            Log.e(TAG, "Unable to find connectAudio() method in BluetoothHeadset proxy.");
            return null;
        }
    }

}
