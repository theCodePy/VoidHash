import hashlib
import random

hashes = []
HASH_COUNT = 5

file = open("../WordLists/rockyou.txt", 'rb').readlines()
for i in range(HASH_COUNT):
    hashes.append(hashlib.md5(file[- random.randint(5000, 10000)].strip(b'\n')).hexdigest())

for i in range(HASH_COUNT):
    hashes.append(hashlib.sha1(file[- random.randint(5000, 10000)].strip(b'\n')).hexdigest())

for i in range(HASH_COUNT):
    hashes.append(hashlib.sha256(file[- random.randint(5000, 10000)].strip(b'\n')).hexdigest())

for i in range(HASH_COUNT):
    hashes.append(hashlib.sha512(file[- random.randint(5000, 10000)].strip(b'\n')).hexdigest())

for i in range(HASH_COUNT):
    hashes.append(hashlib.sha384(file[- random.randint(5000, 10000)].strip(b'\n')).hexdigest())

hashFile = open("hash.txt", 'w')
for i in hashes:
    hashFile.write(i+'\n')

hashFile.close()