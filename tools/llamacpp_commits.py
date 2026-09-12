import urllib.request, json

# Query llama.cpp releases or commit logs for deepseek r1 or thinking
url = "https://api.github.com/repos/ggerganov/llama.cpp/commits?path=examples/server&per_page=10"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        for c in data:
            msg = c['commit']['message'].split('\n')[0]
            print(f"- {c['sha'][:7]}: {msg}")
except Exception as e:
    print("Error:", e)
