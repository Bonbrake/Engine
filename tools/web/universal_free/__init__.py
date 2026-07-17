"""Universal Free web provider plugin entrypoint.

Registers the keyless search+extract provider. Auto-discovered by Hermes'
plugin loader (kind: backend) -- no config key required beyond pointing
``web.backend`` at ``universal_free``.
"""

from .provider import UniversalFreeWebSearchProvider


def register(ctx) -> None:
    """Register the universal-free provider with the Hermes plugin context."""
    ctx.register_web_search_provider(UniversalFreeWebSearchProvider())
