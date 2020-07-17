#include <iostream>

#include <string>
#include <functional>
#include <stdexcept>
#include <memory>


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
                                         
class FunctionHolder {
    private:
        struct BaseHolder {
            BaseHolder() {}
            virtual ~BaseHolder() {}
        };

        template <typename T>
        struct Holder : public BaseHolder {
            Holder(T arg) : mFptr(arg) {}

            template<typename... Args>
            void Call(Args&&...args) {
                mFptr(std::forward<Args>(args)...);
            }

            template<typename R, typename... Args>
            R CallRet(Args&&...args) {
                return mFptr(std::forward<Args>(args)...);
            }

            T mFptr;
        };

    public:
        template<typename T>
        FunctionHolder(T t) : mBaseHolder(new Holder<typename function_traits<T>::type>(t))
                            , mBaseHolderNoRet(new Holder<typename function_traits<T>::typeNoRet>(t)) {}

        template<typename T, typename...Args>
        FunctionHolder(T&& t, Args&&... args) : mBaseHolder(new Holder<typename function_traits<T>::type>
                                                    (std::bind(std::forward<T>(t), std::forward<Args>(args)...)))
                                              , mBaseHolderNoRet(new Holder<typename function_traits<T>::typeNoRet>
                                                    (std::bind(std::forward<T>(t), std::forward<Args>(args)...))) {}

    void operator()() {
        this->operator()<>();
    }

    template<typename... Args>
    void operator()(Args&&... args) {
        auto f = dynamic_cast<Holder<std::function < void(Args...) > >*>(mBaseHolderNoRet.get());
        if (f) {
            f->Call(std::forward<Args>(args)...);
            return;
        }
        throw std::invalid_argument("");
    }

    template<typename R, typename... Args>
    R call(Args&&... args) {
        auto f = dynamic_cast<Holder<std::function<R(Args...)>>*>(mBaseHolder.get());
        if (f) {
            return f->template CallRet<R>(std::forward<Args>(args)...);
        }
        throw std::invalid_argument("");
    }

    private:
        std::unique_ptr<BaseHolder> mBaseHolder;
        std::unique_ptr<BaseHolder> mBaseHolderNoRet;
};


// test

struct st0 {
    st0(int x) : mX(x) {}

    std::string  print(int p) {
        std::cout << "st0::print " 
                  << mX << " " << p << std::endl;
        return "ret_from_st0::print";
    }
    int mX;
};

struct st1 {
    st1(int x) : mX(x) {}

    void operator()() {
        std::cout << "st1::operator() " 
                  << mX << " " << std::endl;
    }
    int mX;
};

void Func0(int a, int b) {
    std::cout << "Func0. "
              << " a: " << a
              << " b: " << b << std::endl;
}

void Func1(int a, int b, std::string str) {
    std::cout << "Func0. "
              << " a: " << a
              << " b: " << b
              << " str: " << str << std::endl;
}

uint64_t Func2(int a, int b, std::string str) {
    std::cout << "Func0. "
              << " a: " << a
              << " b: " << b
              << " str: " << str << std::endl;
    return 0xBAB0CAFE;
}

int main() {
    try {
        // void(int, int)
        FunctionHolder ex1(&Func0);
        ex1(1,12);
    
        // void(int, int, std::string)
        FunctionHolder ex2(&Func1);
        ex2(1, 12, std::string("Some text here"));
        
        // int(int, int, std::string)
        // call and print return value
        FunctionHolder ex3(&Func2);
        std::cout << "Ret: " << std::hex << ex3.call<uint64_t>(123, 3211, std::string("another text")) 
                  << std::dec << std::endl;
        // call and drop return value
        ex3(123, 3211, std::string("another text"));
    
        // Hold std::function<void(int)>
        std::function<void(int)> ex4 = std::bind(&Func0, 1, std::placeholders::_1);
        FunctionHolder c(std::function<void(int)>(std::bind(&Func0, 1, std::placeholders::_1)));
        ex4(12);
    
        // will bind to st0 member function print
        st0 st0object(8955);
        FunctionHolder ex5(&st0::print, st0object, std::placeholders::_1);
        ex5(2222);
        // call and print return value
        std::cout << "Ret: " << ex5.call<std::string>(7531) << std::endl;
    
        // wrap lambda function with std::function and pass to holder
        FunctionHolder ex6(std::function<void(void)>([]() {std::cout << "lambda function called" << std::endl;}));
        ex6();
    
        // functor object st1
        FunctionHolder ex7(st1(123654));
        ex7();
        
        // Will throw, because st1::operator() gets no arguments
        ex7(123);
    } catch (std::invalid_argument &e) {
        std::cout << "Invalid argument(s) were passed" << std::endl;
        // error handling here...
    }

    return 0;
}
