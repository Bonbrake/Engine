import urllib.request, json

url = "https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B/raw/main/tokenizer.json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0", "Range": "bytes=0-10000"})
try:
    with urllib.request.urlopen(req) as resp:
        content = resp.read().decode("utf-8", errors="ignore")
        print("Header content length:", len(content))
        # look for special tokens in the start
        if "<|im_start|>" in content:
            print("Found <|im_start|> in tokenizer.json!")
        else:
            print("Did NOT find <|im_start|> in first 10KB")
        if "<｜User｜>" in content:
            print("Found <｜User｜> in tokenizer.json!")
except Exception as e:
    print("Error:", e)
