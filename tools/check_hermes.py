import urllib.request, json

url = "http://127.0.0.1:8642/v1/models"
req = urllib.request.Request(url, headers={"User-Agent": "test"})
try:
    with urllib.request.urlopen(req, timeout=3) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        print(json.dumps(data, indent=2))
except Exception as e:
    print("Error:", e)
