#pragma once
#include <zlib.h>

#include <vector>
#include <chrono>
#include <fstream>
#include <optional>
#include <charconv>
#include <filesystem>

using std::optional;

template <typename T> constexpr std::optional<T> str2num(const char* str, size_t len)
{
    static_assert(std::is_arithmetic_v<T>, "T must be a numeric type");

    if (str == nullptr || len == 0)
    {
        return std::nullopt;
    }

    T value;
    std::from_chars_result result = std::from_chars(str, str + len, value);

    if (result.ec == std::errc{} && result.ptr == str + len)
    {
        return value;
    }

    return std::nullopt;
}

inline uint32_t GetSecTimestamp()
{
    return std::time(nullptr);
}

inline void CompressFileStream(const std::string& inputFile, const std::string& outputFile,
                               size_t chunkSize = 1024 * 1024 * 10)
{
    // 如果输出文件存在则删除
    if (std::filesystem::exists(outputFile))
    {
        std::filesystem::remove(outputFile);
    }

    std::ifstream in(inputFile, std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("Failed to open input file: " + inputFile);
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Failed to open output file: " + outputFile);
    }

    // 写入原始文件名长度和内容
    uint32_t nameLength = inputFile.size();
    out.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    out.write(inputFile.c_str(), nameLength);

    // 初始化 zlib 的压缩流
    z_stream zstrm{};
    deflateInit(&zstrm, Z_BEST_COMPRESSION);

    std::vector<char> inBuffer(chunkSize);
    std::vector<char> outBuffer(chunkSize);

    int flush = Z_NO_FLUSH;
    do
    {
        in.read(inBuffer.data(), chunkSize);
        zstrm.avail_in = in.gcount();
        zstrm.next_in = reinterpret_cast<Bytef*>(inBuffer.data());

        flush = in.eof() ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            zstrm.avail_out = chunkSize;
            zstrm.next_out = reinterpret_cast<Bytef*>(outBuffer.data());

            deflate(&zstrm, flush);
            size_t writeSize = chunkSize - zstrm.avail_out;
            out.write(outBuffer.data(), writeSize);
        } while (zstrm.avail_out == 0);

    } while (flush != Z_FINISH);

    deflateEnd(&zstrm);
}

inline void DecompressFileStream(const std::string& inputFile, const std::string& outputFile,
                                 size_t chunkSize = 1024 * 1024 * 10)
{
    // 如果输出文件存在则删除
    if (std::filesystem::exists(outputFile))
    {
        std::filesystem::remove(outputFile);
    }

    std::ifstream in(inputFile, std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("open input file failed");
    }

    // Read the length and content of the original file name
    uint32_t nameLength = 0;
    in.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
    std::string originalFileName(nameLength, '\0');
    in.read(originalFileName.data(), nameLength);

    // Modify: Use the specified output file name
    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("open output file failed");
    }

    // Initialize zlib's decompression stream
    z_stream zstrm{};
    inflateInit(&zstrm);

    std::vector<char> inBuffer(chunkSize);
    std::vector<char> outBuffer(chunkSize);

    int ret = Z_OK;
    do
    {
        in.read(inBuffer.data(), chunkSize);
        zstrm.avail_in = in.gcount();
        if (zstrm.avail_in == 0)
        {
            break;
        }
        zstrm.next_in = reinterpret_cast<Bytef*>(inBuffer.data());

        do
        {
            zstrm.avail_out = chunkSize;
            zstrm.next_out = reinterpret_cast<Bytef*>(outBuffer.data());

            ret = inflate(&zstrm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR)
            {
                throw std::runtime_error("decompression failed");
            }

            size_t writeSize = chunkSize - zstrm.avail_out;
            out.write(outBuffer.data(), writeSize);
        } while (zstrm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    inflateEnd(&zstrm);
}
