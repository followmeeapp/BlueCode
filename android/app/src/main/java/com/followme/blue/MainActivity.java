package com.followme.blue;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
//import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.support.v4.content.LocalBroadcastManager;
import android.content.Context;
import android.util.Log;
import android.widget.TextView;

import org.capnproto.MessageReader;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

import io.branch.referral.Branch;
import io.branch.referral.BranchError;

public class MainActivity extends AppCompatActivity implements MenuFragment.OnFragmentInteractionListener, CppFragment.OnFragmentInteractionListener, WebSocketFragment.OnFragmentInteractionListener {

    BackgroundService mService;
    boolean mBound = false;
    private LMDBJava lmdb;
    private long dbi;

    // Defines callbacks for service binding,
    // passed to bindService()
    private ServiceConnection mConnection = new ServiceConnection()
    {
        @Override
        public void onServiceConnected(ComponentName name,
                                       IBinder service) {

            // We've bound to LocalService,
            // cast the IBinder and get LocalService instance
            BackgroundService.LocalBinder binder =
                    (BackgroundService.LocalBinder) service;
            mService = binder.getService();
            mBound = true;

        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
            mService = null;
        }
    };

    // Broadcast receiver for receiving status updates from the IntentService
    private class ResponseReceiver extends BroadcastReceiver
    {
        // Prevents instantiation
        private ResponseReceiver() {
        }
        // Called when the BroadcastReceiver gets an Intent it's registered to receive
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
                case Constants.BROADCAST_CARD:
                    byte[] retBuffer = intent.getByteArrayExtra(Constants.CARD_BUFFER);
                    MessageReader reader;
                    try {
                        reader = org.capnproto.Serialize.read(ByteBuffer.wrap(retBuffer));
                        Cardinfo.CardInfo.Reader returnCard = reader.getRoot(Cardinfo.CardInfo.factory);
                        Log.d("Generic", "return from intent card id: " + returnCard.getId());

                        String result = "Echoed Card: timeStamp: " + returnCard.getTimestamp();
                        Fragment curFragment = getSupportFragmentManager().findFragmentById(R.id.fragment_container);
                        if (curFragment != null) {
                            TextView resultField = (TextView)curFragment.getView().findViewById(R.id.lmdbResultView);
                            if (resultField != null) {
                                resultField.setText(result);
                            }
                        }

                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    break;
                case Constants.BROADCAST_LMDB:
                    String lmdbResult = "from background service: " + intent.getStringExtra(Constants.LMDB_STRING);
                    Log.d("Generic", "return from lmdb: " + lmdbResult);
                    { // isolate curFragment
                        Fragment curFragment = getSupportFragmentManager().findFragmentById(R.id.fragment_container);
                        if (curFragment != null) {
                            TextView resultField = (TextView) curFragment.getView().findViewById(R.id.lmdbResultView);
                            if (resultField != null) {
                                resultField.setText(lmdbResult);
                            }
                        }
                    }
                    break;
                case Constants.BROADCAST_WS:
                    String wsResult = intent.getStringExtra(Constants.WS_STRING);
                    Log.d("Generic", "return from ws: " + wsResult);
                    { // isolate curFragment
                        Fragment curFragment = getSupportFragmentManager().findFragmentById(R.id.fragment_container);
                        if (curFragment != null) {
                            TextView resultField = (TextView) curFragment.getView().findViewById(R.id.resultView);
                            if (resultField != null) {
                                resultField.setText(wsResult);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        File dir = getFilesDir();
        lmdb = new LMDBJava();
        String dbPath = dir.getPath();
        lmdb.open(dbPath);

        dbi = lmdb.openDB();

        // Example of a call to a native method

        // setup intents (and filters) for messages from the background service
        IntentFilter lmdbIntentFilter = new IntentFilter(
                Constants.BROADCAST_LMDB);
        IntentFilter cardIntentFilter = new IntentFilter(
                Constants.BROADCAST_CARD);
        IntentFilter wsIntentFilter = new IntentFilter(
                Constants.BROADCAST_WS);
        ResponseReceiver BackgroundServiceReceiver =
                new ResponseReceiver();
        LocalBroadcastManager bcManager = LocalBroadcastManager.getInstance(this);
        bcManager.registerReceiver(
                BackgroundServiceReceiver,
                cardIntentFilter);
        bcManager.registerReceiver(
                BackgroundServiceReceiver,
                lmdbIntentFilter);
        bcManager.registerReceiver(
                BackgroundServiceReceiver,
                wsIntentFilter);

        // Check that the activity is using the layout version with
        // the fragment_container FrameLayout
        if (findViewById(R.id.fragment_container) != null) {

            // However, if we're being restored from a previous state,
            // then we don't need to do anything and should return or else
            // we could end up with overlapping fragments.
            if (savedInstanceState != null) {
                return;
            }

            // Create a new Fragment to be placed in the activity layout
            MenuFragment firstFragment = new MenuFragment();

            // In case this activity was started with special instructions from an
            // Intent, pass the Intent's extras to the fragment as arguments
            firstFragment.setArguments(getIntent().getExtras());

            // Add the fragment to the 'fragment_container' FrameLayout
            getSupportFragmentManager().beginTransaction()
                    .add(R.id.fragment_container, firstFragment).commit();
        }

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        lmdb.closeDB(dbi);
        lmdb.close();
    }

    @Override
    protected void onStart() {
        super.onStart();
        // Bind to LocalService
        Intent intent = new Intent(this, BackgroundService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        Branch branch = Branch.getInstance();

        branch.initSession(new Branch.BranchReferralInitListener(){
            @Override
            public void onInitFinished(JSONObject referringParams, BranchError error) {
                if (error == null) {
                    // params are the deep linked params associated with the link that the user clicked -> was re-directed to this app
                    // params will be empty if no data found
                    // ... insert custom logic here ...
                    Log.i("Blue", "BranchIO init finished");

                } else {
                    Log.i("Blue", error.getMessage());
                }
            }
        }, this.getIntent().getData(), this);
    }

    @Override
    public void onNewIntent(Intent intent) {
        this.setIntent(intent);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    public void onMenuItemSelect(String itemName) {
        Fragment newFragment;
        if (itemName.equalsIgnoreCase("ImageCropFragment")) {
            newFragment = new ImageCropFragment();
        } else if (itemName.equalsIgnoreCase("C++ integration")) {
            newFragment = new CppFragment();
        } else if (itemName.equalsIgnoreCase("Websocket")) {
            newFragment = new WebSocketFragment();
        } else {
            return;
        }
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.fragment_container, newFragment);
        transaction.addToBackStack(null);

        transaction.commit();
    }

    @Override
    public void onStoreLMDBValue(int key, String value) {
        mService.getLMDBString(key, value);
    }

    @Override
    public String onUIStoreLMDBValue(int key, String value) {
        int res = lmdb.put(dbi, key, value);
        if (res != 0) {
            Log.d("Generic", "error storing value");
            return "Error storing value";
        }

        return lmdb.get(dbi, key);
    }

    @Override
    public void onCardEcho(int id, int timeStamp) {
        mService.echoCard(id, timeStamp);
    }

    @Override
    public void onWebSocketAction(int id, int timeStamp) {
        mService.sendWebSocketData("192.168.1.29", id, timeStamp);
    }
}
