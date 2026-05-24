import struct
import os

TICKET_FORMAT = "<qqqi256siiiii512s256s"
ENGINEER_FORMAT = "<i50sxx3i"

TICKET_SIZE = 1072
ENGINEER_SIZE = 68

TICKETS_DB_PATH = "c:\\newsdp2\\tickets.db"
ENGINEERS_DB_PATH = "c:\\newsdp2\\engineers.db"

def pad_str(s, length):
    encoded = s.encode("utf-8")[:length]
    return encoded + b"\x00" * (length - len(encoded))

def read_tickets():
    if not os.path.exists(TICKETS_DB_PATH):
        return []
    with open(TICKETS_DB_PATH, "rb") as f:
        header = f.read(4)
        if not header or len(header) < 4:
            return []
        count = struct.unpack("<i", header)[0]
        tickets = []
        for _ in range(count):
            data = f.read(TICKET_SIZE)
            if len(data) < TICKET_SIZE:
                break
            unpacked = struct.unpack(TICKET_FORMAT, data)
            t = {
                "timeCreated": unpacked[0],
                "timeAssigned": unpacked[1],
                "timeClosed": unpacked[2],
                "issueType": unpacked[3],
                "description": unpacked[4].decode("utf-8", errors="ignore").strip("\x00"),
                "id": unpacked[5],
                "uid": unpacked[6],
                "eid": unpacked[7],
                "priority": unpacked[8],
                "status": unpacked[9],
                "notes": unpacked[10].decode("utf-8", errors="ignore").strip("\x00"),
                "imagePath": unpacked[11].decode("utf-8", errors="ignore").strip("\x00"),
            }
            tickets.append(t)
        return tickets

def write_tickets(tickets):
    with open(TICKETS_DB_PATH, "wb") as f:
        f.write(struct.pack("<i", len(tickets)))
        for t in tickets:
            data = struct.pack(
                TICKET_FORMAT,
                t["timeCreated"],
                t["timeAssigned"],
                t["timeClosed"],
                t["issueType"],
                pad_str(t["description"], 256),
                t["id"],
                t["uid"],
                t["eid"],
                t["priority"],
                t["status"],
                pad_str(t["notes"], 512),
                pad_str(t["imagePath"], 256)
            )
            f.write(data)

def read_engineers():
    if not os.path.exists(ENGINEERS_DB_PATH):
        return []
    with open(ENGINEERS_DB_PATH, "rb") as f:
        header = f.read(4)
        if not header or len(header) < 4:
            return []
        count = struct.unpack("<i", header)[0]
        engineers = []
        for _ in range(count):
            data = f.read(ENGINEER_SIZE)
            if len(data) < ENGINEER_SIZE:
                break
            unpacked = struct.unpack(ENGINEER_FORMAT, data)
            e = {
                "id": unpacked[0],
                "name": unpacked[1].decode("utf-8", errors="ignore").strip("\x00"),
                "specialty": unpacked[2],
                "ticketsAssigned": unpacked[3],
                "ticketsResolved": unpacked[4]
            }
            engineers.append(e)
        return engineers

def write_engineers(engineers):
    with open(ENGINEERS_DB_PATH, "wb") as f:
        f.write(struct.pack("<i", len(engineers)))
        for e in engineers:
            data = struct.pack(
                ENGINEER_FORMAT,
                e["id"],
                pad_str(e["name"], 50),
                e["specialty"],
                e["ticketsAssigned"],
                e["ticketsResolved"]
            )
            f.write(data)

def delete_ticket_bst(ticket_id):
    """
    Deletes the ticket from tickets.db.
    If the ticket was assigned to an engineer, decrements their workload in engineers.db.
    """
    tickets = read_tickets()
    target_ticket = None
    new_tickets = []
    for t in tickets:
        if t["id"] == ticket_id:
            target_ticket = t
        else:
            new_tickets.append(t)
    
    if not target_ticket:
        raise ValueError(f"Ticket #{ticket_id} not found in database.")
    
    # Write back updated tickets
    write_tickets(new_tickets)
    
    # If the ticket was assigned, decrement engineer workload
    eid = target_ticket["eid"]
    if eid > 0:
        engineers = read_engineers()
        for e in engineers:
            if e["id"] == eid:
                if target_ticket["status"] in (3, 4):
                    if e["ticketsResolved"] > 0:
                        e["ticketsResolved"] -= 1
                else:
                    if e["ticketsAssigned"] > 0:
                        e["ticketsAssigned"] -= 1
                break
        write_engineers(engineers)
    
    return target_ticket

def reopen_ticket(ticket_id):
    """
    Reopens a closed/resolved ticket in tickets.db (status becomes ASSIGNED = 1).
    Adjusts the corresponding engineer's workload in engineers.db.
    """
    tickets = read_tickets()
    target_ticket = None
    for t in tickets:
        if t["id"] == ticket_id:
            target_ticket = t
            break
            
    if not target_ticket:
        raise ValueError(f"Ticket #{ticket_id} not found in database.")
        
    if target_ticket["status"] not in (3, 4):
        raise ValueError(f"Ticket #{ticket_id} is not resolved or closed.")
        
    # Reopen ticket: set status to ASSIGNED (1), timeClosed to 0
    target_ticket["status"] = 1
    target_ticket["timeClosed"] = 0
    
    write_tickets(tickets)
    
    # Revert engineer resolution count and restore assignment count
    eid = target_ticket["eid"]
    if eid > 0:
        engineers = read_engineers()
        for e in engineers:
            if e["id"] == eid:
                if e["ticketsResolved"] > 0:
                    e["ticketsResolved"] -= 1
                e["ticketsAssigned"] += 1
                break
        write_engineers(engineers)
        
    return target_ticket
