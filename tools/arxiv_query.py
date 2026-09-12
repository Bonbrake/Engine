import urllib.request, json, urllib.parse

# Search arXiv API for DeepSeek-R1 reasoning bypass / skip thinking
q = "DeepSeek-R1 reasoning"
url = "http://export.arxiv.org/api/query?search_query=all:" + urllib.parse.quote(q) + "&max_results=5"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req) as resp:
        xml = resp.read().decode("utf-8")
        import xml.etree.ElementTree as ET
        root = ET.fromstring(xml)
        for entry in root.findall("{http://www.w3.org/2005/Atom}entry"):
            title = entry.find("{http://www.w3.org/2005/Atom}title").text.strip().replace("\n", " ")
            summary = entry.find("{http://www.w3.org/2005/Atom}summary").text.strip().replace("\n", " ")[:200]
            print(f"Title: {title}")
            print(f"Summary: {summary}...\n")
except Exception as e:
    print("Error:", e)
