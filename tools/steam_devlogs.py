import urllib.request, json

url = "https://api.steampowered.com/ISteamNews/GetNewsForApp/v0002/?appid=2406770&count=20&maxlength=4000&format=json"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        for item in data.get("appnews", {}).get("newsitems", []):
            title = item.get("title", "")
            if "Sound Design" in title or "Animation" in title:
                print(f"=== {title} ===")
                print(item.get("contents"))
                print("\n")
except Exception as e:
    print("Error:", e)
