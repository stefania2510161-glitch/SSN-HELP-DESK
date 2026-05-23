import json
import os
from functools import wraps
from flask import Blueprint, jsonify, request, session
from werkzeug.security import check_password_hash, generate_password_hash
from config import BASE_DIR

auth_bp = Blueprint("auth", __name__)


def login_required(role=None):
    def decorator(fn):
        @wraps(fn)
        def wrapper(*args, **kwargs):
            if "user_id" not in session:
                return jsonify({"success": False, "error": "Not logged in"}), 401
            if role and session.get("role") != role:
                return jsonify({"success": False, "error": "Forbidden"}), 403
            return fn(*args, **kwargs)
        return wrapper
    return decorator


def is_password_hashed(password):
    return isinstance(password, str) and (
        password.startswith("pbkdf2:") or password.startswith("bcrypt:") or password.startswith("scrypt:")
    )


def get_users():
    pf = os.path.join(BASE_DIR, "users.json")
    if os.path.exists(pf):
        try:
            with open(pf, "r") as f:
                users = json.load(f)
            migrated = False
            for user_data in users.values():
                pw = user_data.get("password", "")
                if pw and not is_password_hashed(pw):
                    user_data["password"] = generate_password_hash(pw)
                    migrated = True
            if migrated:
                save_users(users)
            return users
        except Exception:
            pass

    default_users = {
        "user": {"user_id": 1, "password": generate_password_hash("pass"), "full_name": "Normal User", "role": "user"},
        "admin": {"user_id": 2, "password": generate_password_hash("pass"), "full_name": "Administrator", "role": "admin"},
        "middleman": {"user_id": 3, "password": generate_password_hash("pass"), "full_name": "Ticket Middleman", "role": "middleman"},
        "ravi": {"user_id": 101, "password": generate_password_hash("pass"), "full_name": "Ravi Kumar", "role": "engineer"}
    }
    save_users(default_users)
    return default_users


def save_users(users_dict):
    pf = os.path.join(BASE_DIR, "users.json")
    with open(pf, "w") as f:
        json.dump(users_dict, f, indent=2)


@auth_bp.route("/api/signup", methods=["POST"])
def api_signup():
    data = request.get_json(silent=True) or {}
    username = str(data.get("username", "")).strip()
    password = str(data.get("password", "")).strip()
    full_name = str(data.get("full_name", "")).strip()
    role = str(data.get("role", "")).strip()

    if not username or not password or not full_name or not role:
        return jsonify({"success": False, "error": "All fields are required"}), 400

    if role not in ["user", "middleman", "admin", "engineer"]:
        return jsonify({"success": False, "error": "Invalid role specified"}), 400

    users = get_users()
    if username in users:
        return jsonify({"success": False, "error": "Username already exists"}), 400

    new_id = max([u["user_id"] for u in users.values()] + [0]) + 1
    users[username] = {
        "user_id": new_id,
        "password": generate_password_hash(password),
        "full_name": full_name,
        "role": role
    }
    save_users(users)

    session["user_id"] = new_id
    session["username"] = username
    session["full_name"] = full_name
    session["role"] = role

    redirect_map = {"user": "/user-dashboard", "admin": "/admin-dashboard", "middleman": "/middleman-dashboard", "engineer": "/engineer-dashboard"}
    return jsonify({
        "success": True,
        "redirect": redirect_map.get(role, "/"),
        "user_id": new_id,
        "username": username,
        "full_name": full_name,
        "role": role
    }), 200


@auth_bp.route("/api/login", methods=["POST"])
def api_login():
    data = request.get_json(silent=True) or {}
    username = str(data.get("username", "")).strip()
    password = str(data.get("password", "")).strip()
    role = str(data.get("role", "")).strip()

    if not username or not password or not role:
        return jsonify({"success": False, "error": "Credentials and role required"}), 400

    users = get_users()
    if username not in users or not check_password_hash(users[username]["password"], password):
        return jsonify({"success": False, "error": "Invalid credentials"}), 401

    if users[username]["role"] != role:
        return jsonify({"success": False, "error": f"Cannot login to {role.capitalize()} portal with {users[username]['role']} credentials"}), 403

    u = users[username]
    session["user_id"] = u["user_id"]
    session["username"] = username
    session["full_name"] = u["full_name"]
    session["role"] = u["role"]

    redirect_map = {"user": "/user-dashboard", "admin": "/admin-dashboard", "middleman": "/middleman-dashboard", "engineer": "/engineer-dashboard"}
    return jsonify({
        "success": True,
        "redirect": redirect_map.get(role, "/"),
        "user_id": u["user_id"],
        "username": username,
        "full_name": u["full_name"],
        "role": u["role"]
    }), 200


@auth_bp.route("/api/logout", methods=["POST"])
def api_logout():
    session.clear()
    return jsonify({"success": True})
