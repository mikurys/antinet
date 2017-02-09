// #include <iostream>
#include <cstdint>
#include <memory>


int main_test() {
	volatile uint8_t * output = reinterpret_cast<uint8_t*>(malloc(1));
	volatile uint8_t * nope = reinterpret_cast<uint8_t*>(malloc(1)), *nope2=reinterpret_cast<uint8_t*>(malloc(1));
	(*nope2) = (*nope) = 0;

	const size_t tab_size = 1024;
	uint8_t tab[tab_size];

	size_t test_count = 1*1000*1000 / 20;

	for (size_t i=0; i<tab_size; ++i) {
		tab[i]=0; // actually the tab will be all zero, so the test will be long (till end)
	}

	for (size_t test_i=0; test_i<test_count; test_i++) {
		if (*nope) {
			for (int i=0; i<tab_size; ++i) tab[i] = (i * (*nope) ) % (1+ (*nope2)%254 ); // so compiler can't assume the data values there
		}

		int result=1;

		size_t i=0;
		while (i<tab_size)
		{
			if (tab[i] != 0) {
				result=0; break;
			}
			++i;
		}
		(*output) = result; // "use" the result
	}

	// std::cout << "Done. test_count="<<test_count<<" tab_size="<<tab_size << std::endl;
	return 0;
}

int main() {
	return main_test();
}

