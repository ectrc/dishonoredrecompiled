"""Score a New Game run of our build against the retail recording of the opening (resources/reference/mission0/retail.mp4).

  python resources/tools/cutscene_compare.py run --exe <staged exe> --tag <tag> [--times 50:175:1] [--baseline build/cmp/<old>/scores.csv]
      launches through game_input.py (-windowed -ResX=1920 -ResY=1080 -nomovie -finewgame=50 -fosoiree -fzdiag -apshottime=<times>),
      queues `wait <quit-at>` / `quit`, waits for the exit, claims its own apshottime_cc_<tag>_*.bmp by name, reads each frame's
      real world time from the `-apshottime: screenshot i of n ... world time w` lines, rejects non-16:9 captures, stores PNGs
      under build/cmp/<tag>/ours/ and deletes the BMPs, then scores.
  python resources/tools/cutscene_compare.py run --ours-video <mp4> --tag <tag> [--step 1]
      scores a recording (OBS session, Bink phase) instead of a run; "world" is then the video time.
  python resources/tools/cutscene_compare.py score --tag <tag> [--baseline <scores.csv>]
      re-scores the frames a previous `run` stored (no game launch).
  python resources/tools/cutscene_compare.py compare <A> <B> [--times 20,40,60] [--tag selftest]
      scores two frame sets against each other; A/B are `retail`, a directory of PNGs named w<time>.png, or an mp4.
      `compare retail retail` and `selftest` are the instrument's own checks (SSIM >= .99, zoom 1.00, synthetic zoom found).

Output build/cmp/<tag>/: pairs/pair_w<world>.png (retail on top, ours below), diff/, scores.csv, report.md, index.html.
The retail side is decoded once into build/cmp/retail_cache/ (10 fps, 960x540). ffmpeg is not on PATH: CC_FFMPEG overrides
the path below. Nothing here touches the real mouse, keyboard or focus; input goes only through game_input.py.
"""
from __future__ import annotations

import argparse
import csv
import ctypes
import html
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

import numpy as np
from numpy.lib.stride_tricks import sliding_window_view
from PIL import Image

TOOLS = Path(__file__).resolve().parent
GAME_INPUT = TOOLS / "game_input.py"


def main_checkout() -> Path:
    here = TOOLS.parents[1]
    out = subprocess.run(["git", "-C", str(here), "rev-parse", "--git-common-dir"], capture_output=True, text=True)
    common = Path(out.stdout.strip()) if out.returncode == 0 and out.stdout.strip() else None
    if common is not None and not common.is_absolute():
        common = (here / common).resolve()
    return common.parent if common is not None else here


MAIN = main_checkout()
FFMPEG = os.environ.get("CC_FFMPEG", r"C:\Users\Frederik\AppData\Local\Microsoft\WinGet\Packages\Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-8.1-full_build\bin\ffmpeg.exe")
RETAIL_MP4 = Path(os.environ.get("CC_RETAIL_MP4", str(MAIN / "resources" / "reference" / "mission0" / "retail.mp4")))
CMP = MAIN / "build" / "cmp"
RETAIL_CACHE = CMP / "retail_cache"
GAME_ROOT = MAIN.parent / "Dishonored_Latest2026"
LOGS = GAME_ROOT / "DishonoredGame" / "Logs"
SHOTS = GAME_ROOT / "DishonoredGame" / "Screenshots" / "Win32Console"

W, H = 960, 540
FPS = 10
STATS_VERSION = 5

# (x0, x1, y0, y1) as fractions of the frame
AREAS = {
    "vials": (0.0, 0.16, 0.05, 0.30),
    "stance": (0.0, 0.10, 0.85, 1.0),
    "debugtext": (0.0, 1.0, 0.08, 0.14),
    "banner": (0.36, 0.64, 0.24, 0.34),
    "objective": (0.6, 1.0, 0.0, 0.2),
    "hint": (0.3, 0.7, 0.62, 0.72),
    "prompt": (0.5, 0.75, 0.33, 0.45),
}
DETECTORS = list(AREAS)
# a detector is "on" above this percentage of its area; set from the retail cache (see agentHH.md)
ON = {"vials": 3.0, "stance": 0.5, "debugtext": 0.2, "banner": 2.0, "objective": 3.0, "hint": 3.0, "prompt": 3.0}
CENTRE = (0.3, 0.7, 0.4, 0.8)
BLACK = 6.0


def area(img: np.ndarray, key) -> np.ndarray:
    x0, x1, y0, y1 = AREAS[key] if isinstance(key, str) else key
    h, w = img.shape[:2]
    return img[int(round(y0 * h)):int(round(y1 * h)), int(round(x0 * w)):int(round(x1 * w))]


def box(img: np.ndarray, k: int) -> np.ndarray:
    """Mean over a k x k window (same size, edges clamped) by cumulative sums."""
    r = k // 2
    p = np.pad(img.astype(np.float64), ((r + 1, k - r - 1), (r + 1, k - r - 1)), mode="edge")
    c = p.cumsum(0).cumsum(1)
    s = c[k:, k:] - c[:-k, k:] - c[k:, :-k] + c[:-k, :-k]
    return s / (k * k)


