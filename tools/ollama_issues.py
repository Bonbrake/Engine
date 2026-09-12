import urllib.request, json

url = "https://api.github.com/search/issues?q=repo:ollama/ollama+deepseek+r1+think"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        for item in data.get("items", [])[:8]:
            print(f"- {item['title']} (state: {item['state']}) -> {item['html_url']}")
except Exception as e:
    print("Error:", e)
