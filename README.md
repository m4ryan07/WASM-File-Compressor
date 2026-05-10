 Huffman Engine: C-WASM Codec

A high-performance, lossless file compression utility. This project takes a core engine written in C and cross-compiles it to WebAssembly (WASM) to run natively in the browser.

 Features
- Client-Side Processing: Files never leave your computer.
- WASM Speed: Near-native execution of Huffman coding logic.
- Universal Codec: Capable of handling any binary file (PDFs, Images, Text).

  Technical Breakdown
- Algorithm: Huffman Coding using a Min-Heap for tree construction.
- Bridge: Uses Emscripten's Virtual File System to pass data between JS and C.
- UI: Modern "Dark Mode" dashboard with real-time status updates.

  Project Structure
- `main.c`: Core compression/decompression logic.
- `compressor.js/wasm`: The compiled WASM module.
- `index.html`: The professional user interface.
