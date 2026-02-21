package io.jor.ottomatic;

import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputType;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.CountDownLatch;

/**
 * Otto Matic Android Activity
 * Extends SDLActivity to leverage SDL3's Android integration
 */
public class OttoMaticActivity extends SDLActivity {

    private static final String TAG = "OttoMatic";
    private static final int REQUEST_CODE_PICK_TER = 1001;
    private static final int REQUEST_CODE_PICK_TER_RSRC = 1002;

    // Level names matching terrainFiles[] in File.c
    private static final String[] LEVEL_NAMES = {
        "1 - Earth Farm",
        "2 - Blob World",
        "3 - Blob Boss",
        "4 - Apocalypse",
        "5 - Cloud",
        "6 - Jungle",
        "7 - Jungle Boss",
        "8 - Fire & Ice",
        "9 - Saucer",
        "10 - Brain Boss"
    };

    // Terrain file basenames matching terrainFiles[] in File.c
    private static final String[] TERRAIN_BASENAMES = {
        "EarthFarm",
        "BlobWorld",
        "BlobBoss",
        "Apocalypse",
        "Cloud",
        "Jungle",
        "JungleBoss",
        "FireIce",
        "Saucer",
        "BrainBoss"
    };

    // Latch and result for level select dialog
    private CountDownLatch mLevelSelectLatch;
    private volatile int mSelectedLevel = -1;

    // State for custom level file picking
    private CountDownLatch mFilePickLatch;
    private volatile Uri mPickedFileUri;
    private volatile int mPendingFilePickRequest;

