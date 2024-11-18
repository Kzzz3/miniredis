import sds;
import <iostream>;
import <cassert>;

void test_sds_creation_and_access() {
    const char* test_str = "Hello, SDS!";
    size_t len = std::strlen(test_str);

    // 创建 SDS 对象
    sds* s = sds::create(test_str, len);

    // 检查 SDS 对象的长度
    size_t actual_len = s->length();
    std::cout << "Length: " << actual_len << std::endl;
    assert(actual_len == len && "Length should match the input string length.");

    // 检查 SDS 对象的容量
    size_t actual_capacity = s->capacity();
    std::cout << "Capacity: " << actual_capacity << std::endl;
    assert(actual_capacity == len && "Capacity should be equal to the string length.");

    // 检查 SDS 对象的可用空间
    size_t available_space = s->available();
    std::cout << "Available space: " << available_space << std::endl;
    assert(available_space == 0 && "Available space should be zero after full allocation.");

    // 销毁 SDS 对象
    sds::destroy(s);
    assert(s == nullptr && "SDS pointer should be null after destruction.");
}

void test_sds_with_large_string() {
    const char* test_str = "This is a test string for large SDS allocation!";
    size_t len = std::strlen(test_str);

    // 创建 SDS 对象
    sds* s = sds::create(test_str, len);

    // 检查 SDS 对象的长度
    size_t actual_len = s->length();
    std::cout << "Length: " << actual_len << std::endl;
    assert(actual_len == len && "Length should match the input string length.");

    // 检查 SDS 对象的容量
    size_t actual_capacity = s->capacity();
    std::cout << "Capacity: " << actual_capacity << std::endl;
    assert(actual_capacity == len && "Capacity should be equal to the string length.");

    // 销毁 SDS 对象
    sds::destroy(s);
    assert(s == nullptr && "SDS pointer should be null after destruction.");
}

int main() {
    // 运行测试用例
    std::cout << "Running test: test_sds_creation_and_access\n";
    test_sds_creation_and_access();

    std::cout << "Running test: test_sds_with_large_string\n";
    test_sds_with_large_string();

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
