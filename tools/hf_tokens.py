import urllib.request, json

url = "https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B/raw/main/tokenizer_config.json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("added_tokens_decoder:")
        tokens = data.get("added_tokens_decoder", {})
        # print first 20 tokens
        for k in sorted(tokens.keys(), key=lambda x: int(x))[:25]:
            print(f"  {k}: {tokens[k].get('content')}")
        for k in sorted(tokens.keys(), key=lambda x: int(x))[-15:]:
            print(f"  {k}: {tokens[k].get('content')}")
except Exception as e:
    print("Error:", e)
