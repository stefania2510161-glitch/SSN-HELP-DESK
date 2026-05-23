from datetime import datetime, timedelta
from flask import Blueprint, jsonify
from auth import login_required
from helpdesk_client import run_helpdesk

analytics_bp = Blueprint("analytics", __name__)


@analytics_bp.route("/api/analytics/resolutions")
@login_required(role="admin")
def api_resolutions_analytics():
    result = run_helpdesk("list_tickets", 0)
    if not result.get("success"):
        return jsonify(result), 500

    tickets = result.get("tickets", [])
    today = datetime.now().date()
    last_7_days = [(today - timedelta(days=i)).strftime('%Y-%m-%d') for i in range(6, -1, -1)]
    counts = {day: 0 for day in last_7_days}

    for t in tickets:
        if t.get("status") in ["Resolved", "Closed"] and t.get("closed_at") and t.get("closed_at") != "null":
            try:
                closed_date = t["closed_at"].split('T')[0]
                if closed_date in counts:
                    counts[closed_date] += 1
            except Exception:
                continue

    return jsonify({
        "success": True,
        "labels": last_7_days,
        "data": [counts[day] for day in last_7_days]
    })
