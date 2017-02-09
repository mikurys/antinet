#include <iostream>
#include <cstdlib>
#include <chrono>

typedef unsigned char type;
unsigned char counter=0;
const int tab_size = 1024;

type checksum(type *tab, int size, int begin, int end)
{
        type acc=0;
        for(int i=begin; i<end ; i++)
        {
                acc ^= tab[i];
        }
        return acc;
}

//__attribute__((optimize("unroll-loops")))
type const_size_tab()
{
        type tab[tab_size];
        type acc = 0;
        for(int i=0; i<tab_size; i++)
        {
                tab[i]=counter;
                counter+=3;
        }

        for(int i=0; i<tab_size ; i++)
        {
                acc ^= tab[i];
        }

        return acc;

}

type main2(type *tab, int size, int begin, int end)
{

        for(int i=begin; i<end ; i++)
        {
                tab[i]=counter;
                counter+=3;
        }

        type acc = checksum(tab, size, begin, end);

        return acc;

}

int main()
{
        int size;
        int begin, end;
        int iterations = 100000000;
        type *tab;
        type static_tab[tab_size];

        typedef std::chrono::high_resolution_clock Time;
        typedef std::chrono::milliseconds ms;
        typedef std::chrono::duration<float> fsec;

        type acc = 0;

        auto t0 = Time::now();
        for (int i=0; i<iterations; i++)
        {
                acc += const_size_tab();
                //acc += main2(static_tab, tab_size, 1, tab_size);
        }
        auto t1 = Time::now();
        fsec const_size_tab_time = t1 - t0;
        ms c = std::chrono::duration_cast<ms>(const_size_tab_time);
        std::cout << "static tab" << std::endl;
        std::cout << "acc=" << static_cast<int>(acc) << ", time: ";
        std::cout << c.count() << "ms" << std::endl;
        std::cout << ((static_cast<double>(tab_size) * static_cast<double>(iterations) * static_cast<double>(sizeof(type)) * 8.) / (1024. * 1024.)) / (const_size_tab_time.count()) << " Mbps" << std::endl;

        acc = 0;
        std::cin >> size >> begin >> end;
        size /= sizeof(type);
        begin /= sizeof(type);
        end /= sizeof(type);

        tab = static_cast<type *>( std::malloc(size * sizeof(type)));

        t0 = Time::now();
        for(int i=0; i<iterations; i++)
        {
                acc += main2(tab, size, begin, end);
        }
        t1 = Time::now();
        fsec dynamic_size_tab_time = t1 - t0;
        ms d = std::chrono::duration_cast<ms>(dynamic_size_tab_time);
        std::cout << "dynamic tab" << std::endl;
        std::cout << "acc=" << static_cast<int>(acc) << ", time: " << d.count() << "ms" << std::endl;
        std::cout << ((static_cast<double>(size) * static_cast<double>(iterations) * static_cast<double>(sizeof(type)) * 8.) / (1024. * 1024.)) / (dynamic_size_tab_time.count()) << " Mbps" << std::endl;

        free(tab);
}
