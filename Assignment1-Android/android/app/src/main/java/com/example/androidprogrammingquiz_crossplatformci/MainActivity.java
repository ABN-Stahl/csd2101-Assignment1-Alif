package com.example.androidprogrammingquiz_crossplatformci;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.example.androidprogrammingquiz_crossplatformci.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'androidprogrammingquiz_crossplatformci' library on application startup.
    static {
        System.loadLibrary("androidprogrammingquiz_crossplatformci");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }

    /**
     * A native method that is implemented by the 'androidprogrammingquiz_crossplatformci' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}