import urllib.request, json

url = "https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B/raw/main/tokenizer_config.json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("chat_template:", data.get("chat_template"))
        print("eos_token:", data.get("eos_token"))
        print("bos_token:", data.get("bos_token"))
except Exception as e:
    print("Error:", e)
