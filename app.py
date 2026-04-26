import streamlit as st
import subprocess
import os
import tempfile
def extract_phase(output, start_tag, end_tag):
    lines = output.split("\n")
    capture = False
    result = []

    for line in lines:
        if start_tag in line:
            capture = True
            continue
        if end_tag in line:
            break
        if capture:
            result.append(line)

    return "\n".join(result)

# ── Page config ──────────────────────────────────────────────────────────────
st.set_page_config(
    page_title="Mini-C Compiler",
    page_icon="⚙️",
    layout="wide",
    initial_sidebar_state="collapsed"
)

# ── Custom CSS ────────────────────────────────────────────────────────────────
st.markdown("""
<style>
@import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;700&family=Orbitron:wght@700;900&family=Rajdhani:wght@400;600;700&display=swap');

/* ── background ── */
.stApp {
    background: linear-gradient(135deg, #0a0e1a 0%, #0d1b2a 40%, #0a1628 70%, #0e0a1a 100%);
    min-height: 100vh;
}

/* animated grid background */
.stApp::before {
    content: '';
    position: fixed;
    top: 0; left: 0; right: 0; bottom: 0;
    background-image:
        linear-gradient(rgba(0,255,200,0.03) 1px, transparent 1px),
        linear-gradient(90deg, rgba(0,255,200,0.03) 1px, transparent 1px);
    background-size: 50px 50px;
    pointer-events: none;
    z-index: 0;
}

/* ── hide default streamlit elements ── */
#MainMenu, footer, header { visibility: hidden; }
.block-container { padding-top: 1rem !important; }

/* ── title area ── */
.compiler-title {
    font-family: 'Orbitron', monospace;
    font-size: 2.8rem;
    font-weight: 900;
    background: linear-gradient(90deg, #00ffc8, #00aaff, #aa00ff);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    text-align: center;
    letter-spacing: 4px;
    margin-bottom: 0;
    text-shadow: none;
}

.compiler-subtitle {
    font-family: 'Rajdhani', sans-serif;
    font-size: 1.1rem;
    color: #4ecdc4;
    text-align: center;
    letter-spacing: 6px;
    text-transform: uppercase;
    margin-top: 0;
}

/* ── phase badges ── */
.phase-badge {
    display: inline-block;
    background: linear-gradient(135deg, #00ffc820, #00aaff20);
    border: 1px solid #00ffc840;
    border-radius: 20px;
    padding: 4px 14px;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.75rem;
    color: #00ffc8;
    margin: 2px;
    letter-spacing: 1px;
}

/* ── editor area ── */
.stTextArea textarea {
    background: #050d14 !important;
    border: 1px solid #00ffc840 !important;
    border-radius: 12px !important;
    color: #e0f7fa !important;
    font-family: 'JetBrains Mono', monospace !important;
    font-size: 0.9rem !important;
    line-height: 1.7 !important;
    padding: 16px !important;
}

.stTextArea textarea:focus {
    border-color: #00ffc8 !important;
    box-shadow: 0 0 20px #00ffc820 !important;
}

/* ── buttons ── */
.stButton > button {
    background: linear-gradient(135deg, #00ffc8, #00aaff) !important;
    color: #0a0e1a !important;
    border: none !important;
    border-radius: 10px !important;
    font-family: 'Orbitron', monospace !important;
    font-weight: 700 !important;
    font-size: 0.9rem !important;
    padding: 14px 32px !important;
    letter-spacing: 2px !important;
    width: 100% !important;
    transition: all 0.3s ease !important;
    cursor: pointer !important;
}

.stButton > button:hover {
    transform: translateY(-2px) !important;
    box-shadow: 0 8px 25px #00ffc840 !important;
}

/* ── phase output cards ── */
.phase-card {
    background: linear-gradient(135deg, #0d1b2a, #0a1420);
    border: 1px solid #00ffc820;
    border-radius: 16px;
    padding: 20px;
    margin-bottom: 16px;
    position: relative;
    overflow: hidden;
}

.phase-card::before {
    content: '';
    position: absolute;
    top: 0; left: 0;
    width: 4px; height: 100%;
    background: linear-gradient(180deg, #00ffc8, #00aaff);
    border-radius: 4px 0 0 4px;
}

.phase-title {
    font-family: 'Orbitron', monospace;
    font-size: 0.85rem;
    color: #00ffc8;
    letter-spacing: 3px;
    margin-bottom: 12px;
    text-transform: uppercase;
}

.phase-content {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.8rem;
    color: #a8d8ea;
    white-space: pre-wrap;
    line-height: 1.8;
    background: #05080f;
    border-radius: 8px;
    padding: 14px;
    border: 1px solid #ffffff08;
    max-height: 350px;
    overflow-y: auto;
}

/* ── stats cards ── */
.stat-card {
    background: linear-gradient(135deg, #0d1b2a, #0a1420);
    border: 1px solid #00aaff30;
    border-radius: 12px;
    padding: 16px;
    text-align: center;
    margin: 4px 0;
}

.stat-number {
    font-family: 'Orbitron', monospace;
    font-size: 2rem;
    font-weight: 900;
    background: linear-gradient(90deg, #00ffc8, #00aaff);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
}

.stat-label {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.8rem;
    color: #4ecdc4;
    letter-spacing: 2px;
    text-transform: uppercase;
}

/* ── success / error boxes ── */
.success-box {
    background: linear-gradient(135deg, #00ffc810, #00ff8810);
    border: 1px solid #00ffc840;
    border-radius: 10px;
    padding: 12px 16px;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.85rem;
    color: #00ffc8;
    margin: 8px 0;
}

.error-box {
    background: linear-gradient(135deg, #ff004410, #ff440010);
    border: 1px solid #ff004440;
    border-radius: 10px;
    padding: 12px 16px;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.85rem;
    color: #ff4444;
    margin: 8px 0;
}

.warning-box {
    background: linear-gradient(135deg, #ffaa0010, #ff880010);
    border: 1px solid #ffaa0040;
    border-radius: 10px;
    padding: 12px 16px;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.85rem;
    color: #ffaa00;
    margin: 8px 0;
}

/* ── divider ── */
.neon-divider {
    height: 1px;
    background: linear-gradient(90deg, transparent, #00ffc840, #00aaff40, transparent);
    margin: 24px 0;
}

/* ── scrollbar ── */
::-webkit-scrollbar { width: 6px; }
::-webkit-scrollbar-track { background: #050810; }
::-webkit-scrollbar-thumb { background: #00ffc840; border-radius: 3px; }

/* ── tabs ── */
.stTabs [data-baseweb="tab-list"] {
    background: #050d14 !important;
    border-radius: 12px !important;
    padding: 4px !important;
    border: 1px solid #00ffc820 !important;
    gap: 4px !important;
}

.stTabs [data-baseweb="tab"] {
    background: transparent !important;
    color: #4ecdc4 !important;
    font-family: 'Rajdhani', sans-serif !important;
    font-weight: 600 !important;
    font-size: 0.85rem !important;
    letter-spacing: 1px !important;
    border-radius: 8px !important;
    padding: 8px 16px !important;
}

.stTabs [aria-selected="true"] {
    background: linear-gradient(135deg, #00ffc820, #00aaff20) !important;
    color: #00ffc8 !important;
    border: 1px solid #00ffc840 !important;
}

/* ── select box ── */
.stSelectbox > div > div {
    background: #050d14 !important;
    border: 1px solid #00ffc840 !important;
    color: #e0f7fa !important;
    border-radius: 10px !important;
    font-family: 'JetBrains Mono', monospace !important;
}

/* ── section headers ── */
.section-header {
    font-family: 'Rajdhani', sans-serif;
    font-size: 1rem;
    color: #4ecdc4;
    letter-spacing: 3px;
    text-transform: uppercase;
    margin-bottom: 12px;
    padding-bottom: 8px;
    border-bottom: 1px solid #00ffc820;
}
</style>
""", unsafe_allow_html=True)

