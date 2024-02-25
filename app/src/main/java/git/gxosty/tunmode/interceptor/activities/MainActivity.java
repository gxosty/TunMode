package git.gxosty.tunmode.interceptor.activities;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;

import android.widget.Toast;
import android.widget.Button;
import android.view.View;

import android.os.Bundle;

import git.gxosty.tunmode.R;
import git.gxosty.tunmode.interceptor.services.TunModeService;

public class MainActivity extends AppCompatActivity implements TunModeService.EventListener {

    private Button toggle_button;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        this.toggle_button = this.findViewById(R.id.tun_switch_button);

        this.toggle_button.setOnClickListener((View v) -> {
            TunModeService.State state = TunModeService.getState();
            if (state.equals(TunModeService.State.CONNECTED)) {
                MainActivity.this.startTunMode(TunModeService.Operation.DISCONNECT);
            } else if (state.equals(TunModeService.State.DISCONNECTED)) {
                MainActivity.this.startTunMode(TunModeService.Operation.CONNECT);
            } else {
                Toast.makeText(this, "Try Again", Toast.LENGTH_SHORT).show();
            }
            
        });

        TunModeService.setEventListener(this);
        TunModeService.setActivity(this);

        this.startTunMode(TunModeService.Operation.INITIALIZE);
    }

    @Override
    protected void onDestroy() {
        TunModeService.setEventListener(null);
        super.onDestroy();
    }

    @Override
    public void onEvent(TunModeService.Event event) {
        runOnUiThread(() -> {
            switch (event) {
            case CONNECTING:
                MainActivity.this.toggle_button.setText("Connecting...");
                break;

            case CONNECTED:
                MainActivity.this.toggle_button.setText("Connected");
                break;

            case DISCONNECTING:
                MainActivity.this.toggle_button.setText("Disconnecting...");
                break;

            case DISCONNECTED:
                MainActivity.this.toggle_button.setText("Disconnected");
                break;

            case NETWORK_ERROR:
                MainActivity.this.toggle_button.setText("Disconnected");
                Toast.makeText(this, "Network Error", Toast.LENGTH_SHORT).show();
                break;

            default:
                break;
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == 1337) {
            switch (resultCode) {
            case RESULT_OK:
                break;
            case RESULT_CANCELED:
                Toast.makeText(this, "Access Denied", Toast.LENGTH_SHORT).show();
                break;
            }
        }
    }

    private void startTunMode(TunModeService.Operation operation) {
        Intent intent = new Intent(this, TunModeService.class);
        intent.putExtra(TunModeService.INTENT_EXTRA_OPERATION, operation);
        startService(intent);
    }
}