import urllib.request, urllib.parse, json

# Let's test Hugging Face model hub API or DuckDuckGo API or Qwant or Mojeek
# Or search via Google CSE / Tavily / Brave if key exists, or DuckDuckGo API
req = urllib.request.Request("https://api.duckduckgo.com/?q=deepseek+r1+skip+thinking&format=json", headers={"User-Agent": "curl/7.68.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Abstract:", data.get("Abstract"))
        print("Related:", len(data.get("RelatedTopics", [])))
except Exception as e:
    print(e)
