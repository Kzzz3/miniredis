import std;
import sds;

int main()
{
	sds s;
	s.init("hello", 5);
	std::cout << std::format("length: {}, capacity: {}, available: {}\n", s.length(), s.capacity(), s.available());

	s.free();
	std::cout << std::format("length: {}, capacity: {}, available: {}\n", s.length(), s.capacity(), s.available());
}
