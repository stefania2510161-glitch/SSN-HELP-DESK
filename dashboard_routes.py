from flask import Blueprint, redirect, render_template, session, url_for

dashboard_bp = Blueprint("dashboard", __name__)


@dashboard_bp.route("/")
def index():
    return render_template("index.html")


@dashboard_bp.route("/user-dashboard")
def user_dashboard():
    if "user_id" not in session or session.get("role") != "user":
        return redirect(url_for("dashboard.index"))
    return render_template("user-dashboard.html")


@dashboard_bp.route("/admin-dashboard")
def admin_dashboard():
    if "user_id" not in session or session.get("role") != "admin":
        return redirect(url_for("dashboard.index"))
    return render_template("admin-dashboard.html")


@dashboard_bp.route("/engineer-dashboard")
def engineer_dashboard():
    if "user_id" not in session or session.get("role") != "engineer":
        return redirect(url_for("dashboard.index"))
    return render_template("engineer-dashboard.html")


@dashboard_bp.route("/middleman-dashboard")
def middleman_dashboard():
    if "user_id" not in session or session.get("role") != "middleman":
        return redirect(url_for("dashboard.index"))
    return render_template("middleman-dashboard.html")
