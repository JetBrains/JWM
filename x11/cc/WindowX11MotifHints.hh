typedef struct {
    unsigned long   flags;
    unsigned long   functions;
    unsigned long   decorations;
    long            inputMode;
    unsigned long   status;
} MotifHints;


enum {
    MOTIF_HINTS_DECORATIONS = (1L << 1),
};