    // Temporary storage for custom level selection
    private volatile int mCustomLevelNum = -1;
    private volatile Uri mCustomTerUri;
    private volatile Uri mCustomTerRsrcUri;

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "OttoMatic"
        };
    }

    @Override
    protected String getMainFunction() {
        return "SDL_main";
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_TER || requestCode == REQUEST_CODE_PICK_TER_RSRC) {
            if (resultCode == RESULT_OK && data != null) {
                mPickedFileUri = data.getData();
            } else {
                mPickedFileUri = null;
            }
            if (mFilePickLatch != null) {
                mFilePickLatch.countDown();
            }
        }
    }

    /**
     * Copy a file from a content URI to a destination path in internal storage.
     * Returns true on success.
     */
    private boolean copyUriToFile(Uri uri, String destPath) {
        try {
            InputStream in = getContentResolver().openInputStream(uri);
            if (in == null) return false;

            File dest = new File(destPath);
            File parent = dest.getParentFile();
            if (parent != null) parent.mkdirs();

            FileOutputStream out = new FileOutputStream(dest);
            byte[] buf = new byte[65536];
            int n;
            while ((n = in.read(buf)) >= 0) {
                out.write(buf, 0, n);
            }
            in.close();
            out.close();
            return true;
        } catch (IOException e) {
            Log.e(TAG, "copyUriToFile failed: " + e.getMessage());
            return false;
        }
    }

    /**
     * Launch a file picker and block until the user picks a file (or cancels).
     * Must be called from a background thread (not UI thread).
     * Returns the picked URI or null if cancelled.
     */
    private Uri pickFile(final int requestCode) {
        mFilePickLatch = new CountDownLatch(1);
        mPickedFileUri = null;
        mPendingFilePickRequest = requestCode;

        runOnUiThread(() -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            startActivityForResult(intent, requestCode);
        });

        try {
            mFilePickLatch.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return mPickedFileUri;
    }

    /**
     * Show the custom level dialog on the UI thread.
     * Returns the selected level number, or -1 if cancelled.
     * Must be called from a background thread.
     */
    private int showCustomLevelDialog() {
        final CountDownLatch latch = new CountDownLatch(1);
        final int[] result = {-1};

        runOnUiThread(() -> {
            LinearLayout layout = new LinearLayout(OttoMaticActivity.this);
            layout.setOrientation(LinearLayout.VERTICAL);
            int pad = (int)(16 * getResources().getDisplayMetrics().density);
            layout.setPadding(pad, pad, pad, pad);
            layout.setSpacing(pad / 2);

            TextView tvLevel = new TextView(OttoMaticActivity.this);
            tvLevel.setText("Level number (1-10):");
            layout.addView(tvLevel);

            final EditText etLevel = new EditText(OttoMaticActivity.this);
            etLevel.setInputType(InputType.TYPE_CLASS_NUMBER);
            etLevel.setHint("1");
            layout.addView(etLevel);

            TextView tvTer = new TextView(OttoMaticActivity.this);
            tvTer.setText(".ter file: (none selected)");
            layout.addView(tvTer);

            Button btnTer = new Button(OttoMaticActivity.this);
            btnTer.setText("Choose .ter file…");
            layout.addView(btnTer);

            TextView tvRsrc = new TextView(OttoMaticActivity.this);
            tvRsrc.setText(".ter.rsrc file: (none selected)");
            layout.addView(tvRsrc);

            Button btnRsrc = new Button(OttoMaticActivity.this);
            btnRsrc.setText("Choose .ter.rsrc file…");
            layout.addView(btnRsrc);

            // File picker buttons launch async pickers; we run them on a worker thread
            btnTer.setOnClickListener(v -> {
                new Thread(() -> {
                    Uri uri = pickFile(REQUEST_CODE_PICK_TER);
                    mCustomTerUri = uri;
                    runOnUiThread(() -> {
                        if (uri != null) {
                            tvTer.setText(".ter file: " + uri.getLastPathSegment());
                        } else {
                            tvTer.setText(".ter file: (none selected)");
                        }
                    });
                }).start();
            });

            btnRsrc.setOnClickListener(v -> {
                new Thread(() -> {
                    Uri uri = pickFile(REQUEST_CODE_PICK_TER_RSRC);
                    mCustomTerRsrcUri = uri;
                    runOnUiThread(() -> {
                        if (uri != null) {
                            tvRsrc.setText(".ter.rsrc file: " + uri.getLastPathSegment());
                        } else {
                            tvRsrc.setText(".ter.rsrc file: (none selected)");
                        }
                    });
                }).start();
            });

            ScrollView scroll = new ScrollView(OttoMaticActivity.this);
            scroll.addView(layout);

            new AlertDialog.Builder(OttoMaticActivity.this)
                .setTitle("Custom Level")
                .setView(scroll)
                .setPositiveButton("Load", (dialog, which) -> {
                    String levelStr = etLevel.getText().toString().trim();
                    int levelNum = -1;
                    try {
                        levelNum = Integer.parseInt(levelStr) - 1; // convert 1-based to 0-based
                    } catch (NumberFormatException ignored) {}

                    if (levelNum < 0 || levelNum >= 10) {
                        tvLevel.setText("Level number (1-10): INVALID - enter 1 to 10");
                        return;
                    }
                    if (mCustomTerUri == null || mCustomTerRsrcUri == null) {
                        tvTer.setText("Please select both .ter and .ter.rsrc files!");
                        return;
                    }

                    // Copy files to internal storage over the target level's terrain files
                    String internalPath = getFilesDir().getAbsolutePath();
                    String basename = TERRAIN_BASENAMES[levelNum];
                    String terDest = internalPath + "/Terrain/" + basename + ".ter";
                    String rsrcDest = internalPath + "/Terrain/" + basename + ".ter.rsrc";

                    boolean ok1 = copyUriToFile(mCustomTerUri, terDest);
                    boolean ok2 = copyUriToFile(mCustomTerRsrcUri, rsrcDest);

                    if (!ok1 || !ok2) {
                        tvTer.setText("Error copying files! Check storage permissions.");
                        return;
                    }

                    result[0] = levelNum;
                    latch.countDown();
                })
                .setNegativeButton("Cancel", (dialog, which) -> {
                    result[0] = -1;
                    latch.countDown();
                })
                .setOnCancelListener(dialog -> {
                    result[0] = -1;
                    latch.countDown();
                })
                .show();
        });

        try {
            latch.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return result[0];
    }

    /**
     * Show the level select dialog.
     * Called from native (C) code via JNI.
     * Returns the selected level number (0-based), or -1 if cancelled.
     * Blocks until the user makes a choice.
     */
    public int showLevelSelectDialog() {
        mLevelSelectLatch = new CountDownLatch(1);
        mSelectedLevel = -1;
        mCustomTerUri = null;
        mCustomTerRsrcUri = null;

        runOnUiThread(() -> {
            // Build items array: 10 level names + "Custom Level..."
            final CharSequence[] items = new CharSequence[LEVEL_NAMES.length + 1];
            for (int i = 0; i < LEVEL_NAMES.length; i++) {
                items[i] = "Level " + LEVEL_NAMES[i];
            }
            items[LEVEL_NAMES.length] = "Custom Level…";

            new AlertDialog.Builder(OttoMaticActivity.this)
                .setTitle("Select Level")
                .setItems(items, (dialog, which) -> {
                    if (which < LEVEL_NAMES.length) {
                        // Direct level selection
                        mSelectedLevel = which;
                        mLevelSelectLatch.countDown();
                    } else {
                        // Custom level - show sub-dialog on a background thread
                        new Thread(() -> {
                            int level = showCustomLevelDialog();
                            mSelectedLevel = level;
                            mLevelSelectLatch.countDown();
                        }).start();
                    }
                })
                .setNegativeButton("Cancel", (dialog, which) -> {
                    mSelectedLevel = -1;
                    mLevelSelectLatch.countDown();
                })
                .setOnCancelListener(dialog -> {
                    mSelectedLevel = -1;
                    mLevelSelectLatch.countDown();
                })
                .show();
        });

        try {
            mLevelSelectLatch.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return mSelectedLevel;
    }
}
