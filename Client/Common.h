#ifndef COMMON_H
#define COMMON_H

#include <QString>

// Constants
const int CHUNK_SIZE = 8 * 1024; // 8KB
const int METADATA_PORT = 12345;
const QString CHUNK_SERVER_IP = "192.168.1.100"; // Replace with second laptop's IP
const int FIRST_CHUNK_PORT = 12346;
const int NUM_CHUNK_SERVERS = 15;

#endif // COMMON_H
