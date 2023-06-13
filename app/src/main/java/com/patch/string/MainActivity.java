package com.patch.string;

import android.Manifest;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;


import com.patch.filepicker.controller.DialogSelectionListener;
import com.patch.filepicker.model.DialogConfigs;
import com.patch.filepicker.model.DialogProperties;
import com.patch.filepicker.view.FilePickerDialog;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class MainActivity extends AppCompatActivity {
    static int spc_count = -1;
    private static Context context;
    //private TextView tvFile;
    private ProgressDialog aa;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        context = this;
        //tvFile = findViewById(R.id.txt_file);

        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 101);
        }
    }

    public void pickFolder(View view) {
        String title = getString(R.string.folder_title);
        DialogProperties properties = new DialogProperties(true);
        properties.setSelectionType(DialogConfigs.DIR_SELECT);
        FilePickerDialog dialog = new FilePickerDialog(MainActivity.this, properties);
        dialog.setTitle(title);
        dialog.setDialogSelectionListener(new DialogSelectionListener() {
            @RequiresApi(api = Build.VERSION_CODES.KITKAT)
            @Override
            public void onSelectedFilePaths(String[] files) {
                String aFile = files[0];
                try {
                    SS(new File(aFile));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        dialog.show();
    }

    

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    private void SS(final File aFile) throws FileNotFoundException, IOException {

        spc_count++;
        String spcs = "";
        for (int i = 0; i < spc_count; i++)
            spcs += " ";
        if (aFile.isFile())

        
        {
            try {
                sssll(aFile);
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else if (aFile.isDirectory()) {
            System.out.println(spcs + "[DIR] " + aFile.getName());
            File[] listOfFiles = aFile.listFiles();
            
            if (listOfFiles != null) {
                try {
                    for (int i = 0; i < listOfFiles.length; i++)

                        SS(listOfFiles[i]);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                
            } else {
                System.out.println(spcs + " [ACCESS DENIED]");
            }
        }
        spc_count--;

    }


    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    private void sssll(final File aFile) throws IOException {
        
        if (!(aFile == null)){
            Toast.makeText(MainActivity.context, "Done obfuscate string " + aFile.getName(),
                    Toast.LENGTH_LONG).show();
        }
        StringBuilder sb = new StringBuilder();
        InputStreamReader read = null;
        try {
            read = new InputStreamReader(new FileInputStream(aFile), StandardCharsets.UTF_8);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        BufferedReader br = new BufferedReader(read);
        String str = "";
        while (true) {
            try {
                if (!((str = br.readLine()) != null)) break;
            } catch (IOException e) {
                e.printStackTrace();
            }
            Matcher m = Pattern.compile("\".*\"").matcher(str);
            if (m.find()) {

                String tmp = m.group(0);
                String sa = tmp.replaceAll("\"", "");

                String hh = FF(sa);
                String ccc = str.replaceAll(tmp, hh);
                sb.append(ccc).append("\n");


            } else {
                sb.append(str + "\n");
            }

        }

        try {
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            read.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(aFile);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        try {
            fos.write(sb.toString().getBytes(StandardCharsets.UTF_8));
         
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            fos.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        }

    Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 0:
                    aa = new ProgressDialog(MainActivity.this);
                    aa.setTitle("");
                    // aa.setCancelable(false);
                    aa.setMessage("Encryption string start...");
                    aa.show();
                    break;
                case 1:
                    aa.dismiss();
                    new AlertDialog.Builder(MainActivity.this)
                            //.setTitle("")

                            // .setCancelable(false)
                            .setMessage("Done!")
                            .setPositiveButton("Закрыть", new DialogInterface.OnClickListener() {

                                @Override
                                public void onClick(DialogInterface p1, int p2) {
                                }
                            }).show();

                    break;
            }
        }
    };

   

    public static String FF(String sa) {
        Random r = new Random(System.currentTimeMillis());
        byte[] b = sa.getBytes();
        int c = b.length;
        StringBuilder sb = new StringBuilder();

        sb.append("(new Object() {");
        sb.append("int t;");
        sb.append("public String toString() {");
        sb.append("byte[] buf = new byte[");
        sb.append(c);
        sb.append("];");

        for (int i = 0; i < c; ++i) {
            int t = r.nextInt();
            int f = r.nextInt(24) + 1;

            t = (t & ~(0xff << f)) | (b[i] << f);

            sb.append("t = ");
            sb.append(t);
            sb.append(";");
            sb.append("buf[");
            sb.append(i);
            sb.append("] = (byte) (t >>> ");
            sb.append(f);
            sb.append(");");
        }

        sb.append("return new String(buf);");
        sb.append("}}.toString())");

        return sb.toString();
    }


   


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (requestCode == 100) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                String permGranted = getString(R.string.perm_granted);
                Toast.makeText(this, permGranted, Toast.LENGTH_SHORT).show();
            } else {
                String permDenied = getString(R.string.perm_denied);
                Toast.makeText(this, permDenied, Toast.LENGTH_SHORT).show();
                finish();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }
}
