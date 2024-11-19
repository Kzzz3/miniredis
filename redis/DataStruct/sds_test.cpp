import sds;  // 导入sds模块
import std;
import <cassert>;

void test_sds_create_and_length() {
    const char* str = "Hello, SDS!";
    size_t len = std::strlen(str);

    // 创建一个sds对象
    sds* my_sds = sds::create(str, len, 64);

    // 检查长度
    assert(my_sds->length() == len);
    std::cout << "test_sds_create_and_length passed!" << std::endl;
}

void test_sds_capacity_and_available() {
    const char* str = "Test capacity";
    size_t len = std::strlen(str);
    size_t alloc = 32;

    // 创建一个sds对象
    sds* my_sds = sds::create(str, len, alloc);

    // 检查容量
    assert(my_sds->capacity() >= alloc);
    // 检查可用空间
    assert(my_sds->available() >= (my_sds->capacity() - my_sds->length()));
    std::cout << "test_sds_capacity_and_available passed!" << std::endl;
}

void test_sds_append() {
    const char* str1 = "Hello";
    const char* str2 = " World!";

    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);

    // 创建第一个sds对象
    sds* my_sds = sds::create(str1, len1, 64);

    // 追加第二个字符串
    my_sds = my_sds->append(str2, len2);

    // 检查结果字符串的长度
    assert(my_sds->length() == len1 + len2);
    std::cout << "test_sds_append passed!" << std::endl;
}

void test_sds_copy() {
    const char* str1 = "Original string";

    size_t len1 = std::strlen(str1);

    // 创建第一个sds对象
    sds* my_sds = sds::create(str1, len1, 64);

    // 复制sds对象
    sds* copied_sds = my_sds->copy(my_sds);

    // 检查长度是否相同
    assert(copied_sds->length() == my_sds->length());
    std::cout << "test_sds_copy passed!" << std::endl;
}

void test_sds_dilatation() {
    const char* str = "Short string";
    size_t len = std::strlen(str);

    // 创建sds对象
    sds* my_sds = sds::create(str, len, 32);

    // 扩展sds对象
    my_sds = my_sds->dilatation(20);

    // 检查扩展后的容量
    assert(my_sds->capacity() >= len + 20);
    std::cout << "test_sds_dilatation passed!" << std::endl;
}

void test_sds_edge_case() {
    const char* str = "Edge Case!";
    size_t len = std::strlen(str);

    // 创建一个sds对象，预分配容量为 0
    sds* my_sds = sds::create(str, len, 0);

    // 检查长度和容量
    assert(my_sds->length() == len);
    assert(my_sds->capacity() > 0);
    std::cout << "test_sds_edge_case passed!" << std::endl;
}

void test_sds_copy_empty_string() {
    const char* str = "";
    size_t len = std::strlen(str);

    // 创建一个空字符串的sds对象
    sds* my_sds = sds::create(str, len, 64);

    // 复制空字符串
    sds* copied_sds = my_sds->copy(my_sds);

    // 检查复制后字符串的长度
    assert(copied_sds->length() == 0);
    std::cout << "test_sds_copy_empty_string passed!" << std::endl;
}

void test_sds_append_empty_string() {
    const char* str1 = "Hello";
    const char* str2 = "";

    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);

    // 创建第一个sds对象
    sds* my_sds = sds::create(str1, len1, 64);

    // 追加空字符串
    my_sds = my_sds->append(str2, len2);

    // 检查字符串长度不变
    assert(my_sds->length() == len1);
    std::cout << "test_sds_append_empty_string passed!" << std::endl;
}

void test_sds_dilatation_large_addition() {
    const char* str = "Test dilatation";
    size_t len = std::strlen(str);

    // 创建sds对象
    sds* my_sds = sds::create(str, len, 32);

    // 扩展容量
    my_sds = my_sds->dilatation(1000);

    // 检查扩展后的容量
    assert(my_sds->capacity() >= len + 1000);
    std::cout << "test_sds_dilatation_large_addition passed!" << std::endl;
}

int main() {
    try {
        // 执行所有测试
        test_sds_create_and_length();
        test_sds_capacity_and_available();
        test_sds_append();
        test_sds_copy();
        test_sds_dilatation();
        test_sds_edge_case();
        test_sds_copy_empty_string();
        test_sds_append_empty_string();
        test_sds_dilatation_large_addition();

        std::cout << "All tests passed!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