def window_sums(img: np.ndarray, k: int) -> np.ndarray:
    c = np.zeros((img.shape[0] + 1, img.shape[1] + 1))
    c[1:, 1:] = img.cumsum(0).cumsum(1)
    return c[k:, k:] - c[:-k, k:] - c[k:, :-k] + c[:-k, :-k]


def extremum(img: np.ndarray, k: int, fn) -> np.ndarray:
    r = k // 2
    p = np.pad(img, ((0, 0), (r, r)), mode="edge")
    out = fn(sliding_window_view(p, k, axis=1), axis=-1)
    p = np.pad(out, ((r, r), (0, 0)), mode="edge")
    return fn(sliding_window_view(p, k, axis=0), axis=-1)


def tophat(img: np.ndarray, k: int) -> np.ndarray:
    """White top-hat: what is brighter than its k x k opening, i.e. thin bright strokes such as text, not bright areas."""
    return img - extremum(extremum(img, k, np.min), k, np.max)


def grey(rgb: np.ndarray) -> np.ndarray:
    return rgb[..., 0] * 0.299 + rgb[..., 1] * 0.587 + rgb[..., 2] * 0.114


def load_rgb(path: Path) -> np.ndarray:
    img = Image.open(path).convert("RGB")
    if img.size != (W, H):
        img = img.resize((W, H), Image.BILINEAR)
    return np.asarray(img).astype(np.int16)


def stance_template() -> np.ndarray:
    """The crouch icon as our build draws it (ours.mp4 at 110 s, 960x540 pixels 24..102 x 438..526), band-passed; matched
    at (32, 446) +-4 px."""
    g = grey(np.asarray(Image.open(TOOLS / "cutscene_compare_stance.png").convert("RGB")).astype(np.float64))
    band = (box(g, 3) - box(g, 15))[8:-8, 8:-8]
    return band - band.mean()


STANCE = stance_template()
STANCE_AT = (32, 446)


class Frame:
    def __init__(self, rgb: np.ndarray):
        self.rgb = rgb
        self.g = grey(rgb.astype(np.float64))
        self._blur = None

    @property
    def blur(self) -> np.ndarray:
        if self._blur is None:
            self._blur = box(box(self.g, 5), 5)
        return self._blur

    def centre(self) -> float:
        return float(area(self.g, CENTRE).mean())

    def textlike(self, key: str) -> float:
        g = area(self.g, key)
        return float(((tophat(g, 7) > 60) & (g > 150)).mean() * 100)

    def stance(self) -> float:
        band = box(self.g, 3) - box(self.g, 15)
        x0, y0 = STANCE_AT
        th, tw = STANCE.shape
        best = -1.0
        for dy in range(-4, 5):
            for dx in range(-4, 5):
                p = band[y0 + dy:y0 + dy + th, x0 + dx:x0 + dx + tw]
                p = p - p.mean()
                n = np.sqrt((p * p).sum() * (STANCE * STANCE).sum())
                if n > 0:
                    best = max(best, float((p * STANCE).sum() / n))
        return best

    def detect(self) -> dict:
        r, gr, b = (self.rgb[..., i] for i in range(3))
        out = {}
        red = (r > 150) & (gr < 90)
        blue = (b > 150) & (b > r + 60)
        out["vials"] = float(area(red | blue, "vials").mean() * 100)
        out["stance"] = self.stance()
        cyan = area((gr > 100) & (b > 100) & (np.abs(gr - b) < 20) & (r < gr - 60), "debugtext")
        out["debugtext"] = float((cyan & (tophat(area(self.g, "debugtext"), 7) > 25)).mean() * 100)
        rest = self.g.copy()
        x0, x1, y0, y1 = AREAS["banner"]
        rest[int(y0 * H):int(y1 * H), int(x0 * W):int(x1 * W)] = np.nan
        rest[:, :int(0.2 * W)] = np.nan
        bright = float((area(self.g, "banner") > 150).mean() * 100)
        out["banner"] = bright if np.nanmean(rest) < 20 else 0.0
        for key in ("objective", "hint", "prompt"):
            out[key] = self.textlike(key)
        return out


def mad(a: Frame, b: Frame) -> float:
    return float(np.abs(a.blur - b.blur).mean())


def ssim(a: Frame, b: Frame, k: int = 8) -> float:
    x, y = a.g, b.g
    n = k * k
    sx, sy = window_sums(x, k), window_sums(y, k)
    sxx, syy, sxy = window_sums(x * x, k), window_sums(y * y, k), window_sums(x * y, k)
    mx, my = sx / n, sy / n
    vx, vy = sxx / n - mx * mx, syy / n - my * my
    cxy = sxy / n - mx * my
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    m = ((2 * mx * my + c1) * (2 * cxy + c2)) / ((mx * mx + my * my + c1) * (vx + vy + c2))
    return float(m.mean())


