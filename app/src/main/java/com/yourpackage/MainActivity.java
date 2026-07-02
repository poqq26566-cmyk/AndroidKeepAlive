package com.yourpackage.keepalive;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TextView tv = new TextView(this);
        tv.setText("保活已启动！");
        setContentView(tv);
        
        // 启动保活
        KeepAlive.start(this);
    }
}
