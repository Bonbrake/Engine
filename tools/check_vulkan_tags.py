import urllib.request, json

url = "https://api.github.com/repos/KhronosGroup/Vulkan-Docs/tags?per_page=5"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print("Latest Vulkan Tags:")
        for r in data:
            print(f"- {r.get('name')}")
except Exception as e:
    print("Error:", e)
