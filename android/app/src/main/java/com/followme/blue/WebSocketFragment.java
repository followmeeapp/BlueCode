package com.followme.blue;

import android.content.Context;
import android.net.Uri;
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
 * {@link WebSocketFragment.OnFragmentInteractionListener} interface
 * to handle interaction events.
 */
public class WebSocketFragment extends Fragment {
    private OnFragmentInteractionListener mListener;
    private EditText timeStampValueField;
    private TextView resultsField;


    public WebSocketFragment() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_web_socket, container, false);

        timeStampValueField = (EditText) rootView.findViewById(R.id.timeStampValue);
        resultsField = (TextView) rootView.findViewById(R.id.resultView);
        Button wsButton = (Button) rootView.findViewById(R.id.wsButton);
        wsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // call the MainActivity.onWebSocketAction -> delegates to background service
                // Async, so result comes back through intent broadcast
                Integer value = Integer.parseInt(timeStampValueField.getText().toString());
                mListener.onWebSocketAction(1, value);
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
        // send CardInfo over a websocket to the wsserver
        void onWebSocketAction(int id, int timeStamp);
    }
}