def zoom(ref: Frame, ours: Frame) -> tuple[float, float, float]:
    """Agent FE's fit: the scale s at which ours' centre crop, magnified by s about the screen centre, best matches retail's
    centre crop (NCC over band-passed grey). s > 1 means ours sees a wider field than retail. Returns (s, ncc, ncc at s=1)."""
    w0, h0 = int(0.4 * W), int(0.36 * H)
    band_r = box(ref.g, 3) - box(ref.g, 21)
    band_o = box(ours.g, 3) - box(ours.g, 21)
    target = band_r[(H - h0) // 2:(H - h0) // 2 + h0, (W - w0) // 2:(W - w0) // 2 + w0]
    t = target - target.mean()
    tn = np.sqrt((t * t).sum())
    if tn < 1e-6 or ref.g.std() < 4 or ours.g.std() < 4:
        return float("nan"), float("nan"), float("nan")
    src = Image.fromarray(band_o.astype(np.float32), mode="F")
    best = (float("nan"), -2.0)
    at1 = float("nan")
    for s in np.round(np.arange(0.5, 2.2001, 0.01), 2):
        cw, ch = w0 / s, h0 / s
        boxc = (W / 2 - cw / 2, H / 2 - ch / 2, W / 2 + cw / 2, H / 2 + ch / 2)
        o = np.asarray(src.resize((w0, h0), Image.BILINEAR, box=boxc), dtype=np.float64)
        o = o - o.mean()
        on = np.sqrt((o * o).sum())
        ncc = float((o * t).sum() / (on * tn)) if on > 1e-6 else -1.0
        if s == 1.0:
            at1 = ncc
        if ncc > best[1]:
            best = (float(s), ncc)
    return best[0], best[1], at1


# ---------------------------------------------------------------- video decode / retail cache

def ffmpeg_decode(mp4: Path, out_dir: Path) -> list[Path]:
    stamp = out_dir / "decoded.json"
    st = mp4.stat()
    key = {"src": str(mp4), "size": st.st_size, "mtime": int(st.st_mtime), "fps": FPS, "w": W, "h": H}
    if stamp.is_file() and json.loads(stamp.read_text()) == key:
        return sorted(out_dir.glob("f_*.png"))
    if not Path(FFMPEG).is_file():
        raise SystemExit(f"cutscene_compare: ffmpeg not found at {FFMPEG} (set CC_FFMPEG)")
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in out_dir.glob("f_*.png"):
        old.unlink()
    print(f"decoding {mp4} -> {out_dir} ({FPS} fps, {W}x{H})")
    subprocess.run([FFMPEG, "-v", "error", "-y", "-i", str(mp4), "-vf", f"fps={FPS},scale={W}:{H}:flags=area",
                    "-start_number", "0", str(out_dir / "f_%05d.png")], check=True)
    stamp.write_text(json.dumps(key))
    return sorted(out_dir.glob("f_*.png"))


class VideoSet:
    """A decoded video: frame k is time k / FPS."""

    def __init__(self, mp4: Path, cache: Path):
        self.files = ffmpeg_decode(mp4, cache)
        self.cache = cache
        self.duration = len(self.files) / FPS
        self.stats = self.load_stats()

    def load_stats(self) -> dict:
        path = self.cache / "stats.json"
        if path.is_file():
            data = json.loads(path.read_text())
            if data.get("version") == STATS_VERSION and len(data["centre"]) == len(self.files):
                return data
        print(f"measuring {len(self.files)} frames of {self.cache.name}")
        centre, det = [], {k: [] for k in DETECTORS}
        for f in self.files:
            fr = Frame(load_rgb(f))
            centre.append(fr.centre())
            for k, v in fr.detect().items():
                det[k].append(v)
        data = {"version": STATS_VERSION, "centre": centre, "det": det}
        path.write_text(json.dumps(data))
        return data

    def times(self) -> np.ndarray:
        return np.arange(len(self.files)) / FPS

    def frame_at(self, t: float) -> tuple[Frame | None, float]:
        k = int(round(t * FPS))
        if k < 0 or k >= len(self.files):
            return None, float("nan")
        return Frame(load_rgb(self.files[k])), k / FPS


def retail_set() -> VideoSet:
    return VideoSet(RETAIL_MP4, RETAIL_CACHE)


# ---------------------------------------------------------------- events and alignment

def events(times, centre, banner) -> dict:
    """E1 = first sample with the location banner on; E2 = first sample after the longest black run whose centre brightness
    reaches 50 % of the median of everything after that run (interpolated between samples); F = the first sample after the
    black run that is not black (first boat frame)."""
    times, centre, banner = np.asarray(times, float), np.asarray(centre, float), np.asarray(banner, float)
    out = {}
    on = np.nonzero(banner > ON["banner"])[0]
    if len(on):
        out["E1"] = float(times[on[0]])
    dark = centre < BLACK
    best, start = (0.0, -1, -1), None
    for i, d in enumerate(list(dark) + [False]):
        if d and start is None:
            start = i
        elif not d and start is not None:
            span = times[i - 1] - times[start]
            if span > best[0]:
                best = (span, start, i - 1)
            start = None
    if best[1] < 0 or best[0] < 1.5 or best[2] + 1 >= len(times):
        return out
    end = best[2]
    out["black"] = (float(times[best[1]]), float(times[end]))
    out["F"] = float(times[end + 1])
    ride = centre[end + 1:]
    half = 0.5 * float(np.median(ride))
    out["ride_median"] = float(np.median(ride))
    for i in range(end + 1, len(times)):
        if centre[i] >= half:
            c0, c1 = centre[i - 1], centre[i]
            f = (half - c0) / (c1 - c0) if c1 > c0 else 1.0
            out["E2"] = float(times[i - 1] + f * (times[i] - times[i - 1]))
            break
    return out


def fit_alignment(ours_ev: dict, retail_ev: dict, fallback_world: float | None) -> dict:
    pts = [(k, ours_ev[k], retail_ev[k]) for k in ("E1", "E2") if k in ours_ev and k in retail_ev]
    fb = None
    if fallback_world is not None and "F" in retail_ev:
        fb = (fallback_world, retail_ev["F"])
    res = {"points": [(k, w, r) for k, w, r in pts], "fallback": fb}
    if len(pts) >= 2:
        w = np.array([p[1] for p in pts])
        r = np.array([p[2] for p in pts])
        b, a = np.polyfit(w, r, 1)
        res.update(a=float(a), b=float(b), mode="fit " + "+".join(p[0] for p in pts))
        if not 0.9 <= b <= 1.1:
            res.update(a=float((r - w).mean()), b=1.0, mode=res["mode"] + f" (b={b:.3f} out of range, b fixed 1)")
    elif len(pts) == 1:
        res.update(a=pts[0][2] - pts[0][1], b=1.0, mode=f"{pts[0][0]} only, b fixed 1")
    elif fb is not None:
        res.update(a=fb[1] - fb[0], b=1.0, mode="fallback soiree ENTER vs retail first boat frame")
    else:
        ofb = ours_ev.get("F")
        if ofb is None or "F" not in retail_ev:
            raise SystemExit("cutscene_compare: no event to align on (no banner, no black run, no soiree ENTER)")
        res.update(a=retail_ev["F"] - ofb, b=1.0, mode="first non-black frame after the black on both sides")
    resid = [r - (res["a"] + res["b"] * w) for _, w, r in pts]
    res["residual"] = float(np.sqrt(np.mean(np.square(resid)))) if resid else float("nan")
    res["residuals"] = {k: r - (res["a"] + res["b"] * w) for k, w, r in pts}
    if fb is not None:
        res["fallback_residual"] = fb[1] - (res["a"] + res["b"] * fb[0])
    for k in ("E1", "E2", "F"):
        if k in ours_ev and k in retail_ev:
            res[f"offset_{k}"] = retail_ev[k] - ours_ev[k]
    return res


# ---------------------------------------------------------------- the game run

PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
STILL_ACTIVE = 259


def pid_alive(pid: int) -> bool:
    k32 = ctypes.windll.kernel32
    k32.OpenProcess.restype = ctypes.c_void_p
    k32.OpenProcess.argtypes = [ctypes.c_uint32, ctypes.c_int, ctypes.c_uint32]
    h = k32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
    if not h:
        return False
    try:
        code = ctypes.c_ulong()
        k32.GetExitCodeProcess.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ulong)]
        if not k32.GetExitCodeProcess(h, ctypes.byref(code)):
            return False
        return code.value == STILL_ACTIVE
    finally:
        k32.CloseHandle.argtypes = [ctypes.c_void_p]
        k32.CloseHandle(h)


