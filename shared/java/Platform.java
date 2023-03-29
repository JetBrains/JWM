package io.github.humbleui.jwm;

public enum Platform {
    WINDOWS,
    MACOS,
    X11,
    WAYLAND;

    public static Platform CURRENT;
    static {
        String os = System.getProperty("os.name").toLowerCase();        
        if (os.contains("mac") || os.contains("darwin"))
            CURRENT = MACOS;
        else if (os.contains("windows"))
            CURRENT = WINDOWS;
        else if (os.contains("nux") || os.contains("nix"))
            CURRENT = X11;
        else
            throw new RuntimeException("Unsupported platform: " + os);
    }

    // Dynamically update the platform name.
    // As systems can support both X11 and Wayland,
    // we suppport identifying the platform at runtime here.
    public static void update() {
        // X11 is Linux by default, so we fall back to it.
        if (Platform.CURRENT == Platform.X11) {
            String sessionType = System.getenv("XDG_SESSION_TYPE");
             if (sessionType.equals("wayland"))
                     CURRENT = WAYLAND;
        }
    }
}
