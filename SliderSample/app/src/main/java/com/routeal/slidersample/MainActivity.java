package com.routeal.slidersample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SnappingSeekBar bar = (SnappingSeekBar) findViewById(R.id.seekbar1);

        bar.setProgressToIndex(2);

        bar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SnappingSeekBar bar = (SnappingSeekBar) v;
                int index = bar.getSelectedItemIndex();
                Toast.makeText(getApplicationContext(), "Selected: " + index, Toast.LENGTH_LONG).show();
            }
        });

        bar.setOnItemSelectionListener(new SnappingSeekBar.OnItemSelectionListener() {
            public void onItemSelected(final int itemIndex, final String itemString) {
                Toast.makeText(getApplicationContext(), "Selected: " + itemIndex, Toast.LENGTH_LONG).show();
            }
        });
    }
}
