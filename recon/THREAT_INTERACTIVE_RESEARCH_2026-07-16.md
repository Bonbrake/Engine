# Threat Interactive — Primary-Source Research (fetch-verified 2026-07-16)

> Resolves open item [B] from SESSION_STATE_2026-07-16.md. Prior session could only
> paraphrase their thesis from a third-party summary (se7en.ws) because search returned
> HTTP 429. This session, search rate-limit had cooled and the YouTube channel was
> reachable. Findings below are PRIMARY SOURCE (their own channel video titles + chapter
> lists pulled live from youtube.com), NOT a third-party paraphrase.

## How it was obtained
- Egress search (DDG/Bing/Mojeek/Startpage) was partly bot-blocked; Startpage returned
  discussion hits, Bing returned an empty shell. YouTube channel `@ThreatInteractive`
  loaded fine (1.1 MB). Channel ID `UC2ksme4hP4Nx97ilFTQ0K0Q`; 15 videos in the public RSS.
- Captions API was IP-blocked (`ip=0.0.0.0` in the signed timedtext URL → environmental,
  not a script bug). Could NOT pull transcript text. Video TITLES + chapter lists in each
  video's `shortDescription` ARE primary source and were extracted successfully.
- Authoritative enough to confirm thesis framing and the lighting-fix provenance; NOT
  enough to quote exact technical claims beyond what the chapter titles state.

## Their own words — the lighting fix provenance (CONFIRMS M4.5-EXT-20)
Video 20 — "How Incompetent Graphics Create Fake Realism & Why Your Brain Hates it"
(https://www.youtube.com/watch?v=qZtNU-4yqtI). Chapter list (verbatim from their description):
```
What Are BRDFs? : 0:25
Common Models Vs The Reference : 1:30
The Disney/Burley Diffuse : 2:38
The BIG Problem With Unreal Engine : 3:17
Callisto BRDF Does Everything : 5:03
Proof As To Why Most (If Not All) Games Look Like Plastic : 5:24
Callisto BRDF Issues (Part 1) : 6:36
The Pathetic State Of Massive Budget Productions : 8:14
Conclusion 3 (Why This is NOT a Subjective Matter) : 11:12
Conclusion 4 (AI is NOT the Answer) : 11:39
```
=> The Burley/Disney diffuse + "Callisto BRDF" (Kalisto) thesis we applied to M4.5-EXT-20
is DIRECTLY their Video 20 thesis. Previously this was only second-hand (se7en.ws); now
fetch-verified against their own chapter list. The fix holds with primary-source backing.

## Their own words — the "slop / regression / 9th-gen neglect" thesis (CONFIRMS [B] mapping)
Video 30 — "The Regression In Graphics Is A Psychotic, Anti-Consumer Mess"
(https://www.youtube.com/watch?v=xojL9MdxcRs). Chapter list (verbatim):
```
0:00  This Test Was Stupid
2:12  How Incompetent Graphics Hurt Consumers
5:10  Developer Efforts History Beat Vendor Locked Graphics
6:47  The Slow Destruction Of Optimization Market Value
10:15 Stop Defending Graphics That Are Purposefully Worse
13:14 The Constant Reward System Of Garbage & Lies
14:22 Our Channel Is Counteracting The Lies
```
=> The "stop defending graphics that are purposefully worse" + "reward system of garbage"
framing maps exactly to the spec guardrails we drafted: silhouette-preserving LOD,
material-identity-preserving streaming, Tier-0 floor as a MINIMUM not the target. Now
primary-source confirmed (was previously paraphrased).

## Other on-point primary-source titles (for future spec mapping if needed)
- "Understanding Crysis 3's Rendering Will Make You Hate Where We Ended Up" (ElBUUMi_L5c)
- "Why MSAA Should Be In EVERY Deferred Renderer | The Incompetence Of Crysis 3's Implementation" (SxCMaTEoBoI)
- "We Are Proving Older Realistic Effects Are Rendering Faster" (ljWylilACdI)
- "This Vendor Agnostic Ray Tracing Runs 120FPS: And It's Not Even Optimized!" (yxSrDAOB2xc)
- "How Textures & GPU Utilization Make Or Break Optimization" (sjZX5u3I4rM)
- "When a Poorly Optimized Menu Kills The Entire Game's FPS" (ytUD8UkrY18)
- "Devs Can Optimize Graphics By Never Letting THIS Happen" (T1MKlxM04L4)
- "When Butchered Rasterization Leads To Raytracing (It's Crazy)" (PhEIa5RtKVA)
- "Took For Granted: Why Fox Engine Is So Crazy Optimized" (aB5qxp6SPPQ)
- "Watch Threat Interactive 26 ... Fox Engine Rendering In MGSV" (w1OzfuqCS10)

## What remains UNVERIFIED (do NOT fabricate)
- Exact technical mechanism of their LOD/streaming critique (they argue for
  silhouette/material-identity preservation, but the precise algorithmic claim was NOT
  extracted — only the thesis framing via chapter titles).
- Any claim requiring transcript text (captions IP-blocked this session).
- ACTION if deeper quotes needed later: retry captions after egress IP rotates, OR user
  drops a saved transcript/video file. For now, thesis framing is verified; specifics are not.

## Spec impact (already applied / to apply)
- M4.5-EXT-20 Burley/Kalisto diffuse: APPLIED earlier, now PRIMARY-SOURCE CONFIRMED. ✅
- M4 (streaming) + M4.5 (LOD) silhouette/material-identity guardrails: drafted in
  SESSION_STATE [B] mapping; NOT yet written into milestone files (those are GATED spec
  edits — needs user GO, see [A]/[C]/[D] decision).
