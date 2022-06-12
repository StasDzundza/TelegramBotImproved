#pragma once

template <class T>
class Singleton {
   public:
   protected:
    Singleton() = default;
    ~Singleton() = default;

   public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    template <typename... Args>
    static T& getInstance(Args&&... args) {
        static T instance(std::forward<Args>(args)...);
        return instance;
    }
};
