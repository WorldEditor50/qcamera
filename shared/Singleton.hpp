#ifndef SINGLETON_HPP
#define SINGLETON_HPP
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
template <typename T>
class Singleton
{
public:
    using Type = T;
private:
    static QMutex mutex;
    static QSharedPointer<T> ptr;
    Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton& operator =(const Singleton &) = delete;
public:
    template<typename ...TArgs>
    static T* instance(TArgs&& ...args)
    {
        if (ptr == nullptr) {
            QMutexLocker locker(&mutex);
            if (ptr == nullptr) {
                ptr = QSharedPointer<T>(new T(std::forward(args)...));
            }
        }
        return ptr.data();
    }
    static T* instance()
    {
        if (ptr == nullptr) {
            QMutexLocker locker(&mutex);
            if (ptr == nullptr) {
                ptr = QSharedPointer<T>(new T);
            }
        }
        return ptr.data();
    }

};
template<typename T>
QSharedPointer<T> Singleton<T>::ptr(nullptr);
template<typename T>
QMutex Singleton<T>::mutex;

#endif // SINGLETON_HPP