def parse_times(spec: str) -> list[float]:
    out = []
    for part in spec.split(","):
        if ":" in part:
            a, b, s = (float(v) for v in part.split(":"))
            out += [round(v, 3) for v in np.arange(a, b + s / 2, s)]
        elif part.strip():
            out.append(float(part))
    return sorted(set(out))


LOG_TS = re.compile(r"^\[(\d+\.\d+)\]")
SHOT_LINE = re.compile(r"-apshottime: screenshot (\d+) of (\d+), mark ([\d.]+), frame at world time ([\d.]+)")


def parse_log(log: Path) -> dict:
    shots, anchors = [], {}
    d3d9 = None
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        m = LOG_TS.match(line)
        ts = float(m.group(1)) if m else None
        s = SHOT_LINE.search(line)
        if s:
            shots.append({"i": int(s.group(1)), "n": int(s.group(2)), "mark": float(s.group(3)), "world": float(s.group(4)), "ts": ts})
            continue
        if d3d9 is None and "scene rendered (0 so far, d3d9" in line:
            d3d9 = ts
        for key, pat in (("finewgame", "-finewgame at world time"), ("soiree_enter", "soiree ENTER"), ("soiree_exit", "soiree EXIT"),
                         ("blackstripes", "SetBlackStripes(1)"), ("location_discovery", "ShowLocationDiscovery"),
                         ("new_game_confirm", "OnNewGameConfirm")):
            if pat in line and ts is not None:
                anchors.setdefault(key, []).append(ts)
        if "Critical" in line:
            anchors.setdefault("critical", []).append(ts)
    return {"shots": shots, "anchors_ts": anchors, "d3d9_ts": d3d9}


def ts_to_world(ts: float, shots: list[dict]) -> float:
    pts = [(s["ts"], s["world"]) for s in shots if s["ts"] is not None]
    if not pts:
        return float("nan")
    t = np.array([p[0] for p in pts])
    w = np.array([p[1] for p in pts])
    i = int(np.argmin(np.abs(t - ts)))
    return float(ts - t[i] + w[i])


