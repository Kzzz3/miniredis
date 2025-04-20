# miniredis

[![Logo Placeholder](placeholder_logo.png)](link_to_your_project_website_or_repo)  <!-- Replace with your actual logo -->

## Project Description

`miniredis` is a simplified implementation of the Redis in-memory data store written in C++.  It is designed as a learning project to understand the core functionalities of Redis, including its command processing, data structures, and networking aspects. This implementation provides a subset of Redis commands and data structures, focusing on essential features.

**Note:** This is a simplified implementation for educational purposes and may not include all features or production-level robustness of a full-fledged Redis server.

## Features

- **Data Structures:**
    - Strings
    - Lists (Linked List)
    - Sets (IntSet/Hashtable)
    - Sorted Sets (RBTree)
    - Hashes (Hashtable)
- **Supported Commands (Subset):**
    - **String Commands:** `SET`, `GET`
    - **List Commands:** `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LRANGE`
    - **Set Commands:** `SADD`, `SREM`, `SMEMBERS`
    - **Sorted Set Commands:** `ZADD`, `ZRANGE`, `ZREVRANGE`
    - **Hash Commands:** `HSET`, `HGET`, `HGETALL`
    - **General Commands:** `PING`, `ECHO`

## Build Instructions

1. **Clone the repository:**
   