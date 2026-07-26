import sys
sys.path.append('C:\\ZombieEngine\\scripts')
from fetch_research import fetch_arxiv_abstract
import re

plan = open(r'C:\ZombieEngine\recon\plans\FINAL_COMPREHENSIVE_PLAN.md', encoding='utf-8').read()
arxiv_ids = re.findall(r'arXiv:([0-9]+\.[0-9]+)', plan)

results = {}
for aid in arxiv_ids:
    try:
        html = fetch_arxiv_abstract(aid)
        match = re.search(r'<blockquote class="abstract mathjax">\s*<span class="descriptor">Abstract:</span>(.+?)</blockquote>', html, re.DOTALL)
        if match:
            results[aid] = match.group(1).strip()
        else:
            results[aid] = 'Abstract not found'
    except Exception as e:
        results[aid] = f'Error: {e}'

print(f'Fetched {len(results)} abstracts:')
for aid, abstract in results.items():
    print(f'\n--- arXiv:{aid} ---')
    print(abstract[:300].replace('\n', ' ') + '...')
