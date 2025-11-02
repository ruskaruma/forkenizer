# forkenizer

Forkenizer is a focused C++20 tokenizer that converts text, numbers, and mathematical expressions into deterministic tokens. It supports byte-level BPE-style vocab, efficient longest-match trie encoding, deterministic decoding, and a small CLI for encode/decode/inspect. No image/audio handling. Minimal docs, production-minded, and built for integration with LLM systems.

## Quick Start

```bash
# buildd the project
mkdir -p build && cd build
cmake -DBUILD_TESTS=OFF ..
make forkenizer-cli

# simple file tokenization
./build/forkenizer-cli document.txt                    #for creatingdocument_tokenized.txt
./build/forkenizer-cli -m data_examples file.txt      #specify model directory

# decode tokens
./build/forkenizer-cli decode tokens.txt              #this creates tokens_decoded.txt

# train model
./build/forkenizer-cli train corpus.txt               #creates model in ./model

# tnspect vocabulary
./build/forkenizer-cli inspect                         # Shows vocabulary stats
```

The CLI automatically detects model files (`vocab.json` and `merges.txt`) in the current directory or specified model path.

Model training for production-scale corpora is out-of-scope; use trainer scaffold to extend.

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.
