package io.github.wh201906.medifier.qt;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothProfile.ServiceListener;
import android.util.Log;
import android.content.Context;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class ProfileProxyHelper implements ServiceListener
{
    private static final String TAG = "ProfileProxyHelper";

    public static final int STATE_UNCONNECTED = 0;
    public static final int STATE_CONNECTING_A2DP = 1;
    public static final int STATE_CONNECTING_HEADSET = 2;
    public static final int STATE_CONNECTED = 3;

    private Context mContext = null;
    private BluetoothDevice mDevice = null;
    private BluetoothA2dp mA2dpProfile = null;
    private BluetoothHeadset mHeadsetProfile = null;
    private int mState = STATE_UNCONNECTED;

    public ProfileProxyHelper(Context context)
    {
        mContext = context;
    }

    @Override
    public void onServiceConnected(int profile, BluetoothProfile proxy)
    {

        Log.i(TAG, "profile: " + String.valueOf(profile) + ", connectionState:" + String.valueOf(proxy.getConnectionState(mDevice)));
        if (profile == BluetoothProfile.A2DP)
            mA2dpProfile = (BluetoothA2dp) proxy;
        else if (profile == BluetoothProfile.HEADSET)
            mHeadsetProfile = (BluetoothHeadset) proxy;
        onServiceStateChanged(profile, true);
    }

    @Override
    public void onServiceDisconnected(int profile)
    {
        Log.i(TAG, "Service disconnected. profile: " + String.valueOf(profile));
        if (profile == BluetoothProfile.A2DP)
            mA2dpProfile = null;
        else if (profile == BluetoothProfile.HEADSET)
            mHeadsetProfile = null;
        onServiceStateChanged(profile, false);
    }

    public void connectToDevice(String address)
    {
        BluetoothAdapter BTAdapter = BluetoothAdapter.getDefaultAdapter();
        // UpperCase is mandatory there
        mDevice = BTAdapter.getRemoteDevice(address.toUpperCase());

        mState = STATE_CONNECTING_A2DP;
        if (!BTAdapter.getProfileProxy(mContext, this, BluetoothProfile.A2DP) && !BTAdapter.getProfileProxy(mContext, this, BluetoothProfile.HEADSET))
            mState = STATE_UNCONNECTED;
    }

    public void setTargetDevice(BluetoothDevice device)
    {
        mDevice = device;
    }

    private void onServiceStateChanged(int profile, boolean serviceConnected)
    {
        if (!serviceConnected)
        {
            boolean deviceConnected = false;
            if (mA2dpProfile == null && mHeadsetProfile == null)
                deviceConnected = false;
            else
            {
                if (mA2dpProfile != null)
                    deviceConnected = deviceConnected || (mA2dpProfile.getConnectionState(mDevice) == BluetoothProfile.STATE_CONNECTED);
                if (mHeadsetProfile != null)
                    deviceConnected = deviceConnected || (mHeadsetProfile.getConnectionState(mDevice) == BluetoothProfile.STATE_CONNECTED);
            }
            mState = deviceConnected ? STATE_CONNECTED : STATE_UNCONNECTED;
        }
        else
        {
            if (mState == STATE_CONNECTED)
            {
                Log.i(TAG, "mDevice is already connected");
                return;
            }

            if (profile == BluetoothProfile.A2DP)
                mState = STATE_CONNECTING_A2DP;
            else if (profile == BluetoothProfile.HEADSET)
                mState = STATE_CONNECTING_HEADSET;

            if (connectProfileToDevice(profile))
                mState = STATE_CONNECTED;
            else if (mState == STATE_CONNECTING_A2DP)
            {
                // fallback: try BluetoothHeadset
                mState = STATE_CONNECTING_HEADSET;
                if (!BluetoothAdapter.getDefaultAdapter().getProfileProxy(mContext, this, BluetoothProfile.HEADSET))
                    mState = STATE_UNCONNECTED;
            }
            else
                mState = STATE_UNCONNECTED;
        }


    }

    private boolean connectProfileToDevice(int profile)
    {
        Method connectMethod = getConnectMethod(profile);
        Method connectAudioMethod = getConnectAudioMethod();
        if (connectMethod == null)
        {
            return false;
        }
        if (mDevice == null)
        {
            Log.e(TAG, "mDevice is not set");
            return false;
        }

        try
        {
            connectMethod.setAccessible(true);
            if (profile == BluetoothProfile.A2DP)
            {
                boolean result = ((Boolean) connectMethod.invoke(mA2dpProfile, mDevice)).booleanValue();
                return result;
            }
            else if (profile == BluetoothProfile.HEADSET)
            {
                boolean result = ((Boolean) connectMethod.invoke(mHeadsetProfile, mDevice)).booleanValue();
                if (result && connectAudioMethod != null)
                {
                    // just try it anyway, idk if it helps
                    connectAudioMethod.invoke(mHeadsetProfile);
                }
                return result;
            }
            else
            {
                Log.e(TAG, "Unexpected profile: " + String.valueOf(profile));
                return false;
            }
        } catch (InvocationTargetException e)
        {
            Log.e(TAG, "Unable to invoke method on proxy. " + e.toString());
        } catch (IllegalAccessException e)
        {
            Log.e(TAG, "Illegal Access! " + e.toString());
        }
        return false;
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
