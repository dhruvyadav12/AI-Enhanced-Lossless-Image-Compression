#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <fstream>

using namespace std;
int predict(const vector<int>& image, int index) {

    if (index == 0) {
        return 0;
    }

    return image[index - 1];
}

vector<int> decode(const vector<int>& residuals) {

    vector<int> reconstructed;

    for (int i = 0; i < residuals.size(); i++) {

        int prediction;

        if (i == 0) {
            prediction = 0;
        } else {
            prediction = reconstructed[i - 1];
        }

        int pixel = prediction + residuals[i];

        reconstructed.push_back(pixel);
    }

    return reconstructed;
}

int zigzagEncode(int value) {

    if (value >= 0) {
        return 2 * value;
    } else {
        return -2 * value - 1;
    }
}

struct HuffmanNode {

    int value;
    int frequency;

    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int value, int frequency) {

        this->value = value;
        this->frequency = frequency;

        left = nullptr;
        right = nullptr;
    }
};


struct Compare {

    bool operator()(HuffmanNode* a, HuffmanNode* b) {

        return a->frequency > b->frequency;
    }
};

void generateCodes(
    HuffmanNode* node,
    string code,
    map<int, string>& codes
) {

    // If this is a leaf node, store its code
    if (node->left == nullptr && node->right == nullptr) {

        codes[node->value] = code;

        return;
    }

    // Go left = 0
    if (node->left != nullptr) {
        generateCodes(node->left, code + "0", codes);
    }

    // Go right = 1
    if (node->right != nullptr) {
        generateCodes(node->right, code + "1", codes);
    }
}


void printBytes(const string& bits) {

    cout << "Bit groups:" << endl;

    for (int i = 0; i < bits.size(); i += 8) {

        cout << bits.substr(i, 8) << endl;
    }
}


vector<unsigned char> packBits(
    const string& bits,
    int& validBits
) {

    validBits = bits.size();

    vector<unsigned char> bytes;

    for (int i = 0; i < bits.size(); i += 8) {

        unsigned char byte = 0;

        for (int j = 0; j < 8; j++) {

            byte <<= 1;

            if (i + j < bits.size() &&
                bits[i + j] == '1') {

                byte |= 1;
            }
        }

        bytes.push_back(byte);
    }

    return bytes;
}


int main() {

    // Our sample grayscale image
    vector<int> image = {
        100, 102, 105, 108,
        101, 103, 106, 109,
        102, 104, 107, 110
    };

    cout << "Image pixels:" << endl;

    for (int pixel : image) {
        cout << pixel << " ";
    }

    cout << endl;

   vector<int> residuals;

for (int i = 0; i < image.size(); i++) {

    int prediction = predict(image, i);

    int residual = image[i] - prediction;

    residuals.push_back(residual);
}

cout << "Residuals:" << endl;

for (int residual : residuals) {
    cout << residual << " ";
}

cout << endl;

cout << "ZigZag encoded:" << endl;

for (int residual : residuals) {

    int encoded = zigzagEncode(residual);

    cout << encoded << " ";
}

cout << endl;

map<int, int> frequency;

for (int residual : residuals) {

    int encoded = zigzagEncode(residual);

    frequency[encoded]++;
}

cout << "Frequency table:" << endl;

for (auto entry : frequency) {

    cout << entry.first
         << " -> "
         << entry.second
         << endl;
}

priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;

for (auto entry : frequency) {

    HuffmanNode* node =
        new HuffmanNode(entry.first, entry.second);

    pq.push(node);
}

cout << "Smallest frequency: "
     << pq.top()->frequency
     << endl;

     HuffmanNode* left = pq.top();
pq.pop();

HuffmanNode* right = pq.top();
pq.pop();

HuffmanNode* parent =
    new HuffmanNode(-1, left->frequency + right->frequency);

parent->left = left;
parent->right = right;

pq.push(parent);

cout << "New parent frequency: "
     << parent->frequency
     << endl;

 HuffmanNode* left2 = pq.top();
pq.pop();

HuffmanNode* right2 = pq.top();
pq.pop();

HuffmanNode* parent2 =
    new HuffmanNode(-1, left2->frequency + right2->frequency);

parent2->left = left2;
parent2->right = right2;

pq.push(parent2);

cout << "Second parent frequency: "
     << parent2->frequency
     << endl;    


HuffmanNode* left3 = pq.top();
pq.pop();

HuffmanNode* right3 = pq.top();
pq.pop();

HuffmanNode* root =
    new HuffmanNode(-1, left3->frequency + right3->frequency);

root->left = left3;
root->right = right3;

pq.push(root);

cout << "Root frequency: "
     << root->frequency
     << endl;

     map<int, string> codes;

generateCodes(root, "", codes);

cout << "Huffman codes:" << endl;

for (auto entry : codes) {

    cout << entry.first
         << " -> "
         << entry.second
         << endl;
}

string encodedBits = "";

for (int residual : residuals) {

    int value = zigzagEncode(residual);

    encodedBits += codes[value];
}

cout << "Encoded bitstream:" << endl;
cout << encodedBits << endl;

printBytes(encodedBits);

int validBits;

vector<unsigned char> compressedBytes =
    packBits(encodedBits, validBits);

cout << "Number of bytes: "
     << compressedBytes.size()
     << endl;

cout << "Valid bits: "
     << validBits
     << endl;

     ofstream outputFile("test.aic", ios::binary);

outputFile.write(
    reinterpret_cast<const char*>(compressedBytes.data()),
    compressedBytes.size()
);

outputFile.close();

cout << "Compressed file created: test.aic" << endl;

vector<int> reconstructed = decode(residuals);

cout << "Reconstructed image:" << endl;

for (int pixel : reconstructed) {
    cout << pixel << " ";
}

cout << endl;

if (image == reconstructed) {
    cout << "LOSSLESS TEST: PASSED" << endl;
} else {
    cout << "LOSSLESS TEST: FAILED" << endl;
}

    return 0;
}