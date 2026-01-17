package io.jor.ottomatic;

import org.libsdl.app.SDLActivity;

/**
 * Otto Matic Android Activity
 * Extends SDLActivity to leverage SDL3's Android integration
 */
public class OttoMaticActivity extends SDLActivity {
    
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "OttoMatic"
        };
    }
    
    @Override
    protected String getMainFunction() {
        return "main";
    }
}
