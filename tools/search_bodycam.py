import urllib.request, json, re

# Search for Bodycam game updates (Reissad Studio)
q = "Bodycam game update Reissad Studio patch notes zombie"
url = "https://api.duckduckgo.com/?q=Bodycam+game+update+Reissad&format=json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Abstract:", data.get("AbstractText"))
except Exception as e:
    print("Error:", e)
