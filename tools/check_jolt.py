import urllib.request, json

url = "https://api.github.com/repos/jrouwe/JoltPhysics/releases?per_page=5"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Latest Jolt Physics Releases:")
        for r in data:
            print(f"- {r.get('tag_name')} ({r.get('published_at')})")
except Exception as e:
    print("Error:", e)
