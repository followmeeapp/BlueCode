package com.followme.blue;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;


/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link MenuFragment.OnFragmentInteractionListener} interface
 * to handle interaction events.
 * Use the {@link MenuFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class MenuFragment extends Fragment {

    private MenuAdapter menuAdapter;

    public class MenuAdapter extends BaseAdapter {
        List<Map<String, String>> menuItems = new ArrayList<>();

        public void addMenuItem(String title, String description) {
            Map<String, String> item = new HashMap<>();
            item.put("title", title);
            item.put("desc", description);
            menuItems.add(item);
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return menuItems.size();
        }

        @Override
        public Map<String, String> getItem(int whichItem) {
            return menuItems.get(whichItem);
        }

        @Override
        public long getItemId(int whichItem) {
            return whichItem;
        }

        @Override
        public View getView(int whichItem, View view, ViewGroup viewGroup)   {
            if(view == null) {
                LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                view = inflater.inflate(R.layout.menuitem, viewGroup,false);
            }

            TextView menuTitle = (TextView) view.findViewById(R.id.menuLabel);
            TextView menuDescription = (TextView) view.findViewById(R.id.menuDescription);

            menuTitle.setText(menuItems.get(whichItem).get("title"));
            menuDescription.setText(menuItems.get(whichItem).get("desc"));

            return view;
        }

    }
    private OnFragmentInteractionListener mListener;

    public MenuFragment() {
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
        View rootView = inflater.inflate(R.layout.fragment_menu, container, false);
        if (container == null) {
            return null;
        }

        menuAdapter = new MenuAdapter();

        ListView menuView = (ListView) rootView.findViewById(R.id.menu);

        menuView.setAdapter(menuAdapter);

        menuAdapter.addMenuItem("ImageCropFragment", "Demonstration of image cropping and image selection");
        menuAdapter.addMenuItem("C++ integration", "Demonstration of calling C++ code");
        menuAdapter.addMenuItem("Websocket", "Demonstration of websocket communication.");

        menuView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapter, View view, int whichItem, long id) {
                if (mListener != null) {
                    mListener.onMenuItemSelect(menuAdapter.getItem(whichItem).get("title"));
                }
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
        // TODO: Update argument type and name
        public void onMenuItemSelect(String itemName);
    }
}
