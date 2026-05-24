from flask import Flask
from flask_wtf.csrf import CSRFProtect

from auth import auth_bp
from analytics import analytics_bp
from dashboard_routes import dashboard_bp
from ticket_routes import ticket_bp

app = Flask(__name__)
app.secret_key = "ssn-helpdesk-secret-2024"

csrf = CSRFProtect(app)
app.register_blueprint(dashboard_bp)
app.register_blueprint(auth_bp)
app.register_blueprint(ticket_bp)
app.register_blueprint(analytics_bp)

if __name__ == "__main__":
    app.run(debug=True, port=5000)
