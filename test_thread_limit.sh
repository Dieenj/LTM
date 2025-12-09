#!/bin/bash
# Script test giới hạn DedicatedThread

echo "=== Test Thread Limit ==="
echo "Tạo 55 file upload đồng thời (vượt MAX_DEDICATED_THREADS=50)"
echo ""

# Tạo file test nhỏ
echo "Creating test files..."
for i in {1..55}; do
    echo "Test file $i" > /tmp/test_file_$i.txt
done

echo "Files created. Bây giờ chạy server và client để test:"
echo ""
echo "Terminal 1: ./run_server.sh"
echo "Terminal 2: Watch server logs - should see '503 Service temporarily unavailable' after 50 concurrent uploads"
echo ""
echo "Monitor sẽ hiển thị:"
echo "  [Monitor] ⚠️  Cannot create DedicatedThread: limit reached (50/50)"
echo "  [Worker] Rejected upload request: system overloaded"
echo ""
echo "Cleanup: rm /tmp/test_file_*.txt"
