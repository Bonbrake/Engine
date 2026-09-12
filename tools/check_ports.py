import urllib.request, json

ports = [5119, 8642, 8765, 8315]
for p in ports:
    try:
        url = f"http://127.0.0.1:{p}/"
        req = urllib.request.Request(url, headers={"User-Agent": "test"})
        with urllib.request.urlopen(req, timeout=2) as resp:
            print(f"Port {p}: {resp.status}")
    except Exception as e:
        print(f"Port {p}: {e}")
