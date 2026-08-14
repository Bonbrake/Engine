# Fact-Check: M13 SLM / Appendix K (Modding) / Appendix L / Appendix M / Part A
Scope: real-world constants only (no code/design edits). Sources: HuggingFace MiniCPM5-1B model card + config.json + GGUF repo.

## MiniCPM5-1B hard facts (doc §M13, lines 8586–8588, 8591)
| CLAIM (doc) | VERDICT | CORRECT VALUE | SOURCE |
|---|---|---|---|
| Total params 1,080,632,832 | CORRECT | same | huggingface.co/openbmb/MiniCPM5-1B |
| Non-embed params 679,552,512 | CORRECT | same | ibid |
| Arch: 24 layers, 16 Q heads, 2 KV heads, head_dim 128, hidden 1536, vocab 130560, ctx 131072 | CORRECT | exact (config.json) | huggingface.co/openbmb/MiniCPM5-1B/raw/main/config.json |
| Standard LlamaForCausalLM, no custom kernels | CORRECT | "standard LlamaForCausalLM … no custom kernels" | ibid (model card) |
| Q4_K_M GGUF = 688 MB | CORRECT | 688 MB | huggingface.co/openbmb/MiniCPM5-1B-GGUF |
| Apache 2.0 license | CORRECT | Apache-2.0 | ibid |
| Sampling: Think 0.9/0.95, No-Think 0.7/0.95, enable_thinking flag | CORRECT | exact table match | ibid (model card) |
| Q8_0 1.15 GB, F16 2.17 GB | N/A-in-doc | real sizes, doc only cites Q4 | huggingface.co/openbmb/MiniCPM5-1B-GGUF |

## Items the task expected but the doc does NOT claim
- "quant 2–4GB GGUF": doc never says this. Real GGUF sizes = 688 MB (Q4_K_M) → 1.15 GB (Q8_0) → 2.17 GB (F16). The doc's 688 MB Q4 figure is correct; the "2–4GB" framing is the task's, not the doc's.
- "llama.cpp speed on 8GB GPU" / "on-device LLM latency": doc gives NO benchmark tok/s or ms latency number. Only a design throttle: 1–2 tokens/frame, off the critical path (line 8594). That is engineering guidance, not a measured claim — not wrong, just absent.
- Async job throughput: no explicit jobs/sec figure. M0-EXT-14 async-compute-queue pattern (line 10751) is a valid Vulkan design; no false numeric claim.

## Modding (Appendix K [K-EXT-13] + L, lines 10482–10731)
- JSON data-table hot-reload, schema-validated loudly at boot/change, layered mod stack: accurate design.
- Lua scripting: doc explicitly DEFERS Lua ("out of scope for this pass", line 10731) — no false Lua claim. Correctly scoped to JSON data + (future) sandboxed VM.

## Verdict
Every verifiable real-world constant in M13 and the modding appendices matches the official OpenBMB sources exactly. VRAM math (688 MB + KV + ~100–150 MB ≈ 800–850 MB) is arithmetically sound and fits the 6–8 GB Tier-0 budget. No corrections required to the doc's SLM / modding / async claims — they are design statements, and the few hard constants (params, arch, GGUF size, license, sampling) all verify. The only mismatch is the task's own "2–4GB GGUF" expectation; the doc is right at 688 MB Q4_K_M.
