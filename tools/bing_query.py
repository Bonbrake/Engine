import urllib.request, urllib.parse, re, html

def search_bing(q):
    url = "https://www.bing.com/search?q=" + urllib.parse.quote(q)
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"})
    try:
        with urllib.request.urlopen(req) as resp:
            content = resp.read().decode("utf-8", errors="ignore")
            matches = re.findall(r'<li class="b_algo">.*?<h2><a href="([^"]+)".*?>(.*?)</a></h2>.*?<div class="b_caption"><p>(.*?)</p>', content, re.DOTALL)
            for m in matches[:5]:
                url, title, snippet = m
                clean_t = re.sub(r'<[^>]+>', '', title)
                clean_s = re.sub(r'<[^>]+>', '', snippet)
                print(f"TITLE: {clean_t}\nURL: {url}\nSNIPPET: {clean_s}\n")
    except Exception as e:
        print("Error:", e)

search_bing("deepseek r1 disable thinking bypass think tag")