def run_game(args) -> Path:
    out = CMP / args.tag
    ours_dir = out / "ours"
    ours_dir.mkdir(parents=True, exist_ok=True)
    shot_tag = f"cc_{args.tag}_"
    shot_re = re.compile(rf"^apshottime_{re.escape(shot_tag)}(\d+)\.bmp$", re.I)
    for stale in SHOTS.glob(f"apshottime_{shot_tag}*.bmp"):
        if shot_re.match(stale.name):
            stale.unlink()
    times = parse_times(args.times)
    log_name = f"CC_{args.tag}"
    ini_dir = out / "config"
    ini_dir.mkdir(parents=True, exist_ok=True)
    for stale in ini_dir.glob("Dishonored*.ini"):
        stale.unlink()
    cmd_file = out / "inputcmd.txt"
    # -nomovie: since HK (d6c3aa4) Bink is on by default and the briefing page waits for a key from controller 0,
    # which the scripted -finewgame route never assigns, so the map change blocks in GameThreadWaitForMovie forever
    # (agent IA, 2026-10-19). The gate passes -nomovie for the same reason; the Bink pages are scored by HG/HK's own runs.
    extra = ["-windowed", "-ResX=1920", "-ResY=1080", "-nomovie", "-finewgame=50", "-fosoiree", "-fzdiag",
             "-apshottime=" + ",".join(f"{t:g}" for t in times)]
    extra += [f"-{kind}INI={ini_dir / ('Dishonored' + name + '.ini')}" for kind, name in (("ENGINE", "Engine"), ("GAME", "Game"), ("INPUT", "Input"), ("UI", "UI"))]
    extra += args.extra
    launch = [sys.executable, str(GAME_INPUT), "launch", "--exe", args.exe, "--log", f"{log_name}.log", "--tag", shot_tag,
              "--cmd-file", str(cmd_file), "--", *extra]
    t0 = time.time()
    res = subprocess.run(launch, capture_output=True, text=True)
    print(res.stdout.strip())
    m = re.search(r"launched pid (\d+)", res.stdout)
    if res.returncode != 0 or not m:
        raise SystemExit(f"cutscene_compare: launch failed\n{res.stderr}")
    pid = int(m.group(1))
    quit_at = args.quit_at if args.quit_at else max(times) + 10
    subprocess.run([sys.executable, str(GAME_INPUT), "--cmd-file", str(cmd_file), "raw", f"wait {quit_at:g}", "quit"], check=True)
    while pid_alive(pid) and time.time() - t0 < args.timeout:
        time.sleep(5)
    if pid_alive(pid):
        print(f"pid {pid} still alive after {args.timeout:.0f} s; it is left running, the frames written so far are scored")
    wall = time.time() - t0
    log = LOGS / f"{log_name}.log"
    info = parse_log(log)
    bmps = sorted((p for p in SHOTS.glob(f"apshottime_{shot_tag}*.bmp") if shot_re.match(p.name)),
                  key=lambda p: int(shot_re.match(p.name).group(1)))
    shots = info["shots"]
    if len(bmps) != len(shots):
        print(f"WARNING: {len(bmps)} bitmaps for {len(shots)} -apshottime lines; pairing in order")
    for old in ours_dir.glob("w*.png"):
        old.unlink()
    kept, rejected = [], []
    for bmp, shot in zip(bmps, shots):
        img = Image.open(bmp).convert("RGB")
        w, h = img.size
        shot.update(width=w, height=h)
        if abs(w * 9 - h * 16) > 16:
            rejected.append(shot)
            img.close()
            continue
        png = ours_dir / f"w{shot['world']:08.3f}.png"
        img.save(png)
        img.close()
        shot["png"] = png.name
        kept.append(shot)
    for bmp in bmps:
        bmp.unlink()
    meta = {"exe": args.exe, "tag": args.tag, "log": str(log), "times": times, "wall_s": round(wall, 1),
            "bitmaps": len(bmps), "kept": len(kept), "rejected": [(s["world"], s["width"], s["height"]) for s in rejected],
            "shots": kept, "anchors_ts": info["anchors_ts"], "d3d9_ts": info["d3d9_ts"],
            "anchors_world": {k: [ts_to_world(t, shots) for t in v] for k, v in info["anchors_ts"].items() if k != "critical"}}
    (out / "run.json").write_text(json.dumps(meta, indent=1))
    print(f"{len(kept)} frames kept, {len(rejected)} rejected (not 16:9), wall {wall:.0f} s, d3d9 first scene at log {info['d3d9_ts']}")
    return out


def video_ours(args) -> Path:
    mp4 = Path(args.ours_video).resolve()
    out = CMP / args.tag
    out.mkdir(parents=True, exist_ok=True)
    vs = VideoSet(mp4, CMP / "video_cache" / mp4.stem)
    meta = {"video": str(mp4), "tag": args.tag, "duration": vs.duration, "step": args.step, "anchors_world": {}}
    (out / "run.json").write_text(json.dumps(meta, indent=1))
    return out


# ---------------------------------------------------------------- scoring

class OursSet:
    def __init__(self, out: Path, meta: dict, step: float):
        self.meta = meta
        if "video" in meta:
            mp4 = Path(meta["video"])
            self.video = VideoSet(mp4, CMP / "video_cache" / mp4.stem)
            self.samples = [(float(t), None) for t in np.arange(0, self.video.duration - 0.05, step)]
            self.ev_times = self.video.times()
            self.ev_centre = self.video.stats["centre"]
            self.ev_banner = self.video.stats["det"]["banner"]
        else:
            self.video = None
            self.samples = [(s["world"], out / "ours" / s["png"]) for s in meta["shots"]]
            frames = [Frame(load_rgb(p)) for _, p in self.samples]
            self.ev_times = [w for w, _ in self.samples]
            self.ev_centre = [f.centre() for f in frames]
            self.ev_banner = [f.detect()["banner"] for f in frames]

    def frame(self, world: float, path) -> Frame:
        if self.video is not None:
            return self.video.frame_at(world)[0]
        return Frame(load_rgb(path))


