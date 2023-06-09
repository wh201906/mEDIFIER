package priv.wh201906.medifier;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity
{
    private static final String TAG = "mEDIFIER";
    private final UUID SPP_UUID = UUID.fromString("EDF00000-EDFE-DFED-FEDF-EDFEDFEDFEDF");
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothSocket mBluetoothSocket;
    private OutputStream mOutputStream;
    private boolean mConnected = false;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button connectButton = findViewById(R.id.connectButton);
        ListView commandListView = findViewById(R.id.commandListView);

        connectButton.setOnClickListener(view ->
        {
            mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

            if (mBluetoothAdapter == null)
            {
                Toast.makeText(getApplicationContext(), R.string.toast_no_bluetooth, Toast.LENGTH_SHORT).show();
            }
            if (!mBluetoothAdapter.isEnabled())
            {
                Toast.makeText(getApplicationContext(), R.string.toast_bluetooth_not_open, Toast.LENGTH_SHORT).show();
            }
            else
            {
                connectToDevice();
            }

        });

        InputStream inputStream = getResources().openRawResource(R.raw.cmd);
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder stringBuilder = new StringBuilder();
        String line;
        try
        {
            while ((line = reader.readLine()) != null)
                stringBuilder.append(line);
            reader.close();
            inputStream.close();
        } catch (IOException e)
        {
            e.printStackTrace();
        }

        JSONArray jsonArray = null;
        List<Map<String, String>> adapterData = new ArrayList<>();
        try
        {
            jsonArray = new JSONArray(stringBuilder.toString());
            String packageName = getPackageName();
            for (int i = 0; i < jsonArray.length(); i++)
            {
                JSONObject jsonObject = jsonArray.getJSONObject(i);
                String name = jsonObject.getString("name");
                int translatedTextId = getResources().getIdentifier(name, "string", packageName);
                String cmd = jsonObject.getString("cmd");

                Map<String, String> item = new HashMap<>();
                item.put("name", (translatedTextId == 0) ? name : getString(translatedTextId));
                item.put("cmd", cmd);

                adapterData.add(item);
            }
        } catch (JSONException e)
        {
            throw new RuntimeException(e);
        }

        String[] from = {"name", "cmd"};
        int[] to = {android.R.id.text1, android.R.id.text2};

        SimpleAdapter adapter = new SimpleAdapter(this, adapterData, android.R.layout.simple_list_item_2, from, to);
        commandListView.setAdapter(adapter);
        commandListView.setOnItemClickListener((parent, view, position, id) ->
        {
            Map<String, String> button = (Map<String, String>) parent.getItemAtPosition(position);
            String cmdHex = button.get("cmd");
            byte[] cmd = hexToBytes(cmdHex);
            byte[] cmdWithHead = new byte[cmd.length + 2];
            cmdWithHead[0] = -86;
            // should not be more than 127
            cmdWithHead[1] = (byte) cmd.length;
            System.arraycopy(cmd, 0, cmdWithHead, 2, cmd.length);
            byte[] fullCmd = addCRC(cmdWithHead);
            Log.i("DATA", bytesToHex(fullCmd));
            sendData(fullCmd);
        });


    }

    public static String bytesToHex(byte[] bytes)
    {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes)
        {
            sb.append(String.format("%02X", b));
        }
        return sb.toString();
    }

    public static byte[] hexToBytes(String hexString)
    {
        int length = hexString.length();
        byte[] data = new byte[length / 2];
        for (int i = 0; i < length; i += 2)
        {
            data[i / 2] = (byte) ((Character.digit(hexString.charAt(i), 16) << 4)
                    + Character.digit(hexString.charAt(i + 1), 16));
        }
        return data;
    }

    public byte[] addCRC(byte[] bArr)
    {
        int i;
        byte b;
        i = 8217;
        for (byte b2 : bArr)
        {
            i += b2 & 255;
        }
        int length = bArr.length + 2;
        byte[] bArr2 = new byte[length];
        System.arraycopy(bArr, 0, bArr2, 0, bArr.length);
        bArr2[length - 2] = (byte) (i >> 8);
        bArr2[length - 1] = (byte) (i & 255);
        CRCCheck(bArr2);
        Log.i("CRC", "CRC_value:" + bytesToHex(bArr2) + ",length:" + i + ",data:" + bytesToHex(bArr));
        return bArr2;
    }

    public static boolean CRCCheck(byte[] bArr)
    {
        int i = (bArr[bArr.length - 1] & 255) + ((bArr[bArr.length - 2] << 8) & 65280);
        int i2 = 0;
        for (int i3 = 0; i3 < bArr.length - 1; i3++)
        {
            if (i3 < bArr.length - 2)
            {
                i2 += bArr[i3] & 255;
            }
        }
        return i2 + 8217 == i;
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 0)
        {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
            {
            }
            else
            {
            }
        }
    }

    private void connectToDevice()
    {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 0);
            return;
        }
        Set<BluetoothDevice> bondedDevices = mBluetoothAdapter.getBondedDevices();
        BluetoothSocket socket = null;
        boolean isEDIFIERDevice = false;
        for (BluetoothDevice device : bondedDevices)
        {
            try
            {
                isEDIFIERDevice = false;
                Log.i(TAG, device.getAddress());
                for (ParcelUuid uuid : device.getUuids())
                {
                    if (uuid.getUuid().equals(SPP_UUID))
                    {
                        isEDIFIERDevice = true;
                        break;
                    }
                }
                if (!isEDIFIERDevice)
                    continue;

                socket = device.createRfcommSocketToServiceRecord(SPP_UUID);
                socket.connect();
                break;
            } catch (IOException e)
            {
                e.printStackTrace();
            }
        }
        if (!isEDIFIERDevice)
        {
            Toast.makeText(getApplicationContext(), R.string.toast_device_not_found, Toast.LENGTH_SHORT).show();
        }
        if (socket.isConnected())
        {
            mBluetoothSocket = socket;
            try
            {
                mOutputStream = mBluetoothSocket.getOutputStream();
            } catch (IOException e)
            {
                e.printStackTrace();
            }
            mConnected = true;
            Log.i(TAG, "Connected");
            Toast.makeText(getApplicationContext(), R.string.toast_device_connected, Toast.LENGTH_SHORT).show();
        }
    }

    private void sendData(byte[] data)
    {
        if (!mConnected)
        {
            Toast.makeText(getApplicationContext(), R.string.toast_device_not_connected, Toast.LENGTH_SHORT).show();
            return;
        }
        try
        {
            mOutputStream.write(data);
        } catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}