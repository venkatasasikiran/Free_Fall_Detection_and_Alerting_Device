package com.example.myapp;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.Button;

public class Main2Activity extends Activity {

    Button button,button1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);
        button = (Button) findViewById(R.id.button);
        button1 = (Button) findViewById(R.id.button1);
        button1.setVisibility(View.INVISIBLE);
        View.OnClickListener oclBtnOk = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                button1.setVisibility(View.VISIBLE);

                button.setVisibility(View.INVISIBLE);

                startService(new Intent(getBaseContext(), MyService.class));

            }
        };
        View.OnClickListener oclBtnCancel = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub

                button.setVisibility(View.VISIBLE);

                button1.setVisibility(View.INVISIBLE);

                stopService(new Intent(getBaseContext(), MyService.class));

            }
        };


        button.setOnClickListener(oclBtnOk);
        button1.setOnClickListener(oclBtnCancel);
    }


}