def pair_image(top: np.ndarray, bottom: np.ndarray, path: Path) -> None:
    Image.fromarray(np.vstack([top, bottom]).astype(np.uint8)).save(path)


def score(out: Path, step: float = 1.0, baseline: str | None = None) -> list[dict]:
    meta = json.loads((out / "run.json").read_text())
    retail = retail_set()
    ours = OursSet(out, meta, step)
    r_ev = events(retail.times(), retail.stats["centre"], retail.stats["det"]["banner"])
    o_ev = events(ours.ev_times, ours.ev_centre, ours.ev_banner)
    enter = meta.get("anchors_world", {}).get("soiree_enter")
    align = fit_alignment(o_ev, r_ev, enter[0] if enter else None)
    a, b = align["a"], align["b"]
    for sub in ("pairs", "diff"):
        (out / sub).mkdir(exist_ok=True)
        for old in (out / sub).glob("*.png"):
            old.unlink()
    rows = []
    for world, path in ours.samples:
        rt = a + b * world
        rf, rt_used = retail.frame_at(rt)
        of = ours.frame(world, path)
        if of is None:
            continue
        row = {"world": round(world, 3), "retail_t": round(rt_used, 2) if rf is not None else "", "ours_centre": round(of.centre(), 1)}
        od = of.detect()
        if rf is not None:
            s, ncc, ncc1 = zoom(rf, of)
            row.update(mad=round(mad(rf, of), 2), ssim=round(ssim(rf, of), 4), zoom=round(s, 2) if s == s else "",
                       zoom_ncc=round(ncc, 3) if ncc == ncc else "", ncc_at_1=round(ncc1, 3) if ncc1 == ncc1 else "",
                       retail_centre=round(rf.centre(), 1))
            pair_image(rf.rgb, of.rgb, out / "pairs" / f"pair_w{world:08.3f}.png")
            d = np.clip(np.abs(rf.blur - of.blur) * 3, 0, 255).astype(np.uint8)
            Image.fromarray(d).save(out / "diff" / f"diff_w{world:08.3f}.png")
            rd = rf.detect()
        else:
            rd = {}
        for k in DETECTORS:
            row[k] = round(od[k], 3)
        for k in DETECTORS:
            row["retail_" + k] = round(rd[k], 3) if k in rd else ""
        rows.append(row)
    cols = ["world", "retail_t", "mad", "ssim", "zoom", "zoom_ncc", "ncc_at_1", "ours_centre", "retail_centre"] + DETECTORS + ["retail_" + k for k in DETECTORS]
    with (out / "scores.csv").open("w", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=cols)
        wr.writeheader()
        for r in rows:
            wr.writerow({c: r.get(c, "") for c in cols})
    if not meta.get("anchors_world", {}).get("soiree_exit"):
        mapped = [r["world"] for r in rows if r["retail_t"] != ""]
        meta["ride"] = (o_ev.get("F", rows[0]["world"]), max(mapped) if mapped else rows[-1]["world"])
    summary = summarise(rows, meta)
    base_txt = baseline_table(rows, meta, Path(baseline)) if baseline else ""
    write_report(out, meta, rows, align, o_ev, r_ev, summary, base_txt)
    write_index(out, rows)
    print((out / "report.md").read_text())
    return rows


def ride_window(meta: dict) -> tuple[float, float]:
    aw = meta.get("anchors_world", {})
    if aw.get("soiree_enter") and aw.get("soiree_exit"):
        return aw["soiree_enter"][0], aw["soiree_exit"][-1]
    return tuple(meta["ride"])


def nums(rows, key, lo=-1e9, hi=1e9):
    return np.array([r[key] for r in rows if lo <= r["world"] <= hi and r.get(key, "") != ""], float)


def summarise(rows: list[dict], meta: dict) -> dict:
    lo, hi = ride_window(meta)
    s = {"ride": (lo, hi)}
    for key in ("mad", "ssim", "zoom", "zoom_ncc"):
        v = nums(rows, key, lo, hi)
        s[key] = float(np.median(v)) if len(v) else float("nan")
        v = nums(rows, key)
        s[key + "_all"] = float(np.median(v)) if len(v) else float("nan")
    for k in DETECTORS:
        on = [r["world"] for r in rows if r[k] > ON[k]]
        s[k] = on
        s["retail_" + k] = [r["world"] for r in rows if r.get("retail_" + k, "") != "" and r["retail_" + k] > ON[k]]
    return s


def spans(ws: list[float], gap: float = 2.5) -> str:
    if not ws:
        return "-"
    out, start, prev = [], ws[0], ws[0]
    for w in ws[1:]:
        if w - prev > gap:
            out.append((start, prev))
            start = w
        prev = w
    out.append((start, prev))
    return ", ".join(f"{a:.1f}" if a == b else f"{a:.1f}-{b:.1f}" for a, b in out)


