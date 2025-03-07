#ifndef SWATCHES_H
#define SWATCHES_H

// Define color arrays

struct pairedSwatch {
    const char* primary;
    const char* secondary;
};

const pairedSwatch mainSwatch[] = {
    {"rgb(255,255,0)",      "rgb(128,128,0)"},
    {"rgb(255,215,0)",      "rgb(128,108,0)"},
    {"rgb(255,165,0)",      "rgb(128,83,0)"},
    {"rgb(255,140,0)",      "rgb(128,70,0)"},
    {"rgb(210,105,30)",     "rgb(105,53,15)"},
    {"rgb(255,69,0)",       "rgb(128,35,0)"},
    {"rgb(255,0,255)",      "rgb(128,0,128)"},
    {"rgb(153,50,204)",     "rgb(77,25,102)"},
    {"rgb(138,43,226)",     "rgb(69,22,113)"},
    {"rgb(75,0,130)",       "rgb(38,0,65)"},
    {"rgb(0,0,97)",         "rgb(0,0,49)"},
};

const char* const Anodize[] = {
    "rgb(255,255,0)",
    "rgb(255,215,0)",
    "rgb(255,165,0)",
    "rgb(255,140,0)",
    "rgb(210,105,30)",
    "rgb(255,69,0)",
    "rgb(255,0,255)",
    "rgb(153,50,204)",
    "rgb(138,43,226)",
    "rgb(75,0,130)",
    "rgb(0, 0, 97)",
    NULL  // End marker
};

const char* const BisexualLighting[] = {
    "rgb(255,192,203)",
    "rgb(255,105,180)",
    "rgb(255,20,147)",
    "rgb(199,21,133)",
    "rgb(128,0,128)",
    "rgb(75,0,130)",
    "rgb(72,61,139)",
    "rgb(0,206,209)",
    "rgb(72,209,204)",
    "rgb(135,206,250)",
    "rgb(224,255,255)",
    "rgb(0,255,255)",
    "rgb(0,139,139)",
    NULL  // End marker
};

const char* const bootswatch[] = {
    "rgb(0,0,0)",
    "rgb(255, 220, 106)",
    "rgb(209, 0, 52)",
    "rgb(75,0,130)",
    "rgb(0, 0, 97)",
    NULL  // End marker
};

#endif // SWATCHES_H
