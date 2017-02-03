package com.followme.blue;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link CppFragment.OnFragmentInteractionListener} interface
 * to handle interaction events.
 */
public class CppFragment extends Fragment {

    private OnFragmentInteractionListener mListener;

    private EditText lmdbValueField;
    private TextView lmdbReturnField;
    private EditText cardTimeStampField;

    public CppFragment() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment

        View rootView = inflater.inflate(R.layout.fragment_cpp, container, false);

        lmdbValueField = (EditText) rootView.findViewById(R.id.lmdbValue);
        cardTimeStampField = (EditText) rootView.findViewById(R.id.editTimeStamp);
        lmdbReturnField = (TextView) rootView.findViewById(R.id.lmdbResultView);
        Button backgroundButton = (Button) rootView.findViewById(R.id.lmdbBGStoreButton);
        backgroundButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // call the MainActivity.onStoreLMDBValue -> delegates to background service
                // Async, so result comes back through intent broadcast
                mListener.onStoreLMDBValue(1, lmdbValueField.getText().toString());
            }
        });
        Button uiButton = (Button) rootView.findViewById(R.id.lmdbStoreButton);
        uiButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // call the MainActivity.onUIStoreLMDBValue -> executes on UI Thread
                // sync so can update UI right away
                String result = mListener.onUIStoreLMDBValue(1, lmdbValueField.getText().toString());
                result = "from UI thread: " + result;
                lmdbReturnField.setText(result);
            }
        });
        Button cardEchoButton = (Button) rootView.findViewById(R.id.cardEchoButton);
        cardEchoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Integer timeStamp = Integer.parseInt(cardTimeStampField.getText().toString());
                mListener.onCardEcho(1, timeStamp);
            }
        });

        return rootView;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof OnFragmentInteractionListener) {
            mListener = (OnFragmentInteractionListener) context;
        } else {
            throw new RuntimeException(context.toString()
                    + " must implement OnFragmentInteractionListener");
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
    }

    /**
     * This interface must be implemented by activities that contain this
     * fragment to allow an interaction in this fragment to be communicated
     * to the activity and potentially other fragments contained in that
     * activity.
     * <p>
     * See the Android Training lesson <a href=
     * "http://developer.android.com/training/basics/fragments/communicating.html"
     * >Communicating with Other Fragments</a> for more information.
     */
    public interface OnFragmentInteractionListener {
        // store and read lmdb value on the background service thread
        void onStoreLMDBValue(int key, String value);
        // store and read lmdb value on the UI thread
        String onUIStoreLMDBValue(int key, String value);
        // call card echo on the background service
        void onCardEcho(int id, int timeStamp);
    }
}