def baseline_table(rows: list[dict], meta: dict, base_csv: Path) -> str:
    with base_csv.open() as f:
        base = [{k: (float(v) if v not in ("",) and k != "x" else v) for k, v in r.items()} for r in csv.DictReader(f)]
    lo, hi = ride_window(meta)
    lines = [f"Before/after against `{base_csv}` (medians over world {lo:.1f}-{hi:.1f}; detector = frames on / frames)", "",
             "| metric | before | after | change |", "|---|---|---|---|"]
    for key in ("mad", "ssim", "zoom", "zoom_ncc"):
        bv, av = nums(base, key, lo, hi), nums(rows, key, lo, hi)
        bm = float(np.median(bv)) if len(bv) else float("nan")
        am = float(np.median(av)) if len(av) else float("nan")
        ch = (am - bm) / bm * 100 if bm == bm and bm != 0 else float("nan")
        lines.append(f"| {key} | {bm:.3f} | {am:.3f} | {ch:+.1f} % |")
    for k in DETECTORS:
        bon = sum(1 for r in base if r.get(k, "") != "" and r[k] > ON[k])
        aon = sum(1 for r in rows if r[k] > ON[k])
        lines.append(f"| {k} | {bon}/{len(base)} | {aon}/{len(rows)} | |")
    txt = "\n".join(lines)
    print(txt)
    return txt


def write_report(out, meta, rows, align, o_ev, r_ev, s, base_txt) -> None:
    L = [f"# cutscene_compare: {meta['tag']}", ""]
    if "video" in meta:
        L.append(f"Ours: video `{meta['video']}` ({meta['duration']:.1f} s); world = video time.")
    else:
        L.append(f"Ours: `{meta['exe']}`, log `{meta['log']}`, wall {meta['wall_s']} s, d3d9 first scene at log {meta['d3d9_ts']} s.")
        L.append(f"Frames: {meta['bitmaps']} bitmaps, {meta['kept']} kept, rejected (not 16:9): {meta['rejected'] or 'none'}.")
        sizes = sorted({(x['width'], x['height']) for x in meta['shots']})
        L.append(f"Capture sizes: {sizes}.")
        aw = {k: [round(v, 2) for v in vs] for k, vs in meta.get("anchors_world", {}).items()}
        L.append(f"Log anchors (world time): {aw}.")
        if meta.get("anchors_ts", {}).get("critical"):
            L.append(f"**Critical lines in the log at {meta['anchors_ts']['critical'][:5]}**")
    L += ["", "## Alignment", "",
          f"retail_t = {align['a']:.3f} + {align['b']:.4f} * world  ({align['mode']}); residual {align['residual']:.3f} s",
          f"ours events: {fmt_ev(o_ev)}", f"retail events: {fmt_ev(r_ev)}"]
    if align.get("fallback"):
        L.append(f"fallback soiree ENTER world {align['fallback'][0]:.2f} vs retail first boat frame {align['fallback'][1]:.1f}: "
                 f"residual {align.get('fallback_residual', float('nan')):+.2f} s")
    for k in ("E1", "E2", "F"):
        if f"offset_{k}" in align:
            L.append(f"offset retail-ours at {k}: {align[f'offset_{k}']:.2f} s")
    lo, hi = s["ride"]
    L += ["", "## Summary", "", f"Ride window world {lo:.1f}-{hi:.1f}.", "",
          "| metric | median over ride | median over all |", "|---|---|---|"]
    for key in ("mad", "ssim", "zoom", "zoom_ncc"):
        L.append(f"| {key} | {s[key]:.3f} | {s[key + '_all']:.3f} |")
    L += ["", "| detector | on threshold % | ours on at world | retail on at (ours world) |", "|---|---|---|---|"]
    for k in DETECTORS:
        L.append(f"| {k} | {ON[k]} | {spans(s[k])} | {spans(s['retail_' + k])} |")
    if base_txt:
        L += ["", "## Baseline", "", base_txt]
    L += ["", "Files: `pairs/` (retail top, ours bottom), `diff/` (|blurred grey| x3), `scores.csv`, `index.html`."]
    (out / "report.md").write_text("\n".join(L) + "\n")


def fmt_ev(ev: dict) -> str:
    parts = []
    for k, v in ev.items():
        parts.append(f"{k} {v[0]:.1f}-{v[1]:.1f}" if isinstance(v, tuple) else f"{k} {v:.2f}")
    return ", ".join(parts) or "none"


def write_index(out: Path, rows: list[dict]) -> None:
    cols = ["world", "retail_t", "mad", "ssim", "zoom"] + DETECTORS
    trs = []
    for r in rows:
        name = f"pair_w{r['world']:08.3f}.png"
        cells = "".join(f"<td>{html.escape(str(r.get(c, '')))}</td>" for c in cols)
        img = f'<a href="pairs/{name}"><img src="pairs/{name}" loading="lazy"></a>' if (out / "pairs" / name).is_file() else ""
        trs.append(f"<tr>{cells}<td>{img}</td></tr>")
    head = "".join(f"<th>{c}</th>" for c in cols)
    page = f"""<!doctype html><html><head><meta charset="utf-8"><title>Cutscene compare</title>
<style>body{{font:13px sans-serif;background:#111;color:#ddd}}table{{border-collapse:collapse}}td,th{{border:1px solid #333;padding:2px 6px}}img{{width:240px}}</style>
</head><body><h3>{html.escape(out.name)}</h3><p>See report.md. Pairs: retail on top, ours below.</p>
<table><tr>{head}<th>pair</th></tr>{''.join(trs)}</table></body></html>"""
    (out / "index.html").write_text(page, encoding="utf-8")


