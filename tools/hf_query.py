import urllib.request, json

url = "https://huggingface.co/api/models/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B/discussions"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        for d in data.get("discussions", [])[:15]:
            print(f"#{d['num']}: {d['title']} ({d['status']})")
except Exception as e:
    print("Error:", e)
