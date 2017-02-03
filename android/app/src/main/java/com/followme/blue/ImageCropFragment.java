package com.followme.blue;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.icu.text.SimpleDateFormat;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.FileProvider;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.yalantis.ucrop.UCrop;

import java.io.File;

import java.io.IOException;
import java.util.Date;

import static android.app.Activity.RESULT_OK;


/**
 * A simple {@link Fragment} subclass.
 */
public class ImageCropFragment extends Fragment {
    public ImageView croppedImage;

    //private OnFragmentInteractionListener mListener;

    public ImageCropFragment() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    private static final int WRITE_EXTERNAL_STORAGE = 1;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View rootView = inflater.inflate(R.layout.fragment_second, container, false);
        if (container == null) {
            return null;
        }

        croppedImage = (ImageView) rootView.findViewById(R.id.imageView);
        Button captureCrop = (Button) rootView.findViewById(R.id.captureCropButton);
        captureCrop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dispatchTakePictureIntent();
            }
        });

        Button galleryCrop = (Button) rootView.findViewById(R.id.galleryCropButton);
        galleryCrop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int permissionCheck = ContextCompat.checkSelfPermission(getContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE);

                if (permissionCheck != PackageManager.PERMISSION_GRANTED) {
                    requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, WRITE_EXTERNAL_STORAGE);
                } else {
                    dispatchSelectPictureIntent();
                }
            }
        });
        return rootView;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {

            case WRITE_EXTERNAL_STORAGE:
                if ((grantResults.length > 0) && (grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    dispatchSelectPictureIntent();
                }
                break;

            default:
                break;
        }
    }

    private static final int REQUEST_IMAGE_CAPTURE = 1;
    private static final int RESULT_LOAD_IMAGE = 2;

    String mCurrentPhotoPath;

    private void dispatchSelectPictureIntent() {
        Intent i = new Intent(
                Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);

        startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // Ensure that there's a camera activity to handle the intent
        if (takePictureIntent.resolveActivity(getContext().getPackageManager()) != null) {
            // Create the File where the photo should go
            File photoFile = null;
            try {
                photoFile = createImageFile();
            } catch (IOException ex) {
                // Error occurred while creating the File

            }
            // Continue only if the File was successfully created
            if (photoFile != null) {
                Uri photoURI = FileProvider.getUriForFile(getContext().getApplicationContext(),
                        "com.followme.blue.fileprovider",
                        photoFile);
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
            }
        }
    }

    private File createImageFile() throws IOException {
        // Create an image file name
        // TODO figure out how to get a API level save version of SimpleDateFormat!
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getContext().getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(imageFileName, ".jpg", storageDir);

        // Save a file: path for use with ACTION_VIEW intents
        mCurrentPhotoPath = image.getAbsolutePath();
        return image;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case REQUEST_IMAGE_CAPTURE:
                    File f = new File(mCurrentPhotoPath);
                    Uri contentUri = Uri.fromFile(f);
                    cropPicture(contentUri);
                    break;
                case UCrop.REQUEST_CROP:
                    Log.d("Generic", "succesful crop");

                    File dir = getContext().getFilesDir();
                    String dbPath = dir.getPath();

                    Uri croppedImageURI = Uri.fromFile(new File(dbPath, Constants.croppedFileName));
                    croppedImage.setImageDrawable(null);
                    croppedImage.setImageURI(croppedImageURI);

                    break;
                case RESULT_LOAD_IMAGE:

                    Uri selectedImage = data.getData();
                    String[] filePathColumn = { MediaStore.Images.Media.DATA };

                    Cursor cursor = getContext().getContentResolver().query(selectedImage,
                            filePathColumn, null, null, null);
                    cursor.moveToFirst();

                    int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
                    String picturePath = cursor.getString(columnIndex);
                    cursor.close();
                    cropPicture(selectedImage);
                    break;
            }
        }
        if (resultCode == UCrop.RESULT_ERROR) {
            Log.d("Generic", "error crop");
        }
    }

    private void cropPicture(Uri imageUri) {
        File dir = getContext().getFilesDir();
        String dbPath = dir.getPath();

        UCrop uCrop = UCrop.of(imageUri, Uri.fromFile(new File(dbPath, Constants.croppedFileName)));

        uCrop = uCrop.withAspectRatio(1, 1);
        UCrop.Options options = new UCrop.Options();
        options.setCompressionFormat(Bitmap.CompressFormat.JPEG);
        options.setDimmedLayerColor(Color.argb(200, 255, 255, 255));
        options.setShowCropFrame(false);
        options.setCircleDimmedLayer(true);
        options.setShowCropGrid(false);
        options.setCircleDimmedLayer(true);
        uCrop = uCrop.withOptions(options);
        uCrop.start(getContext(), ImageCropFragment.this);
    }

}
