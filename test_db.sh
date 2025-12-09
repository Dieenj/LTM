#!/bin/bash
# Test database connection

echo "=== Testing Database Connection ==="
echo "show tables;" | sudo mysql file_management

echo ""
echo "=== Users in database ==="
echo "SELECT user_id, username, storage_limit_bytes FROM USERS;" | sudo mysql file_management

echo ""
echo "=== Files in database ==="
echo "SELECT file_id, name, size_bytes, owner_id FROM FILES;" | sudo mysql file_management
