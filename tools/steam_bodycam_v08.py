import urllib.request, json

url = "https://api.steampowered.com/ISteamNews/GetNewsForApp/v0002/?appid=2406770&count=20&maxlength=3000&format=json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        items = data.get("appnews", {}).get("newsitems", [])
        for item in items:
            title = item.get("title", "")
            if "0.8" in title or "MAJOR" in title.upper() or "UPDATE" in title.upper() or "ZOMBIE" in title.upper():
                print(f"TITLE: {title}")
                print(f"CONTENTS:\n{item.get('contents')[:1000]}\n{'='*50}\n")
except Exception as e:
    print("Error:", e)
