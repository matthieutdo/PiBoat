package com.example.piboat;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity implements Runnable, OnSeekBarChangeListener {
	private Socket soc_serv;

	private final String SERVER_IP = "192.168.1.1";
	private final int SERVER_PORT = 4000;

	private int motor1_speed;
	private int motor2_speed;
	private int rudder_pos;

	private SeekBar motor1;
	private SeekBar motor2;
	private SeekBar motorAll;
	private SeekBar rudder;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    	
        motor1_speed = 0;
        motor2_speed = 0;
        rudder_pos = 90;

        motor1 = (SeekBar)findViewById(R.id.seekBarMotor1); // make seekbar object
        motor1.setOnSeekBarChangeListener(this); // set seekbar listener.
        motor2 = (SeekBar)findViewById(R.id.seekBarMotor2); // make seekbar object
        motor2.setOnSeekBarChangeListener(this); // set seekbar listener.
        motorAll = (SeekBar)findViewById(R.id.seekBarAllMotor); // make seekbar object
        motorAll.setOnSeekBarChangeListener(this); // set seekbar listener.
        rudder = (SeekBar)findViewById(R.id.seekBarRudder); // make seekbar object
        rudder.setOnSeekBarChangeListener(this); // set seekbar listener.
        
        new Thread(this).start();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    
    public void onClick_send(View view) {
    	try{
	    	EditText et = (EditText) findViewById(R.id.editText1);
	    	String str = et.getText().toString();
	    	
	    	soc_serv.getOutputStream().write(str.getBytes());
	    	Toast.makeText(MainActivity.this, "sended: "+str, Toast.LENGTH_SHORT).show();
    	} catch (UnknownHostException e) {
    		Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
        	Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
        	Toast.makeText(MainActivity.this, e.toString(), Toast.LENGTH_SHORT).show();
        }
    }
    
    
    public void onClick_disconnect(View view) {
    	try{
	    	soc_serv.getOutputStream().write("exit".getBytes());
	    	Toast.makeText(MainActivity.this, "Disconnected", Toast.LENGTH_SHORT).show();
    	} catch (UnknownHostException e) {
    		Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
        	Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
        	Toast.makeText(MainActivity.this, e.toString(), Toast.LENGTH_SHORT).show();
        }
    }
    
    
    /*class ClientThread implements Runnable {
        @Override
        public void run() {
            EditText et = (EditText) findViewById(R.id.editText1);
            et.setText("connection");
            
            try {
                InetAddress serverAddr = InetAddress.getByName(SERVER_IP);
                soc_serv = new Socket(serverAddr, SERVER_PORT);
                
                et.setText("connected");
                //Toast.makeText(MainActivity.this, "connected", Toast.LENGTH_SHORT).show();
                
                while(true) Thread.currentThread().wait(10);
            } catch (UnknownHostException e) {
            	Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
            } catch (IOException e) {
            	Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
            } catch (Exception e) {
            	Toast.makeText(MainActivity.this, e.toString(), Toast.LENGTH_SHORT).show();
            }
        }
    }*/


	@Override
	public void run() {
        EditText et = (EditText) findViewById(R.id.editText1);
        TextView tv = (TextView) findViewById(R.id.textView1);
        et.setText("connection");
        
        try {
            InetAddress serverAddr = InetAddress.getByName(SERVER_IP);
            soc_serv = new Socket(serverAddr, SERVER_PORT);
            
            et.setText("connected");
            
            //while(true) Thread.currentThread().wait(10);
        } catch (UnknownHostException e) {
        	tv.setText(e.getMessage());
        } catch (IOException e) {
        	tv.setText(e.getMessage());
        } catch (Exception e) {
        	tv.setText(e.getMessage());
        }
	}


	@Override
	public void onProgressChanged(SeekBar sb, int progress, boolean arg2) {
		String cmd;
        TextView tv = (TextView) findViewById(R.id.textView1);
		
		if (sb.getId() == motor1.getId()){ 
				motor1_speed = progress-100;
				if (motor1_speed > -2 && motor1_speed < 2) motor1_speed = 0;
				cmd = "m "+motor1_speed+" "+motor2_speed;
		}
		else if (sb.getId() == motor2.getId()){
				motor2_speed = progress-100;
				if (motor2_speed > -2 && motor2_speed < 2) motor2_speed = 0;
				cmd = "m "+motor1_speed+" "+motor2_speed;
		}
		else if (sb.getId() == motorAll.getId()){
				motor1_speed = progress-100;
				motor2_speed = progress-100;
				if (motor1_speed > -2 && motor1_speed < 2) motor1_speed = 0;
				if (motor2_speed > -2 && motor2_speed < 2) motor2_speed = 0;
				cmd = "m "+motor1_speed+" "+motor2_speed;
		}
		else if (sb.getId() == rudder.getId()){
				rudder_pos = progress+40;
				cmd = "g "+rudder_pos;
		}
		else{
				Toast.makeText(MainActivity.this, "seekBar not fund 0_o", Toast.LENGTH_SHORT).show();
				cmd = "";
		}
		
		// send message
    	try {
			soc_serv.getOutputStream().write(cmd.getBytes());
			
			tv.setText("Send: "+cmd);
			//Toast.makeText(MainActivity.this, "Send: "+cmd, Toast.LENGTH_SHORT).show();
		} catch (IOException e) {
    		Toast.makeText(MainActivity.this, e.getMessage(), Toast.LENGTH_SHORT).show();
		}
	}


	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub
		
	}


	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub
		
	}

}
