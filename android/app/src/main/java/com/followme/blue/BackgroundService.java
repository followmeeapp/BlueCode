package com.followme.blue;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Binder;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import com.koushikdutta.async.AsyncServer;
import com.koushikdutta.async.ByteBufferList;
import com.koushikdutta.async.http.AsyncHttpClient;
import com.koushikdutta.async.http.WebSocket;
import com.koushikdutta.async.http.WebSocket.StringCallback;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.capnproto.MessageReader;

/**
 * Created by erik on 03/01/2017.
 */

public class BackgroundService extends Service {
    AsyncHttpClient client;
    AsyncServer server = new AsyncServer();

    private String dbPath;
    private LMDBJava database;
    // define single backend thread executor
    Executor mExec = Executors.newSingleThreadExecutor();

    // Instance Binder given to clients
    private final IBinder mBinder = new LocalBinder();

    public class LocalBinder extends Binder {
        BackgroundService getService() {
            // Return this instance of LocalService
            // so clients can call public methods
            return BackgroundService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("BGService", "Creating service");
        File dir = getFilesDir();
        dbPath = dir.getPath();
        database = new LMDBJava();
        database.open(dbPath);

        client = new AsyncHttpClient(server);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("BGService", "destroying service");
        database.close();
    }

    void echoCard(final Integer id, final Integer timeStamp) {
        mExec.execute(new Runnable() {
            public void run() {
                Log.d("Generic", "Running From Thread " +
                        Thread.currentThread().getId());

                final org.capnproto.MessageBuilder message =
                        new org.capnproto.MessageBuilder();

                final Cardinfo.CardInfo.Builder cardInfo = message.initRoot(Cardinfo.CardInfo.factory);

                cardInfo.setId(id);
                cardInfo.setTimestamp(timeStamp);

                ByteBuffer buf = ByteBuffer.allocate((int)org.capnproto.Serialize.computeSerializedSizeInWords(message)*8);
                org.capnproto.ArrayOutputStream stream = new org.capnproto.ArrayOutputStream(buf);
                try {
                    org.capnproto.Serialize.write(stream, message);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                byte[] jbuffer = stream.buf.array();
                CapnpEcho echoer = new CapnpEcho();
                byte[] retBuffer = echoer.echoCard(jbuffer);
                MessageReader reader;
                try {
                    reader = org.capnproto.Serialize.read(ByteBuffer.wrap(retBuffer));
                    Cardinfo.CardInfo.Reader returnCard = reader.getRoot(Cardinfo.CardInfo.factory);
                    Log.d("Generic", "return card id: " + returnCard.getId());

                } catch (IOException e) {
                    e.printStackTrace();
                }

                Intent localIntent =
                        new Intent(Constants.BROADCAST_CARD)
                                .putExtra(Constants.CARD_BUFFER, retBuffer);

                LocalBroadcastManager.getInstance(BackgroundService.this).sendBroadcast(localIntent);
            }
        });
    }

    void getLMDBString(final Integer id, final String value) {
        // methods get executed on the background executor
        mExec.execute(new Runnable() {
            public void run() {

                    Log.d("Generic", "Running From Thread " +
                            Thread.currentThread().getId());

                    long dbi = database.openDB();
                    int res = database.put(dbi, id, value);
                    String result = database.get(dbi, id);

                    database.closeDB(dbi);

                    Intent localIntent =
                            new Intent(Constants.BROADCAST_LMDB)
                                    .putExtra(Constants.LMDB_STRING, result);

                    LocalBroadcastManager.getInstance(BackgroundService.this).sendBroadcast(localIntent);
                }
        });
    }

    void sendWebSocketData(final String ip, final Integer id, final Integer timeStamp) {
        mExec.execute(new Runnable() {
            public void run() {

                final org.capnproto.MessageBuilder message =
                        new org.capnproto.MessageBuilder();

                final Cardinfo.CardInfo.Builder cardInfo = message.initRoot(Cardinfo.CardInfo.factory);

                cardInfo.setId(id);
                cardInfo.setTimestamp(timeStamp);

                // The ip address needs to be a real ip adress and not 'localhost'!
                AsyncHttpClient.getDefaultInstance().websocket("ws://" + ip + ":3000", null, new AsyncHttpClient.WebSocketConnectCallback() {
                    @Override
                    public void onCompleted(Exception ex, WebSocket webSocket) {
                        ByteBuffer buf = ByteBuffer.allocate((int) org.capnproto.Serialize.computeSerializedSizeInWords(message) * 8);
                        org.capnproto.ArrayOutputStream stream = new org.capnproto.ArrayOutputStream(buf);
                        try {
                            org.capnproto.Serialize.write(stream, message);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        webSocket.send(stream.buf.array());
                        webSocket.setStringCallback(new StringCallback() {
                            @Override
                            public void onStringAvailable(String s) {
                                Log.d("Generic", "got String!");
                                Intent localIntent =
                                        new Intent(Constants.BROADCAST_WS)
                                                .putExtra(Constants.WS_STRING, s);

                                LocalBroadcastManager.getInstance(BackgroundService.this).sendBroadcast(localIntent);
                            }
                        });
                    }
                });
            }
        });
    }
}
