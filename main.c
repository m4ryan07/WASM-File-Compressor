#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

struct Node {
    char data;
    unsigned freq;
    struct Node *left, *right;
};

struct Node* newNode(char data, unsigned freq) {
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

void countFrequencies(char *filename, unsigned int *freqArray) {
    FILE *file = fopen(filename, "rb");
    if (!file) return;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        freqArray[(unsigned char)ch]++;
    }
    fclose(file);
}

struct MinHeap {
    unsigned size; unsigned capacity;
    struct Node** array;
};

struct MinHeap* createMinHeap(unsigned capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->size = 0; minHeap->capacity = capacity;
    minHeap->array = (struct Node**)malloc(minHeap->capacity * sizeof(struct Node*));
    return minHeap;
}

void swapNodes(struct Node** a, struct Node** b) { struct Node* t = *a; *a = *b; *b = t; }

void minHeapify(struct MinHeap* minHeap, int idx) {
    int smallest = idx, left = 2 * idx + 1, right = 2 * idx + 2;
    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq) smallest = left;
    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq) smallest = right;
    if (smallest != idx) { swapNodes(&minHeap->array[smallest], &minHeap->array[idx]); minHeapify(minHeap, smallest); }
}

struct Node* extractMin(struct MinHeap* minHeap) {
    struct Node* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size; minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(struct MinHeap* minHeap, struct Node* node) {
    ++minHeap->size; int i = minHeap->size - 1;
    while (i && node->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2]; i = (i - 1) / 2;
    }
    minHeap->array[i] = node;
}

struct Node* buildHuffmanTree(unsigned int *frequencies) {
    struct MinHeap* minHeap = createMinHeap(256);
    for (int i = 0; i < 256; ++i) if (frequencies[i] > 0) insertMinHeap(minHeap, newNode((char)i, frequencies[i]));
    if (minHeap->size == 0) return NULL;
    while (minHeap->size != 1) {
        struct Node *left = extractMin(minHeap), *right = extractMin(minHeap);
        struct Node *top = newNode('$', left->freq + right->freq);
        top->left = left; top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

char* codes[256];
void storeCodes(struct Node* root, char* str, int level) {
    if (root->left) { str[level] = '0'; storeCodes(root->left, str, level + 1); }
    if (root->right) { str[level] = '1'; storeCodes(root->right, str, level + 1); }
    if (!(root->left) && !(root->right)) { str[level] = '\0'; codes[(unsigned char)root->data] = strdup(str); }
}

void compressFile(char* inputName, char* outputName) {
    FILE *in = fopen(inputName, "rb"), *out = fopen(outputName, "wb");
    if(!in || !out) return;
    unsigned char buffer = 0; int bitCount = 0, ch;
    while ((ch = fgetc(in)) != EOF) {
        char* code = codes[(unsigned char)ch];
        for (int i = 0; code[i] != '\0'; i++) {
            buffer <<= 1; if (code[i] == '1') buffer |= 1;
            if (++bitCount == 8) { fputc(buffer, out); buffer = 0; bitCount = 0; }
        }
    }
    if (bitCount > 0) { buffer <<= (8 - bitCount); fputc(buffer, out); }
    fclose(in); fclose(out);
}

void decompressFile(struct Node* root, char* inputName, char* outputName) {
    FILE *in = fopen(inputName, "rb"), *out = fopen(outputName, "wb");
    if(!in || !out || !root) return;
    struct Node* curr = root; unsigned char buffer;
    while (fread(&buffer, 1, 1, in)) {
        for (int i = 7; i >= 0; i--) {
            curr = ((buffer >> i) & 1) ? curr->right : curr->left;
            if (!(curr->left) && !(curr->right)) { fputc(curr->data, out); curr = root; }
        }
    }
    fclose(in); fclose(out);
}

// --- WASM EXPORTS ---

EMSCRIPTEN_KEEPALIVE
int run_compression() {
    unsigned int frequencies[256] = {0};
    for(int i=0; i<256; i++) { if(codes[i]) { free(codes[i]); codes[i] = NULL; } }
    countFrequencies("input_data", frequencies);
    struct Node* root = buildHuffmanTree(frequencies);
    if(!root) return 0;
    char temp[256]; storeCodes(root, temp, 0);
    compressFile("input_data", "output.bin");
    return 1;
}

EMSCRIPTEN_KEEPALIVE
int run_decompression() {
    unsigned int frequencies[256] = {0};
    countFrequencies("tree_data", frequencies);
    struct Node* root = buildHuffmanTree(frequencies);
    if(!root) return 0;
    decompressFile(root, "input_data", "restored_file");
    return 1;
}

int main() { return 0; }