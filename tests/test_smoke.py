import pytest

from app import app
from helpdesk_client import run_helpdesk


def test_flask_index():
    client = app.test_client()
    r = client.get('/')
    assert r.status_code == 200


def test_helpdesk_list_tickets():
    res = run_helpdesk('list_tickets', 0)
    assert isinstance(res, dict)
    assert 'success' in res
