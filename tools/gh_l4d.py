import urllib.request, json

# Search GitHub for Source SDK L4D director references or notes
url = "https://api.github.com/search/code?q=filename:nav_mesh.h+path:game/server+ValveSoftware"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Matches:", data.get("total_count"))
except Exception as e:
    print("Error:", e)
