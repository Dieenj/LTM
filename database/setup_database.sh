#!/bin/bash

echo "========================================="
echo "FILE MANAGEMENT - Database Setup"
echo "========================================="
echo ""

read -p "Enter MySQL root password: " -s MYSQL_ROOT_PASS
echo ""

echo "Running database setup..."
mysql -u root -p"$MYSQL_ROOT_PASS" < schema.sql

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Database setup complete!"
    echo ""
    echo "Default users created:"
    echo "  - admin:123456 (2GB storage)"
    echo "  - guest:guest (1GB storage)"
    echo "  - dien:123456 (10GB storage)"
else
    echo "✗ Database setup failed!"
    exit 1
fi