def run_compiler(source_code: str):
    """Write source to temp file, run compiler, return full output."""
    compiler_dir = os.path.dirname(os.path.abspath(__file__))
    compiler_exe = os.path.join(compiler_dir, "compiler.exe")

    if not os.path.exists(compiler_exe):
        compiler_exe = os.path.join(compiler_dir, "compiler")

    if not os.path.exists(compiler_exe):
        return None, "Compiler executable not found. Please run:\ngcc -o compiler main.c lexer.c parser.c semantic.c codegen.c optimizer.c target.c -lm"

    # ✅ FIX: Close the file BEFORE passing it to compiler (Windows file-lock issue)
    tmp = tempfile.NamedTemporaryFile(
        mode="w", suffix=".c", delete=False,
        dir=compiler_dir, prefix="minic_"
    )
    tmp.write(source_code)
    tmp.close()          # <-- CLOSE HERE before subprocess runs
    tmp_path = tmp.name

    try:
        # result = subprocess.run(
        #     [compiler_exe, tmp_path],
        #     capture_output=True, text=True, timeout=10,
        #     cwd=compiler_dir
        # )
        result = subprocess.run(
        [compiler_exe, tmp_path],
        capture_output=True,
        text=True,
        timeout=10,
        cwd=compiler_dir,
        encoding="utf-8",       # ← ADD THIS
        errors="replace"        # ← ADD THIS (won't crash on bad chars)
        )
        # 🔥 ADD DEBUG HERE
        stdout = result.stdout or ""
        stderr = result.stderr or ""

        print("DEBUG STDOUT:", stdout)
        print("DEBUG STDERR:", stderr)
        print("RETURN CODE:", result.returncode)

