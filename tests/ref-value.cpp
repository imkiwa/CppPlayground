//
// Created by kiva on 2019/12/27.
//

#include <cstdio>
#include <type_traits>
#include <utility>
#include <typeindex>
#include <cxxabi.h>
#include <string>
#include <cstring>

template <typename T>
std::string demangle() {
    char *s = abi::__cxa_demangle(typeid(T).name(),
        nullptr, nullptr, nullptr);
    std::string r = s;
    std::free(s);
    return std::move(r);
}

template <typename T, typename ... Ts>
struct is_one_of {
    static constexpr bool value = false;
};

template <typename U, typename T, typename ... Ts>
struct is_one_of<U, T, Ts...> {
    static constexpr bool value = std::is_same_v<U, T>
                                  || is_one_of<U, Ts...>::value;
};

template <typename T>
struct is_primitive_type {
    static constexpr bool value =
        is_one_of<std::remove_cv_t<T>, bool, char, unsigned char,
            short, unsigned short,
            int, unsigned int,
            long, unsigned long,
            long long, unsigned long long,
            float, double,
            long double>::value;
};

template <typename T>
static constexpr bool is_primitive_type_v = is_primitive_type<T>::value;

template <typename T, typename = void>
struct Do {
    static void doit(const T &t) {
        printf("%s: pass-by-reference\n", demangle<T>().c_str());
    }
};

template <typename T>
struct Do<T, std::enable_if_t<is_primitive_type_v<T>>> {
    static void doit(T t) {
        printf("%s: pass-by-value\n", demangle<T>().c_str());
    }
};

template <typename T>
void fuck(T &&t) {
    Do<T>::doit(std::forward<T>(t));
}

struct App {
    App() = default;

    App(const App &) = delete;

    App(App &&) noexcept = delete;

    App &operator=(App &&) = delete;

    App &operator=(const App &) = delete;
};

struct Fuck {
    int a;
    int b;

    void (*fuck_it)(Fuck *thiz);
};

void Fuck_fuck_it(Fuck *thiz) {
    printf("%d\n", thiz->a);
}

Fuck Fuck_new() {
    return Fuck{
        .fuck_it = Fuck_fuck_it
    };
}

int main() {
    // 43/54/45/4e/46/44/41/4d
    const char MAGIC[8] = {0x43, 0x54, 0x45, 0x4e, 0x46, 0x44, 0x41, 0x4d};

    // f = open(file_path,'rb')
    // header = f.read(8)
    // assert binascii.b2a_hex(header) == b'4354454e4644414d'
    FILE *f = fopen("xxx.ncm", "rb");
    char buf[8];
    fread(buf, 8, 1, f);
    if (memcmp(MAGIC, buf, 8) != 0) {
        // 不是 NCM 文件
    }

    // f.seek(2, 1)
    fseek(f, 2, SEEK_CUR);

    // key_length = f.read(4)
    // key_length = struct.unpack('<I', bytes(key_length))[0]
    int key_length = 0;
    fread(&key_length, 4, 1, f);

    // key_data = f.read(key_length)
    // key_data_array = bytearray(key_data)
    char *key_data = (char *) malloc(key_length);
    fread(key_data, key_length, 1, f);

}
