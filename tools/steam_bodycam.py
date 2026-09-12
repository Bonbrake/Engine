import urllib.request, json

url = "https://api.steampowered.com/ISteamNews/GetNewsForApp/v0002/?appid=2406770&count=5&maxlength=1500&format=json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        items = data.get("appnews", {}).get("newsitems", [])
        for item in items:
            print(f"TITLE: {item.get('title')}")
            print(f"DATE: {item.get('date')}")
            print(f"CONTENTS:\n{item.get('contents')[:600]}\n{'-'*50}\n")
except Exception as e:
    print("Error:", e)
