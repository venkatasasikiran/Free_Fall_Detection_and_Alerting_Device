package com.example.myapp;

import android.app.Activity;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Environment;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.util.Log;
import android.widget.Toast;

import java.io.*;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MyService extends Service {
    private static final int DISCOVER_DURATION = 300;
    boolean firstCall=true;
    boolean firststop=true;
    private static final int REQUEST_BLU = 1;
    Thread t;
    boolean thread = true;
    BluetoothSocket socket;
    Activity activity = new Main2Activity();
    //use serial profile bluetooth stack
    protected static final UUID SERIAL_PROFILE_UUID = UUID.fromString(
            "00001101-0000-1000-8000-00805F9B34FB");
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Let it continue running until it is stopped.
     /*   Toast.makeText(this, "Service Started", Toast.LENGTH_LONG).show();
        String uri = "tel:7035055862";

        Intent callIntent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
        callIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(callIntent);*/
        try {

            BluetoothAdapter mBluetoothAdapter1 = BluetoothAdapter.getDefaultAdapter();


            final UUID applicationUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
            //Connect to HC06 Bluetooth device
            BluetoothDevice device=mBluetoothAdapter1.getRemoteDevice("20:15:07:27:87:53");
            socket=device.createRfcommSocketToServiceRecord(applicationUUID);
            socket.connect();

               // socket.close();
            t= new Thread(new Runnable() {
                public void run(){
                    while(thread) {

                        //Read the inputstream for "call" string
    InputStream input = null;

                        try {
                            input = socket.getInputStream();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }


                        DataInputStream dinput = new DataInputStream(input);

    byte byteArray[] = new byte[100];

                        try {
                            dinput.readFully(byteArray);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                       final String s = new String(byteArray);

                        //If input string contains "call" make a phone call to the number entered below
                        //else ignore
if(s.contains("call")&&firstCall) {
firstCall=false;
//    Toast.makeText(MyService.this, "Service Started", Toast.LENGTH_LONG).show();
    String uri = "tel:7039441864";

    Intent callIntent = new Intent(Intent.ACTION_CALL, Uri.parse(uri));
    callIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    startActivity(callIntent);

}
    else if((s.contains("stop")&&firststop))
{
    firststop= false;
    firstCall=true;

}


}
                }
            });
            t.start();

        } catch (Exception e) {
            e.printStackTrace();
        }

        return START_STICKY;
    }


    //destroy the service when stopapp button is pressed
    @Override
    public void onDestroy() {
        super.onDestroy();
        thread=false;
        try {
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Toast.makeText(this, "Service Destroyed", Toast.LENGTH_LONG).show();
    }
}