# 🔥 Then your logic
        # output = (result.stdout or "") + (result.stderr or "")
        # return output, None
        stdout = result.stdout or ""
        stderr = result.stderr or ""

        output = stdout + stderr

        # # 🔥 Detect lexical or compilation error
        # if "Lexical Error" in output or "Compilation stopped" in output:
        #     return None, output   # treat as error

        # also handle non-zero exit (important!)
        if result.returncode != 0:
            return None, stderr if stderr else stdout

        dot_file = os.path.join(compiler_dir, "tree.dot")
        png_file = os.path.join(compiler_dir, "tree.png")
        # Convert DOT → PNG
        if os.path.exists(dot_file):
            try:
                subprocess.run(["dot", "-Tpng", dot_file, "-o", png_file], check=True)
            except:
                pass
        
        
        # return stdout, None
        return output, png_file if os.path.exists(png_file) else None
    except subprocess.TimeoutExpired:
        return None, "Compiler timed out after 10 seconds."
    except Exception as e:
        return None, str(e)
    finally:
        try:
            os.unlink(tmp_path)
            asm_path = tmp_path.replace(".c", ".asm")
            if os.path.exists(asm_path):
                os.unlink(asm_path)
        except Exception:
            pass
# ── Helper: parse output into phases ─────────────────────────────────────────
def parse_phases(output: str):
    phases = {
        "Phase 1 — Lexical Analysis":     "",
        "Phase 2 — Syntax Analysis":      "",
        "Phase 3 — Semantic Analysis":    "",
        "Phase 4 — Code Generation":      "",
        "Phase 5 — Optimization":         "",
        "Phase 6 — Assembly Output":      "",
    }

    markers = [
        ("PHASE 1", "Phase 1 — Lexical Analysis"),
        ("PHASE 2", "Phase 2 — Syntax Analysis"),
        ("PHASE 3", "Phase 3 — Semantic Analysis"),
        ("PHASE 4", "Phase 4 — Code Generation"),
        ("PHASE 5", "Phase 5 — Optimization"),
        ("PHASE 6", "Phase 6 — Assembly Output"),
    ]

    lines = output.split("\n")
    current = None

    for line in lines:
        matched = False
        for marker, key in markers:
            #if marker in line:
            if marker in line.upper():
                current = key
                matched = True
                break
        if not matched and current:
            phases[current] += line + "\n"

    return phases


# ── Helper: extract token stats ───────────────────────────────────────────────
def extract_stats(phase1_text: str):
    stats = {
        "Total Tokens": 0,
        "Keywords": 0,
        "Identifiers": 0,
        "Int Literals": 0,
        "Float Literals": 0,
        "Operators": 0,
    }
    for line in phase1_text.split("\n"):
        for key in stats:
            if key in line:
                parts = line.split(":")
                if len(parts) == 2:
                    try:
                        stats[key] = int(parts[1].strip())
                    except ValueError:
                        pass
    return stats



