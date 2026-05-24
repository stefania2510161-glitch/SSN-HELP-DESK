import os
import secrets
import json
import time
from datetime import datetime
from flask import Blueprint, jsonify, request, session
from PIL import Image
from auth import login_required
from config import MAX_IMAGE_DIMENSION, JPEG_QUALITY, BASE_DIR
from helpdesk_client import PROBLEM_DB, apply_priorities, run_helpdesk

ticket_bp = Blueprint("ticket", __name__)

UNDO_HISTORY_FILE = "undo_history.json"

def push_undo_action(user_id, action, ticket_id):
    history = {}
    if os.path.exists(UNDO_HISTORY_FILE):
        try:
            with open(UNDO_HISTORY_FILE, "r") as f:
                history = json.load(f)
        except Exception:
            pass
    u_str = str(user_id)
    if u_str not in history:
        history[u_str] = []
    history[u_str].append({
        "action": action,
        "ticket_id": int(ticket_id),
        "timestamp": time.time()
    })
    history[u_str] = history[u_str][-10:]
    try:
        with open(UNDO_HISTORY_FILE, "w") as f:
            json.dump(history, f, indent=2)
    except Exception as e:
        print(f"Error saving undo history: {e}")

def pop_undo_action(user_id):
    if not os.path.exists(UNDO_HISTORY_FILE):
        return None
    try:
        with open(UNDO_HISTORY_FILE, "r") as f:
            history = json.load(f)
    except Exception:
        return None
    u_str = str(user_id)
    if u_str not in history or not history[u_str]:
        return None
    action_item = history[u_str].pop()
    try:
        with open(UNDO_HISTORY_FILE, "w") as f:
            json.dump(history, f, indent=2)
    except Exception as e:
        print(f"Error updating undo history: {e}")
    return action_item


@ticket_bp.route("/api/tickets", methods=["GET"])
@login_required()
def api_get_all_tickets():
    role = session.get("role")
    result = run_helpdesk("list_tickets", 0)
    result = apply_priorities(result, role)
    return jsonify(result), 200 if result.get("success") else 500


@ticket_bp.route("/api/tickets/<int:user_id>", methods=["GET"])
@login_required()
def api_get_tickets(user_id):
    role = session.get("role")
    result = run_helpdesk("list_tickets", user_id)
    result = apply_priorities(result, role)
    return jsonify(result), 200 if result.get("success") else 500


@ticket_bp.route("/api/search_bst", methods=["GET"])
@login_required()
def api_search_bst():
    ticket_id = request.args.get("ticket_id")
    if not ticket_id:
        return jsonify({"success": False, "error": "Missing ticket_id"}), 400
    result = run_helpdesk("search_bst", ticket_id)
    return jsonify(result), 200 if result.get("success") else 500


@ticket_bp.route("/api/create_ticket", methods=["POST"])
@login_required()
def api_create_ticket():
    if session.get("role") == "middleman":
        return jsonify({"success": False, "error": "Middlemen are not authorized to raise tickets"}), 403

    if request.is_json:
        data = request.get_json()
        file = None
    else:
        data = request.form
        file = request.files.get("image")

    original_description = str(data.get("description", "")).strip()
    desc_lower = original_description.lower()
    issue_type = str(data.get("issue_type", "")).strip()

    if not issue_type or not original_description:
        return jsonify({"success": False, "error": "Missing fields"}), 400

    image_path = "null"
    if file and file.filename != "":
        try:
            upload_dir = os.path.join(BASE_DIR, "static", "uploads")
            os.makedirs(upload_dir, exist_ok=True)

            random_hex = secrets.token_hex(4)
            unique_filename = f"ticket_{int(datetime.now().timestamp())}_{random_hex}.jpg"
            full_save_path = os.path.join(upload_dir, unique_filename)

            img = Image.open(file)
            resample_filter = getattr(Image, "Resampling", Image).LANCZOS
            img.thumbnail((MAX_IMAGE_DIMENSION, MAX_IMAGE_DIMENSION), resample_filter)
            if img.mode != "RGB":
                img = img.convert("RGB")
            img.save(full_save_path, format="JPEG", quality=JPEG_QUALITY)
            image_path = f"static/uploads/{unique_filename}"
        except Exception as e:
            print(f"Error processing image: {e}")

    total_score = 0
    notes = []
    for word, impact in PROBLEM_DB.items():
        if word in desc_lower:
            total_score += impact["score"]
            if impact["note"] not in notes:
                notes.append(impact["note"])

    inferred_prio = 5
    if total_score >= 10:
        inferred_prio = 1
    elif total_score >= 7:
        inferred_prio = 2
    elif total_score >= 4:
        inferred_prio = 3

    brief_note = " | ".join(notes) if notes else "Routine check"
    res = run_helpdesk("create_ticket", session["user_id"], issue_type, original_description, inferred_prio, brief_note, image_path)
    if not res.get("success"):
        return jsonify(res), 500

    ticket_id = res["ticket"]["id"]
    assign_res = run_helpdesk("auto_assign_ticket", ticket_id)
    
    # Push to undo history stack
    push_undo_action(session["user_id"], "create_ticket", ticket_id)
    
    return jsonify({"success": True, "ticket": assign_res.get("ticket", res["ticket"]), "inferred_note": brief_note}), 200


