import urllib.request, json

# Search GitHub discussions / issues for "<think></think>" deepseek
url = "https://api.github.com/search/issues?q=%22%3Cthink%3E%5Cn%3C%2Fthink%3E%22+deepseek"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print(f"Total found: {data.get('total_count')}")
        for item in data.get("items", [])[:10]:
            print(f"- {item['title']}: {item['html_url']}")
except Exception as e:
    print("Error:", e)
