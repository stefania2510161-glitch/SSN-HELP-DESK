import heapq
import json
import subprocess
from config import BASE_DIR, HELPDESK_EXE

PROBLEM_DB = {
    "critical": {"score": 5, "note": "Urgent keyword detected"},
    "fire": {"score": 10, "note": "Safety Hazard / Emergency"},
    "smoke": {"score": 10, "note": "Safety Hazard / Emergency"},
    "leak": {"score": 7, "note": "Infrastructure Damage risk"},
    "exam": {"score": 8, "note": "Academic Impact"},
    "server": {"score": 6, "note": "Network wide disruption"},
    "wifi": {"score": 4, "note": "Connectivity issue"},
    "broken": {"score": 2, "note": "Furniture/Hardware repair"},
    "dead": {"score": 5, "note": "Total failure"},
    "smell": {"score": 6, "note": "Potential electrical hazard"}
}


def run_helpdesk(*args):
    if not os.path.isfile(HELPDESK_EXE):
        return {
            "success": False,
            "error": f"Compiled binary not found at '{HELPDESK_EXE}'. Run: gcc -o helpdesk helpdesk.c"
        }

    cmd = [HELPDESK_EXE] + [str(a) for a in args]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=10, cwd=BASE_DIR)
    except Exception as exc:
        return {"success": False, "error": str(exc)}

    stdout = result.stdout.strip()
    if not stdout:
        return {"success": False, "error": result.stderr.strip() or "No output from C binary."}

    try:
        return json.loads(stdout)
    except json.JSONDecodeError:
        return {"success": False, "error": f"Invalid JSON from binary: {stdout[:200]}"}


def apply_priorities(result, role):
    if not result.get("success"):
        return result

    if role == "admin":
        heap = []
        unassigned = []
        for t in result.get("tickets", []):
            if t["priority"] > 0:
                heapq.heappush(heap, (t["priority"], t["id"], t))
            else:
                unassigned.append(t)

        ordered_tickets = []
        while heap:
            ordered_tickets.append(heapq.heappop(heap)[2])
        ordered_tickets.extend(unassigned)
        result["tickets"] = ordered_tickets

    return result
