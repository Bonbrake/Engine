import urllib.request, json

url = "https://api.github.com/repos/ollama/ollama/issues/11010"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("TITLE:", data.get("title"))
        print("BODY:", (data.get("body") or "")[:1000])
except Exception as e:
    print("Error:", e)