# ── Sample programs ───────────────────────────────────────────────────────────
SAMPLES = {
    "Basic Arithmetic": """\
int main() {
    int x;
    int y;
    int sum;
    x = 10;
    y = 5;
    sum = x + y;
    print(sum);
    return 0;
}""",

    "If-Else Condition": """\
int main() {
    int x;
    int y;
    x = 10;
    y = 3;
    int sum;
    sum = x + y;
    if (sum > 10) {
        print(sum);
    } else {
        print(y);
    }
    return 0;
}""",

    "While Loop": """\
int main() {
    int i;
    i = 0;
    while (i < 5) {
        print(i);
        i = i + 1;
    }
    return 0;
}""",

    "Constant Folding Demo": """\
int main() {
    int a;
    int b;
    int c;
    int result;
    a = 6 * 7;
    b = 10 + 5;
    c = 100 - 50;
    result = a + b;
    print(result);
    print(c);
    return 0;
}""",

    "Full Demo (All Features)": """\
int main() {
    int x;
    int y;
    float z;
    x = 10;
    y = 3;
    z = 3.14;
    int sum;
    sum = x + y;
    if (sum > 10) {
        print(sum);
    } else {
        print(y);
    }
    int i;
    i = 0;
    while (i < 5) {
        print(i);
        i = i + 1;
    }
    int result;
    result = 2 + 3 * 4;
    print(result);
    return 0;
}""",
}


# ══════════════════════════════════════════════════════════════════════════════
#  MAIN UI
# ══════════════════════════════════════════════════════════════════════════════

# ── Header ────────────────────────────────────────────────────────────────────
st.markdown('<div class="compiler-title">MINI-C COMPILER</div>', unsafe_allow_html=True)
st.markdown('<div class="compiler-subtitle">All 6 Phases · Visual Pipeline · Real Assembly Output</div>', unsafe_allow_html=True)

# phase badges
badges_html = "".join([
    f'<span class="phase-badge">{p}</span>'
    for p in ["① Lexer", "② Parser", "③ Semantic", "④ TAC Gen", "⑤ Optimizer", "⑥ Assembly"]
])
st.markdown(f'<div style="text-align:center;margin:12px 0 24px">{badges_html}</div>', unsafe_allow_html=True)
st.markdown('<div class="neon-divider"></div>', unsafe_allow_html=True)

# ── Two-column layout ─────────────────────────────────────────────────────────
col_left, col_right = st.columns([1, 1.6], gap="large")

with col_left:
    st.markdown('<div class="section-header">📝 Source Code Editor</div>', unsafe_allow_html=True)

    # Sample selector
    sample_choice = st.selectbox(
        "Load a sample program:",
        ["-- Write your own --"] + list(SAMPLES.keys()),
        label_visibility="collapsed"
    )

    default_code = SAMPLES.get(sample_choice, "") if sample_choice != "-- Write your own --" else """\
int main() {
    int x;
    x = 10;
    print(x);
    return 0;
}"""

    source_code = st.text_area(
        "Source Code",
        value=default_code,
        height=380,
        label_visibility="collapsed",
        placeholder="Write your Mini-C code here..."
    )

    compile_btn = st.button("⚡  COMPILE NOW", use_container_width=True)

    # Language reference
    with st.expander("📖 Mini-C Language Reference"):
        st.markdown("""
<div style="font-family:'JetBrains Mono',monospace;font-size:0.8rem;color:#a8d8ea;line-height:2">
<b style="color:#00ffc8">Types:</b>     int, float<br>
<b style="color:#00ffc8">Keywords:</b>  if, else, while, return, print<br>
<b style="color:#00ffc8">Operators:</b> + - * /  ==  !=  &lt;  &gt;  &lt;=  &gt;=  =<br>
<b style="color:#00ffc8">Structure:</b> int main() { ... }<br><br>
<b style="color:#00ffc8">Example:</b><br>
int x;<br>
x = 2 + 3 * 4;<br>
if (x > 10) { print(x); }<br>
while (x > 0) { x = x - 1; }<br>
return 0;
</div>
""", unsafe_allow_html=True)


