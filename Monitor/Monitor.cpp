#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;  // флаг: появилось ли событие

// Поток-поставщик
void provider() {
    while (true) {
        // Ждём 1 секунду
        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            // Блокируем мьютекс для безопасной записи
            std::lock_guard<std::mutex> lock(mtx);

            // Если предыдущее событие ещё не обработано — пропускаем (для строгой очерёдности)
            if (ready) {
                continue;
            }

            ready = true;
            std::cout << "provided\n" << std::flush;
        }

        // Уведомляем потребителя
        cv.notify_one();
    }
}

// Поток-потребитель
void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        // Ждём, пока ready не станет true
        // cv.wait автоматически разблокирует мьютекс и заснёт,
        // а при пробуждении — снова заблокирует его
        cv.wait(lock, []() { return ready; });

        std::cout << "consumed\n" << std::flush;

        ready = false;
        // lock разблокируется автоматически при выходе из блока
    }
}

int main() {
    // Создаём два потока
    std::thread t_provider(provider);
    std::thread t_consumer(consumer);

    // Ждём их завершения (хотя они бесконечные — программа работает, пока не прервут)
    t_provider.join();
    t_consumer.join();

    return 0;
}