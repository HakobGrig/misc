#include <iostream>
#include <functional>
#include <queue>
#include <thread>
#include <chrono>
#include <string>

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())>
{};

template<typename R, typename ...Args> 
struct function_traits<std::function<R(Args...)>>
{
    typedef R result_type;
    typedef typename std::function<R(Args...)> type;
    typedef typename std::function<void(Args...)> typeNoRet;
};

template<typename R, typename ...Args> 
struct function_traits<R(*)(Args...)>
{
    typedef R result_type;
    typedef typename std::function<R(Args...)> type;
    typedef typename std::function<void(Args...)> typeNoRet;
};

template<typename R, typename cls, typename ...Args> 
struct function_traits<R(cls::*)(Args...)>
{
    typedef R result_type;
    typedef typename std::function<R(Args...)> type;
    typedef typename std::function<void(Args...)> typeNoRet;
};

using namespace std;

class CallStack {
    public:
        CallStack() {}
        ~CallStack() {}
        
    public:
        void Run() {
            while (!mQueue.empty()) {
                mQueue.front()();
                mQueue.pop();
            }

        }

        void Put(std::function<void()> f) {
            mQueue.push(std::bind(f));
        }
        
        template<typename F, typename CB>
        void Put( F f, CB cb) {
            mQueue.push(std::bind(CallHelper<F, CB>, f, cb));
        }
        
    private:
        template<typename function, typename callback>
        static void CallHelper(function f, callback cb) {
            cb(f());
        }

    private:
        std::queue<std::function<void()>> mQueue;
};

int f0(int a, int b) {
    std::cout << "f0: " << a << " " << b << std::endl;
    
}

std::string f1(int a, int b) {
    std::cout << "f1: " << a << " " << b << std::endl;
    return "yeah, that is happened";
}

void f1CallBack(std::string r) {
    std::cout << r << std::endl;
}

#define BIND_AND_CONVERT(func, ...) \
        (std::function<typename function_traits<decltype(&func)>::result_type()> \
        (std::bind(func, ##__VA_ARGS__)))

#define BIND_AND_CONVERT_CB(func) \
        (typename function_traits<decltype(&func)>::type \
        (std::bind(func, std::placeholders::_1)))

int main()
{
    CallStack cs;
    
    cout<<"main thread started" << std::endl;
    
    cs.Put( BIND_AND_CONVERT(f0, 1, 2));
    cs.Put( BIND_AND_CONVERT(f1, 1, 2)
          , BIND_AND_CONVERT_CB(f1CallBack));
    
    cs.Run();
    std::this_thread::sleep_for (std::chrono::seconds(10));
    
    return 0;
}
