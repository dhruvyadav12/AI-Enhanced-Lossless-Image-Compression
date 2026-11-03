#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;


// ==================================================
// LEFT PREDICTOR
// ==================================================

int leftPredictor(
    const vector<unsigned char>& pixels,
    int index
) {
    if (index == 0) {
        return 0;
    }

    return pixels[index - 1];
}


// ==================================================
// AVERAGE PREDICTOR
// ==================================================

int averagePredictor(
    const vector<unsigned char>& pixels,
    int index,
    int width
) {
    if (index == 0) {
        return 0;
    }

    int left = pixels[index - 1];

    if (index < width) {
        return left;
    }

    int top = pixels[index - width];

    return (left + top) / 2;
}


// ==================================================
// PAETH PREDICTOR
// ==================================================

int paethPredictor(
    const vector<unsigned char>& pixels,
    int index,
    int width
) {
    if (index == 0) {
        return 0;
    }

    int left = pixels[index - 1];

    if (index < width) {
        return left;
    }

    int top = pixels[index - width];

    int topLeft = pixels[index - width - 1];

    int p = left + top - topLeft;

    int pa = abs(p - left);
    int pb = abs(p - top);
    int pc = abs(p - topLeft);

    if (pa <= pb && pa <= pc) {
        return left;
    }

    if (pb <= pc) {
        return top;
    }

    return topLeft;
}


// ==================================================
// ANALYZE PREDICTOR
// ==================================================

void analyzePredictor(
    const vector<unsigned char>& pixels,
    int width,
    int (*predictor)(
        const vector<unsigned char>&,
        int,
        int
    ),
    const string& name
) {

    long long sumAbsResidual = 0;

    long long zeroResiduals = 0;

    int maxResidual = 0;


    for (int i = 0; i < pixels.size(); i++) {

        int prediction =
            predictor(
                pixels,
                i,
                width
            );

        int residual =
            static_cast<int>(pixels[i])
            - prediction;

        sumAbsResidual += abs(residual);

        if (residual == 0) {
            zeroResiduals++;
        }

        maxResidual =
            max(
                maxResidual,
                abs(residual)
            );
    }


    double averageAbsResidual =
        static_cast<double>(
            sumAbsResidual
        )
        /
        pixels.size();


    double zeroPercentage =
        100.0 *
        static_cast<double>(
            zeroResiduals
        )
        /
        pixels.size();


    cout << endl;

    cout << "========== "
         << name
         << " =========="
         << endl;

    cout << "Average absolute residual: "
         << averageAbsResidual
         << endl;

    cout << "Zero residuals: "
         << zeroResiduals
         << endl;

    cout << "Zero residual percentage: "
         << zeroPercentage
         << "%"
         << endl;

    cout << "Maximum absolute residual: "
         << maxResidual
         << endl;
}


// ==================================================
// MAIN
// ==================================================

int main() {

    string filePath =
        "../../data/input/image.raw";


    ifstream inputFile(
        filePath,
        ios::binary
    );


    if (!inputFile) {

        cout << "ERROR: Could not open image.raw"
             << endl;

        return 1;
    }


    vector<unsigned char> pixels(
        (istreambuf_iterator<char>(inputFile)),
        istreambuf_iterator<char>()
    );


    inputFile.close();


    // Your image dimensions
    int width = 1086;
    int height = 1448;


    cout << "Image loaded." << endl;

    cout << "Width: "
         << width
         << endl;

    cout << "Height: "
         << height
         << endl;

    cout << "Pixels: "
         << pixels.size()
         << endl;


    // --------------------------------------------------
    // LEFT
    // --------------------------------------------------

    analyzePredictor(
        pixels,
        width,
        [](const vector<unsigned char>& p, int i, int w) {
            if (i == 0)
                return 0;

            return static_cast<int>(p[i - 1]);
        },
        "LEFT PREDICTOR"
    );


    // --------------------------------------------------
    // AVERAGE
    // --------------------------------------------------

    analyzePredictor(
        pixels,
        width,
        averagePredictor,
        "AVERAGE PREDICTOR"
    );


    // --------------------------------------------------
    // PAETH
    // --------------------------------------------------

    analyzePredictor(
        pixels,
        width,
        paethPredictor,
        "PAETH PREDICTOR"
    );


    return 0;
}