with col_right:
    st.markdown('<div class="section-header">🖥️ Compiler Output</div>', unsafe_allow_html=True)

    if compile_btn and source_code.strip():
        with st.spinner("Compiling..."):
            #output, error = run_compiler(source_code)
            # output, _ = run_compiler(source_code)
            output, tree_img = run_compiler(source_code)
            

        # 🔴 IF ERROR → SHOW AND STOP EVERYTHING
        # if error:
        #     st.markdown(f'<div class="error-box">❌ {error}</div>', unsafe_allow_html=True)
        #     st.stop()   # 🚀 IMPORTANT: stops further UI rendering
        # 🚨 Detect critical execution failure (like compiler crash)
        if output is None:
            st.markdown('<div class="error-box">❌ Compiler failed to run</div>', unsafe_allow_html=True)
            st.stop()

        # ✅ SUCCESS CASE ONLY
        if output:
            phases = parse_phases(output)
            stats  = extract_stats(phases["Phase 1 — Lexical Analysis"])

            # ── Status banner ──
            # has_error = (
            #     "[Parser Error]" in output or
            #     "[Semantic Error]" in output or
            #     "syntax error" in output.lower()
            # )
            # has_error = (
            #     "[Parser Error]" in output or
            #     "[Semantic Error]" in output or
            #     "[Lexer Error]" in output
            # )
            has_error = any([
                "[Parser Error]" in phases["Phase 2 — Syntax Analysis"],
                "[Semantic Error]" in phases["Phase 3 — Semantic Analysis"],
                "❌" in phases["Phase 1 — Lexical Analysis"]
            ])
            has_warning = (
                "[Warning]" in output or
                "Warning" in output
            ) and not has_error

            if has_error:
                st.markdown('<div class="error-box">❌ Compilation finished with errors</div>', unsafe_allow_html=True)
                st.stop()
            else:
                st.markdown('<div class="success-box">✅ Compilation successful — all 6 phases completed</div>', unsafe_allow_html=True)

            # if has_warning:
            #     st.markdown('<div class="warning-box">⚠️ Warnings detected — check Phase 3</div>', unsafe_allow_html=True)

            # ── Stats ──
            st.markdown('<div style="margin:16px 0 8px"><span style="font-family:Rajdhani,sans-serif;color:#4ecdc4;letter-spacing:2px;font-size:0.8rem;text-transform:uppercase">Token Statistics</span></div>', unsafe_allow_html=True)

            s1, s2, s3, s4, s5, s6 = st.columns(6)
            for col, (label, key) in zip(
                [s1, s2, s3, s4, s5, s6],
                [("Total", "Total Tokens"), ("KW", "Keywords"),
                 ("IDs", "Identifiers"), ("Int", "Int Literals"),
                 ("Float", "Float Literals"), ("Ops", "Operators")]
            ):
                with col:
                    st.markdown(f"""
<div class="stat-card">
  <div class="stat-number">{stats.get(key, 0)}</div>
  <div class="stat-label">{label}</div>
</div>""", unsafe_allow_html=True)

            st.markdown('<div class="neon-divider"></div>', unsafe_allow_html=True)

            tab_labels = ["① Lexer", "② Parser", "③ Semantic", "④ TAC", "⑤ Optimizer", "⑥ Assembly"]
            tabs = st.tabs(tab_labels)

            phase_keys = list(phases.keys())
            phase_icons = ["🔤", "🌳", "🔍", "📝", "⚡", "🖥️"]
            phase_colors = ["#00ffc8", "#00aaff", "#aa00ff", "#ff6600", "#ffcc00", "#ff0088"]

            for tab, key, icon, color in zip(tabs, phase_keys, phase_icons, phase_colors):
                with tab:
                    
                    content = phases[key].strip()
                      # 🔥 ADD THIS HERE
                    if key != "Phase 3 — Semantic Analysis" and "COMPILATION_STOPPED" in output:
                        st.markdown('<div class="warning-box">⚠️ Skipped due to semantic errors</div>', unsafe_allow_html=True)
                        continue

# 🔥 SPECIAL HANDLING FOR SEMANTIC PHASE
                    if key == "Phase 3 — Semantic Analysis":
                        semantic_output = extract_phase(content, "[SEMANTIC_START]", "[SEMANTIC_END]")

                        if semantic_output.strip():
                            content = semantic_output

                    if content:
                        st.markdown(f"""<div class="phase-card">
 <div class="phase-title" style="color:{color}">{icon} {key}</div>
  <div class="phase-content">{content}</div>
</div>""", unsafe_allow_html=True)

                        if key == "Phase 2 — Syntax Analysis" and tree_img:
                            st.image(tree_img)

                    else:
                          st.markdown("No output")
                
# ── Footer ────────────────────────────────────────────────────────────────────
st.markdown('<div class="neon-divider"></div>', unsafe_allow_html=True)
st.markdown("""
<div style="text-align:center;font-family:Rajdhani,sans-serif;color:#4ecdc440;font-size:0.8rem;letter-spacing:3px;padding:8px 0">
MINI-C COMPILER · 6-PHASE PIPELINE · BUILT WITH C + STREAMLIT
</div>
""", unsafe_allow_html=True)