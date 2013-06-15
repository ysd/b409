#/bin/bash
rm /mnt/cache/*
rm /mnt/index/*
make clean
make dedup_test
exit 0
