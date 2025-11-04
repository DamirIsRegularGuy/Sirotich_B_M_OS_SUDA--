#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>
#include <ctime>


void func_with_one_general_vec(const std::vector<long long>& v){
    //подсчет суммы для одного потока
    long long total1 = 0;
    auto start = std::chrono::steady_clock::now();

    std::thread t1([&v, &total1](){
            for (auto el: v) total1 += el;
        });
    t1.join();

    auto end = std::chrono::steady_clock::now();

    std::cout << "Time sum for 1 thread: "<< std::chrono::duration<double>(end - start).count() << " s\n";
    std::cout << "Total sum (1 thread): " << total1 << "\n";

    //подсчет суммы для 4-х потоков без выделения дополнительной памяти под вектора,
    //каждый поток из 4-х обращается в одну область памяти, только по разным индексам начала и конца
    const size_t numThreads = 4;
    size_t blockSize = v.size() / numThreads;
    std::vector<long long> results(numThreads);
    std::thread threads[numThreads];
    auto start4 = std::chrono::steady_clock::now();

    //лямбда-выражение(функция) для подсчета суммы
    for (size_t i = 0; i < numThreads; ++i) {
        size_t startIdx = i * blockSize;
        size_t endIdx = (i == numThreads - 1) ? v.size() : startIdx + blockSize;
        threads[i] = std::thread([&v, &results, i, startIdx, endIdx]() {
            long long result = 0;
            for (size_t j = startIdx; j < endIdx; j++)
                result += v[j];
            results[i] = result;
        });
    }

    for (auto& t : threads) t.join();

    long long total4 = 0;
    for (auto r : results) total4 += r;

    auto end4 = std::chrono::steady_clock::now();

    std::cout << "Time sum for 4 threads: "<< std::chrono::duration<double>(end4 - start4).count() << " s\n";
    std::cout << "Total sum (4 threads): " << total4 << "\n";
}

void sumvec(const std::vector<long long>& vec, long long& res) {
    for (size_t i = 0; i < vec.size(); ++i) {
        res += vec[i];
    }
}

void func_with_four_vec_thread(const std::vector<long long>& v){
    long long total1 = 0;
    //подсчет суммы через один поток
    auto start = std::chrono::steady_clock::now();

    std::thread t1(sumvec, v, std::ref(total1));
    t1.join();

    auto end = std::chrono::steady_clock::now();

    std::cout << "Time sum for 1 thread: " << std::chrono::duration<double> (end - start).count() << " s\n";
    std::cout << "Total sum (1 thread): " << total1 << "\n";

    //создание векторов, для записи значений суммы на каждом потоке, для создания самих потоков,
    //для создание дополнительных векторов
    size_t blockSize = v.size() / 4;
    std::vector<long long> results(4);
    std::vector<std::vector<long long>> vectors(4);
    std::thread threads[4];
    long long total2 = 0;

    for (size_t i = 0; i < 4; ++i) {
        auto start = v.begin() + i * blockSize;
        auto end = (i == 3) ? v.end() : v.begin() + (i + 1) * blockSize;
        vectors[i] = std::vector<long long>(start, end);
    }

    auto start1 = std::chrono::steady_clock::now();

    for(size_t i=0; i<4; i++)
        threads[i] = std::thread(sumvec, std::cref(vectors[i]), std::ref(results[i]));

    for(auto& thrd: threads) thrd.join();

    auto end1 = std::chrono::steady_clock::now();
    for(auto s: results) total2 += s;

    std::cout << "Time sum for 4 threads: "<< std::chrono::duration<double>(end1 - start1).count() << " s\n";
    std::cout << "Total sum (4 threads): " << total2 << "\n";
}

int main() {
    //кол-во элементов вектора
    const size_t N = 100000000;
    std::srand(std::time(nullptr));
    std::vector<long long> data(N);

    //заполнение вектора псевдо-случайными числами
    for (size_t i = 0; i < N; ++i) {
        data[i] = std::rand() % 1001;
    }

    func_with_one_general_vec(data);
    std::cout << "\n";
    func_with_four_vec_thread(data);
    std::cout << "\n";

    return 0;
}

