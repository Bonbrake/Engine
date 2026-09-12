import urllib.request, json, re

# Let's search for Mike Booth's paper and GDC talk details on L4D AI Director
url = "https://raw.githubusercontent.com/ValveSoftware/source-sdk-2013/master/sp/src/game/server/ai_speechfilter.cpp"
# Or search web / Valve papers
try:
    req = urllib.request.Request("https://developer.valvesoftware.com/wiki/Left_4_Dead_AI_Director", headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req, timeout=5) as resp:
        content = resp.read().decode("utf-8", errors="ignore")
        # Extract headings and text
        clean = re.sub(r'<[^>]+>', ' ', content)
        print("VDC Director Page Snippet:\n", clean[:1500])
except Exception as e:
    print("Error:", e)
