import json
import os
import shutil
import subprocess
from pathlib import Path

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


def test_helpdesk_normalizes_to_single_ravi_engineer(tmp_path):
    root = Path(__file__).resolve().parents[1]
    binary_name = 'helpdesk.exe' if os.name == 'nt' else 'helpdesk'
    target_dir = tmp_path / 'helpdesk-state'
    target_dir.mkdir()

    for filename in (binary_name, 'engineers.db', 'tickets.db'):
        shutil.copy2(root / filename, target_dir / filename)

    result = subprocess.run(
        [str(target_dir / binary_name), 'list_engineers'],
        cwd=target_dir,
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 0
    engineers = json.loads(result.stdout)['engineers']
    assert len(engineers) == 1
    assert engineers[0]['id'] == 101
    assert engineers[0]['name'] == 'Ravi Kumar'

    tickets_result = subprocess.run(
        [str(target_dir / binary_name), 'list_tickets'],
        cwd=target_dir,
        capture_output=True,
        text=True,
        check=False,
    )

    assert tickets_result.returncode == 0
    tickets = json.loads(tickets_result.stdout)['tickets']
    assert tickets
    assert all(ticket['engineer_id'] == 101 for ticket in tickets)
