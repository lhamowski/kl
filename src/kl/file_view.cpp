#include "kl/file_view.hpp"

#include <system_error>
#include <cassert>

#if defined(_WIN32)

struct IUnknown; // Required for /permissive- and WinSDK 8.1
#include <Windows.h>

namespace kl {
namespace detail {

template <LONG_PTR null>
class handle
{
public:
    handle(HANDLE h = null_handle()) noexcept : h_{h} {}
    ~handle() { reset(); }

    handle(const handle&) = delete;
    handle& operator=(const handle&) = delete;

    handle(handle&& other) noexcept
        : h_{std::exchange(other.h_, null)}
    {
    }

    handle& operator=(handle&& other) noexcept
    {
        assert(this != &other);
        swap(*this, other);
        return *this;
    }

    friend void swap(handle& l, handle& r) noexcept
    {
        using std::swap;
        swap(l.h_, r.h_);
    }

    void reset(HANDLE h = null_handle()) noexcept
    {
        if (h_ != null_handle())
            ::CloseHandle(h_);
        h_ = h;
    }

    bool operator!() const noexcept { return h_ == null_handle(); }

    operator HANDLE() noexcept { return h_; }

    constexpr static HANDLE null_handle() noexcept
    {
        return reinterpret_cast<HANDLE>(null);
    }

private:
    HANDLE h_{null};
};
} // namespace detail

struct file_view::impl
{
    impl(gsl::cstring_span<> file_path)
    {
        detail::handle<(LONG_PTR)-1> file_handle{
            ::CreateFileA(file_path.data(), GENERIC_READ, 0x0, nullptr,
                          OPEN_EXISTING, 0x0, nullptr)};
        if (!file_handle)
            throw std::system_error{static_cast<int>(::GetLastError()),
                                    std::system_category()};

        LARGE_INTEGER file_size = {};
        ::GetFileSizeEx(file_handle, &file_size);
        if (!file_size.QuadPart)
            return; // Empty file

        detail::handle<(LONG_PTR)0> mapping_handle{::CreateFileMappingA(
            file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr)};
        if (!mapping_handle)
            throw std::system_error{static_cast<int>(::GetLastError()),
                                    std::system_category()};

        void* file_view =
            ::MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
        if (!file_view)
            throw std::system_error{static_cast<int>(::GetLastError()),
                                    std::system_category()};

        contents_ = gsl::make_span(static_cast<const byte*>(file_view),
                                   file_size.QuadPart);
    }

    ~impl()
    {
        if (!contents_.empty())
            ::UnmapViewOfFile(contents_.data());
    }

    gsl::span<const byte> contents_;
};
} // namespace kl

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace kl {
namespace detail {

class file_descriptor
{
public:
    file_descriptor(int fd = -1) noexcept : fd_{fd} {}
    ~file_descriptor() { reset(); }

    file_descriptor(const file_descriptor&) = delete;
    file_descriptor& operator=(const file_descriptor&) = delete;

    file_descriptor(file_descriptor&& other) noexcept
    {
        swap(*this, other);
    }

    file_descriptor& operator=(file_descriptor&& other) noexcept
    {
        assert(this != &other);
        swap(*this, other);
        return *this;
    }

    friend void swap(file_descriptor& l, file_descriptor& r) noexcept
    {
        using std::swap;
        swap(l.fd_, r.fd_);
    }

    void reset(int fd = -1) noexcept
    {
        if (fd_ != -1)
            ::close(fd_);
        fd_ = fd;
    }

    bool operator!() const noexcept { return fd_ == -1; }

    operator int() noexcept { return fd_; }

private:
    int fd_{-1};
};
} // namespace detail

struct file_view::impl
{
    impl(gsl::cstring_span<> file_path)
    {
        detail::file_descriptor fd{::open(file_path.data(), O_RDONLY)};
        if (!fd)
            throw std::system_error{static_cast<int>(errno),
                                    std::system_category()};

        struct stat file_info;
        if (fstat(fd, &file_info) == -1)
            throw std::system_error{static_cast<int>(errno),
                                    std::system_category()};

        if (!file_info.st_size)
            return; // Empty file

        void* file_view =
            ::mmap(nullptr, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (!file_view)
            throw std::system_error{static_cast<int>(errno),
                                    std::system_category()};

        contents_ =
            gsl::make_span(static_cast<const byte*>(file_view),
                           static_cast<std::ptrdiff_t>(file_info.st_size));
    }

    ~impl()
    {
        if (!contents_.empty())
        {
            ::munmap(const_cast<byte*>(contents_.data()),
                     contents_.size_bytes());
        }
    }

    gsl::span<const byte> contents_;
};
} // namespace kl

#endif

namespace kl {

file_view::file_view(gsl::cstring_span<> file_path)
    : impl_{std::make_unique<impl>(std::move(file_path))}
{
}

file_view::~file_view() = default;

gsl::span<const byte> file_view::get_bytes() const
{
    return impl_->contents_;
}
} // namespace kl
