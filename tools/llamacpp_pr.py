import urllib.request, json

url = "https://api.github.com/repos/ggerganov/llama.cpp/pulls/13174"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Title:", data.get("title"))
        print("Body:", (data.get("body") or "")[:800])
except Exception as e:
    print("Error:", e)