# ---------------------------------------------------------------- compare / selftest

def frame_source(spec: str):
    if spec == "retail":
        rs = retail_set()
        return lambda t: rs.frame_at(t)[0]
    p = Path(spec)
    if p.suffix.lower() == ".mp4":
        vs = VideoSet(p.resolve(), CMP / "video_cache" / p.stem)
        return lambda t: vs.frame_at(t)[0]
    files = {float(f.stem[1:]): f for f in p.glob("w*.png")}
    keys = np.array(sorted(files))
    return lambda t: Frame(load_rgb(files[float(keys[np.argmin(np.abs(keys - t))])]))


def compare(args) -> int:
    fa, fb = frame_source(args.a), frame_source(args.b)
    times = parse_times(args.times)
    print("t,mad,ssim,zoom,zoom_ncc")
    res = []
    for t in times:
        a, b = fa(t), fb(t)
        if a is None or b is None:
            continue
        s, n, _ = zoom(a, b)
        res.append((t, mad(a, b), ssim(a, b), s, n))
        print(f"{t:g},{res[-1][1]:.3f},{res[-1][2]:.4f},{s:.2f},{n:.3f}")
    arr = np.array([r[1:] for r in res], float)
    print(f"median mad {np.nanmedian(arr[:, 0]):.3f} ssim {np.nanmedian(arr[:, 1]):.4f} zoom {np.nanmedian(arr[:, 2]):.2f}")
    return 0


def selftest(args) -> int:
    rs = retail_set()
    ok = True
    print("retail vs retail (same cached frame) and vs a synthetic wider field (retail shrunk about the centre by 1/s0):")
    for t in parse_times(args.times):
        f, _ = rs.frame_at(t)
        if f is None:
            continue
        s, n, _ = zoom(f, f)
        sm = ssim(f, f)
        line = f"t={t:g}: self ssim {sm:.4f} mad {mad(f, f):.3f} zoom {s:.2f} (ncc {n:.3f})"
        ok &= sm >= 0.99 and abs(s - 1.0) < 0.005
        for s0 in (1.5, 1.78):
            img = Image.fromarray(f.rgb.astype(np.uint8))
            small = img.resize((round(W / s0), round(H / s0)), Image.BILINEAR)
            wide = Image.new("RGB", (W, H))
            wide.paste(small, ((W - small.width) // 2, (H - small.height) // 2))
            s2, n2, _ = zoom(f, Frame(np.asarray(wide).astype(np.int16)))
            line += f"; synthetic {s0} -> {s2:.2f} (ncc {n2:.3f})"
            ok &= abs(s2 - s0) <= 0.03
        print(line)
    det = {k: float(np.max(rs.stats["det"][k])) for k in DETECTORS}
    print("retail detector maxima over the whole video (%):", {k: round(v, 3) for k, v in det.items()})
    for k in DETECTORS:
        on = [i / FPS for i, v in enumerate(rs.stats["det"][k]) if v > ON[k]]
        print(f"  retail {k} on at t: {spans(on, 0.25)}")
    print("retail events:", fmt_ev(events(rs.times(), rs.stats["centre"], rs.stats["det"]["banner"])))
    print("SELFTEST", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("--exe", default=str(GAME_ROOT / "Binaries" / "Win32" / "DishonoredGame-Win64-Shipping.exe"))
    r.add_argument("--tag", required=True)
    r.add_argument("--times", default="50:175:1", help="world-time marks: a:b:step and/or comma list")
    r.add_argument("--quit-at", type=float, default=185.0, help="world time of the scripted quit")
    r.add_argument("--timeout", type=float, default=900.0, help="wall-clock seconds to wait for the exit")
    r.add_argument("--ours-video", help="score this mp4 instead of launching the game")
    r.add_argument("--step", type=float, default=1.0, help="--ours-video: sample spacing in seconds")
    r.add_argument("--baseline")
    r.add_argument("extra", nargs="*", help="extra game arguments after --")
    s = sub.add_parser("score")
    s.add_argument("--tag", required=True)
    s.add_argument("--step", type=float, default=1.0)
    s.add_argument("--baseline")
    c = sub.add_parser("compare")
    c.add_argument("a")
    c.add_argument("b")
    c.add_argument("--times", default="16,24,30,40,60,80,100,108")
    t = sub.add_parser("selftest")
    t.add_argument("--times", default="16,30,40,60,80,100,108")
    args = ap.parse_args()
    if args.cmd == "run":
        out = video_ours(args) if args.ours_video else run_game(args)
        score(out, args.step, args.baseline)
    elif args.cmd == "score":
        score(CMP / args.tag, args.step, args.baseline)
    elif args.cmd == "compare":
        return compare(args)
    elif args.cmd == "selftest":
        return selftest(args)
    return 0


if __name__ == "__main__":
    sys.exit(main())
