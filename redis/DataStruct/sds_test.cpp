import sds;  // ����sdsģ��
import std;
import <cassert>;

void test_sds_create_and_length() {
    const char* str = "Hello, SDS!";
    size_t len = std::strlen(str);

    // ����һ��sds����
    sds* my_sds = sds::create(str, len, 64);

    // ��鳤��
    assert(my_sds->length() == len);
    std::cout << "test_sds_create_and_length passed!" << std::endl;
}

void test_sds_capacity_and_available() {
    const char* str = "Test capacity";
    size_t len = std::strlen(str);
    size_t alloc = 32;

    // ����һ��sds����
    sds* my_sds = sds::create(str, len, alloc);

    // �������
    assert(my_sds->capacity() >= alloc);
    // �����ÿռ�
    assert(my_sds->available() >= (my_sds->capacity() - my_sds->length()));
    std::cout << "test_sds_capacity_and_available passed!" << std::endl;
}

void test_sds_append() {
    const char* str1 = "Hello";
    const char* str2 = " World!";

    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);

    // ������һ��sds����
    sds* my_sds = sds::create(str1, len1, 64);

    // ׷�ӵڶ����ַ���
    my_sds = my_sds->append(str2, len2);

    // ������ַ����ĳ���
    assert(my_sds->length() == len1 + len2);
    std::cout << "test_sds_append passed!" << std::endl;
}

void test_sds_copy() {
    const char* str1 = "Original string";

    size_t len1 = std::strlen(str1);

    // ������һ��sds����
    sds* my_sds = sds::create(str1, len1, 64);

    // ����sds����
    sds* copied_sds = my_sds->copy(my_sds);

    // ��鳤���Ƿ���ͬ
    assert(copied_sds->length() == my_sds->length());
    std::cout << "test_sds_copy passed!" << std::endl;
}

void test_sds_dilatation() {
    const char* str = "Short string";
    size_t len = std::strlen(str);

    // ����sds����
    sds* my_sds = sds::create(str, len, 32);

    // ��չsds����
    my_sds = my_sds->dilatation(20);

    // �����չ�������
    assert(my_sds->capacity() >= len + 20);
    std::cout << "test_sds_dilatation passed!" << std::endl;
}

void test_sds_edge_case() {
    const char* str = "Edge Case!";
    size_t len = std::strlen(str);

    // ����һ��sds����Ԥ��������Ϊ 0
    sds* my_sds = sds::create(str, len, 0);

    // ��鳤�Ⱥ�����
    assert(my_sds->length() == len);
    assert(my_sds->capacity() > 0);
    std::cout << "test_sds_edge_case passed!" << std::endl;
}

void test_sds_copy_empty_string() {
    const char* str = "";
    size_t len = std::strlen(str);

    // ����һ�����ַ�����sds����
    sds* my_sds = sds::create(str, len, 64);

    // ���ƿ��ַ���
    sds* copied_sds = my_sds->copy(my_sds);

    // ��鸴�ƺ��ַ����ĳ���
    assert(copied_sds->length() == 0);
    std::cout << "test_sds_copy_empty_string passed!" << std::endl;
}

void test_sds_append_empty_string() {
    const char* str1 = "Hello";
    const char* str2 = "";

    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);

    // ������һ��sds����
    sds* my_sds = sds::create(str1, len1, 64);

    // ׷�ӿ��ַ���
    my_sds = my_sds->append(str2, len2);

    // ����ַ������Ȳ���
    assert(my_sds->length() == len1);
    std::cout << "test_sds_append_empty_string passed!" << std::endl;
}

void test_sds_dilatation_large_addition() {
    const char* str = "Test dilatation";
    size_t len = std::strlen(str);

    // ����sds����
    sds* my_sds = sds::create(str, len, 32);

    // ��չ����
    my_sds = my_sds->dilatation(1000);

    // �����չ�������
    assert(my_sds->capacity() >= len + 1000);
    std::cout << "test_sds_dilatation_large_addition passed!" << std::endl;
}

int main() {
    try {
        // ִ�����в���
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
