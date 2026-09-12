import urllib.request, json

# Let's search Hugging Face models for "dynamic reasoning" or "hybrid reasoning" or "think mode" or "Qwen2.5"
def search_hf_models(query, limit=10):
    url = f"https://huggingface.co/api/models?search={urllib.parse.quote(query)}&limit={limit}&full=false"
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    try:
        with urllib.request.urlopen(req) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            for m in data:
                print(f"- {m.get('id')} (downloads: {m.get('downloads', 0)}, likes: {m.get('likes', 0)})")
    except Exception as e:
        print("Error:", e)

print("=== Search: reasoning on demand ===")
search_hf_models("reasoning on demand", 5)

print("\n=== Search: switchable reasoning ===")
search_hf_models("switchable reasoning", 5)

print("\n=== Search: Qwen2.5 1.5B ===")
search_hf_models("Qwen2.5-1.5B", 5)
