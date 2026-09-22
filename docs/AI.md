# Optional AI authoring

AI is optional and is used only as an authoring assistant.

```bash
bitweave ai-author-prompt examples/ai-brief.txt > request.txt
```

The request asks a local model to return a **new complete `.bwx` program**. That source is then built normally:

```bash
bitweave build-linux-pie generated.bwx generated
```

The AI model is not linked into BitWeave, is not part of the generated executable, and is not required for deterministic builds.
