package me.edoren.engine;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "engine-d",
            "LightCasters"
        };
    }

}