@ticket_bp.route("/api/edit_ticket", methods=["POST"])
@login_required()
def api_edit_ticket():
    data = request.get_json(silent=True) or {}
    ticket_id = data.get("ticket_id")
    new_description = data.get("description")
    result = run_helpdesk("edit_ticket", session["user_id"], ticket_id, new_description)
    return jsonify(result), 200 if result.get("success") else 400


@ticket_bp.route("/api/delete_ticket", methods=["POST"])
@login_required()
def api_delete_ticket():
    data = request.get_json(silent=True) or {}
    ticket_id = data.get("ticket_id")
    result = run_helpdesk("delete_ticket", session["user_id"], ticket_id)
    return jsonify(result), 200 if result.get("success") else 400


@ticket_bp.route("/api/close_ticket", methods=["POST"])
@login_required()
def api_close_ticket():
    data = request.get_json(silent=True) or {}
    ticket_id = int(data.get("ticket_id", 0))
    result = run_helpdesk("close_ticket", ticket_id)
    if result.get("success"):
        push_undo_action(session["user_id"], "close_ticket", ticket_id)
    return jsonify(result), 200 if result.get("success") else 400


@ticket_bp.route("/api/assign_ticket", methods=["POST"])
@login_required()
def api_assign_ticket():
    if session.get("role") not in ["admin", "middleman"]:
        return jsonify({"success": False, "error": "Forbidden"}), 403

    data = request.get_json(silent=True) or {}
    ticket_id = int(data.get("ticket_id", 0))
    engineer_id = data.get("engineer_id")
    args = ["assign_ticket", ticket_id]
    if engineer_id:
        args.append(engineer_id)
    result = run_helpdesk(*args)
    return jsonify(result), 200 if result.get("success") else 400


@ticket_bp.route("/api/set_priority", methods=["POST"])
@login_required(role="middleman")
def api_set_priority():
    data = request.get_json(silent=True) or {}
    ticket_id = int(data.get("ticket_id", 0))
    priority = int(data.get("priority", 0))
    if not (1 <= priority <= 5):
        return jsonify({"success": False, "error": "Priority must be between 1 and 5"}), 400

    result = run_helpdesk("set_priority", ticket_id, priority)
    return jsonify(result), 200 if result.get("success") else 400


@ticket_bp.route("/api/engineers", methods=["GET"])
@login_required()
def api_get_engineers():
    result = run_helpdesk("list_engineers")
    return jsonify(result), 200 if result.get("success") else 500


@ticket_bp.route("/api/undo", methods=["POST"])
@login_required()
def api_undo():
    user_id = session.get("user_id")
    action_item = pop_undo_action(user_id)
    if not action_item:
        return jsonify({"success": False, "error": "No actions to undo"}), 400
        
    time_limit = 300
    if time.time() - action_item["timestamp"] > time_limit:
        return jsonify({"success": False, "error": "Undo window has expired (5-minute limit)"}), 400
        
    ticket_id = action_item["ticket_id"]
    action = action_item["action"]
    
    try:
        import db_helper
        if action == "create_ticket":
            db_helper.delete_ticket_bst(ticket_id)
            return jsonify({"success": True, "message": f"Creation of Ticket #{ticket_id} has been undone"}), 200
        elif action == "close_ticket":
            db_helper.reopen_ticket(ticket_id)
            return jsonify({"success": True, "message": f"Closure of Ticket #{ticket_id} has been undone"}), 200
        else:
            return jsonify({"success": False, "error": f"Unknown undo action: {action}"}), 400
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500


@ticket_bp.route("/api/undo/status", methods=["GET"])
@login_required()
def api_undo_status():
    user_id = session.get("user_id")
    if not os.path.exists(UNDO_HISTORY_FILE):
        return jsonify({"success": True, "has_undo": False}), 200
    try:
        with open(UNDO_HISTORY_FILE, "r") as f:
            history = json.load(f)
    except Exception:
        return jsonify({"success": True, "has_undo": False}), 200
        
    u_str = str(user_id)
    if u_str not in history or not history[u_str]:
        return jsonify({"success": True, "has_undo": False}), 200
        
    last_item = history[u_str][-1]
    time_limit = 300
    is_valid = (time.time() - last_item["timestamp"]) <= time_limit
    
    return jsonify({
        "success": True,
        "has_undo": is_valid,
        "last_action": last_item["action"],
        "ticket_id": last_item["ticket_id"]
    }), 200
