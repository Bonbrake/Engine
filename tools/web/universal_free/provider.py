"""Universal free web provider - NO API key.

Search  : GitHub API + SearXNG public instances + DDGS fallback.
Extract : keyless urllib HTML->markdown.

All tools work with zero credentials.
"""

from __future__ import annotations

import concurrent.futures as _cf
import html
import json
import logging
import re
import ssl
import urllib.parse
import urllib.request
from typing import Any, Dict, List

from agent.web_search_provider import WebSearchProvider

logger = logging.getLogger(__name__)

_SEARCH_TIMEOUT_SECS = 30
_EXTRACT_TIMEOUT_SECS = 30
_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) HermesAgent/1.0"
_SSL = ssl.create_default_context()
_SSL.check_hostname = False
_SSL.verify_mode = ssl.CERT_NONE

# SearXNG public instances (no key, community-run, may be flaky but improves results)
_SEARXNG_INSTANCES = [
    "https://searx.be",
    "https://searx.space",
    "https://search.newt.gg",
]


def _github_search(query: str, limit: int) -> List[Dict[str, Any]]:
    """Search GitHub repos via public API (no key required)."""
    results: List[Dict[str, Any]] = []
    q = urllib.parse.quote_plus(query + " in:name,description,readme")
    url = f"https://api.github.com/search/repositories?per_page={limit}&q={q}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": _USER_AGENT, "Accept": "application/json"})
        with urllib.request.urlopen(req, timeout=_SEARCH_TIMEOUT_SECS, context=_SSL) as resp:
            charset = resp.headers.get_content_charset() or "utf-8"
            data = json.loads(resp.read().decode(charset))
        for i, item in enumerate(data.get("items", [])[:limit]):
            results.append(
                {"title": item.get("full_name", ""), "url": item.get("html_url", ""), "description": item.get("description", "") or "", "position": i + 1}
            )
    except Exception as e:
        logger.debug("GitHub search failed: %s", e)
    return results


def _searxng_search(query: str, limit: int) -> List[Dict[str, Any]]:
    """SearXNG search via public instances (JSON API, no key)."""
    for inst in _SEARXNG_INSTANCES:
        try:
            url = f"{inst}/search?q={urllib.parse.quote_plus(query)}&format=json"
            req = urllib.request.Request(url, headers={"User-Agent": _USER_AGENT, "Accept": "application/json"})
            with urllib.request.urlopen(req, timeout=10, context=_SSL) as resp:
                data = json.loads(resp.read())
            results = []
            for i, item in enumerate(data.get("results", [])[:limit]):
                results.append({"title": item.get("title", ""), "url": item.get("url", ""), "description": item.get("content", "") or item.get("snippet", ""), "position": i + 1})
            if results:
                return results
        except Exception:
            continue
    return []


def _ddgs_search(query: str, limit: int) -> List[Dict[str, Any]]:
    """DuckDuckGo search via ddgs package (fallback)."""
    out: List[Dict[str, Any]] = []
    try:
        from ddgs import DDGS

        with DDGS(timeout=10) as client:
            for i, hit in enumerate(client.text(query, max_results=limit)):
                if i >= limit:
                    break
                out.append({"title": str(hit.get("title", "")), "url": str(hit.get("href") or hit.get("url") or ""), "description": str(hit.get("body", "")), "position": i + 1})
    except Exception as e:
        logger.debug("DDGS search failed: %s", e)
    return out


_TAG_RE = re.compile(r"<[^>]+>")


def _html_to_markdown(raw: str, url: str) -> str:
    text = html.unescape(raw)
    text = re.sub(r"(?is)<(script|style|noscript|svg|head)[^>]*>.*?</\1>", " ", text)
    text = re.sub(r"(?is)<h([1-6])[^>]*>", lambda m: "\n\n" + "#" * int(m.group(1)) + " ", text)
    text = re.sub(r"(?is)</h[1-6]>", "\n", text)
    text = re.sub(r"(?is)<(p|div|li|tr|br|/p|/div|/li|/tr)[^>]*>", "\n", text)
    text = _TAG_RE.sub("", text)
    lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
    return f"# Extracted: {url}\n\n" + "\n".join(lines) + "\n"


def _fetch_extract(url: str) -> Dict[str, Any]:
    result: Dict[str, Any] = {"url": url, "title": "", "content": "", "error": ""}
    try:
        req = urllib.request.Request(url, headers={"User-Agent": _USER_AGENT})
        with urllib.request.urlopen(req, timeout=_EXTRACT_TIMEOUT_SECS, context=_SSL) as resp:
            charset = resp.headers.get_content_charset() or "utf-8"
            raw = resp.read().decode(charset, errors="ignore")
        m = re.search(r"(?is)<title[^>]*>(.*?)</title>", raw)
        if m:
            result["title"] = html.unescape(m.group(1).strip())[:200]
        result["content"] = _html_to_markdown(raw, url)
    except Exception as e:
        result["error"] = f"extract failed: {type(e).__name__}: {e}"
    return result


class UniversalFreeWebSearchProvider(WebSearchProvider):
    @property
    def name(self) -> str:
        return "universal_free"

    @property
    def display_name(self) -> str:
        return "Universal Free (GitHub + SearXNG + keyless extract)"

    def is_available(self) -> bool:
        return True

    def supports_search(self) -> bool:
        return True

    def supports_extract(self) -> bool:
        return True

    def search(self, query: str, limit: int = 5) -> Dict[str, Any]:
        results = _github_search(query, limit)
        if not results:
            results = _searxng_search(query, limit)
        if not results:
            results = _ddgs_search(query, limit)
        if results:
            return {"success": True, "data": {"web": results}}
        return {"success": False, "error": "All providers failed (network/rate-limited)."}

    def extract(self, urls: List[str], **kwargs: Any) -> List[Dict[str, Any]]:
        if not urls:
            return []
        results: List[Dict[str, Any]] = []
        with _cf.ThreadPoolExecutor(max_workers=min(5, len(urls))) as pool:
            futs = {pool.submit(_fetch_extract, u): u for u in urls}
            for fut in futs:
                url = futs[fut]
                try:
                    results.append(fut.result(timeout=_EXTRACT_TIMEOUT_SECS))
                except Exception as e:
                    results.append({"url": url, "title": "", "content": "", "error": f"timeout: {e}"})
        return results

    def get_setup_schema(self) -> Dict[str, Any]:
        return {"name": "Universal Free (GitHub + SearXNG + extract)", "badge": "free · no key · search + extract", "tag": "GitHub search, SearXNG public instances, DDGS fallback. Zero credentials.", "env_vars": []}